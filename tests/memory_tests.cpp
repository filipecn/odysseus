//
// Created by filipecn on 06/03/2021.
//
#include <catch2/catch.hpp>
#include <odysseus/memory/mem.h>
#include <odysseus/memory/stack_allocator.h>
#include <odysseus/memory/double_stack_allocator.h>
#include <odysseus/memory/pool_allocator.h>
#include <iostream>

using namespace odysseus;

TEST_CASE("mem", "[memory]") {
  SECTION("alignTo") {
    REQUIRE(mem::alignTo(1, sizeof(u8)) == sizeof(u8));
    REQUIRE(mem::alignTo(1, sizeof(u16)) == sizeof(u16));
    REQUIRE(mem::alignTo(1, sizeof(u32)) == sizeof(u32));
    REQUIRE(mem::alignTo(1, sizeof(u64)) == sizeof(u64));
    struct S {
      f32 a;
      u8 b;
      u16 c;
    } s{};
    REQUIRE(sizeof(S) == 8);
    REQUIRE(mem::alignTo(15, sizeof(S)) == 16);
    REQUIRE(mem::alignTo(17, sizeof(S)) == 24);
  }//
  SECTION("allocAligned") {
    auto *ptr = mem::allocAligned(10, 1);
    mem::freeAligned(ptr);
  }//
  SECTION("sanity") {
    REQUIRE(mem::availableSize() == 0);
    REQUIRE(mem::init(200) == OdResult::SUCCESS);
    REQUIRE(mem::availableSize() == 200);
    REQUIRE(mem::pushContext<StackAllocator>(100) == OdResult::SUCCESS);
    REQUIRE(mem::availableSize() == 100 - sizeof(StackAllocator));
    auto &sa = mem::getContext<StackAllocator>(0);

    for (int i = 0; i < 10; ++i) {
      auto *ptr = sa.allocate<int>();
      *ptr = i + 1;
    }

    std::cerr << mem::dump(0, 100 + sizeof(StackAllocator));
  }
}

TEST_CASE("StackALlocator", "[memory]") {
  SECTION("sanity") {
    /*
    StackAllocator stack_allocator(100);
    REQUIRE(stack_allocator.capacityInBytes() == 100);
    REQUIRE(stack_allocator.availableSizeInBytes() == 100);
    REQUIRE(stack_allocator.topMarker() == 0);
    auto *p = stack_allocator.allocate(50);
    REQUIRE(p != nullptr);
    REQUIRE(stack_allocator.availableSizeInBytes() == 50);
    REQUIRE(stack_allocator.topMarker() == 50);
    stack_allocator.clear();
    REQUIRE(stack_allocator.availableSizeInBytes() == 100);
    REQUIRE(stack_allocator.topMarker() == 0);
    stack_allocator.resize(200);
    REQUIRE(stack_allocator.capacityInBytes() == 200);
    p = stack_allocator.allocate(180);
    REQUIRE(p != nullptr);
    REQUIRE(stack_allocator.topMarker() == 180);
    p = stack_allocator.allocate(40);
    REQUIRE(p == nullptr);
    REQUIRE(stack_allocator.availableSizeInBytes() == 20);
    stack_allocator.freeToMarker(0);
    REQUIRE(stack_allocator.availableSizeInBytes() == 200);
  */}
}

TEST_CASE("DoubleStackAllocator", "[memory]") {
  SECTION("sanity") {
    // L                                                                     U
    // 0 ------------------------------------------------------------------- 99
    DoubleStackAllocator dsa(100);
    REQUIRE(dsa.capacityInBytes() == 100);
    REQUIRE(dsa.availableLowerSizeInBytes() == 100);
    REQUIRE(dsa.availableUpperSizeInBytes() == 100);
    REQUIRE(dsa.topLowerMarker() == 0);
    REQUIRE(dsa.topUpperMarker() == 99);
    //                     L                                                 U
    // 0 -----------------40------------------------------------------------ 99
    REQUIRE(dsa.allocateLower(40) != nullptr);
    REQUIRE(dsa.topLowerMarker() == 40);
    REQUIRE(dsa.availableLowerSizeInBytes() == 60);
    REQUIRE(dsa.availableUpperSizeInBytes() == 60);
    //                     L                          U
    // 0 -----------------40--------------------------59-------------------- 99
    REQUIRE(dsa.allocateUpper(40) != nullptr);
    REQUIRE(dsa.topUpperMarker() == 59);
    REQUIRE(dsa.availableLowerSizeInBytes() == 20);
    REQUIRE(dsa.availableUpperSizeInBytes() == 20);
    //                     L            T             U
    // 0 -----------------40-----------50-------------59-------------------- 99
    dsa.setThreshold(50);
    REQUIRE(dsa.availableLowerSizeInBytes() == 10);
    REQUIRE(dsa.availableUpperSizeInBytes() == 10);
    REQUIRE(!dsa.allocateLower(11));
    REQUIRE(!dsa.allocateUpper(11));
    //                                 LT             U
    // 0 -----------------40-----------50-------------59-------------------- 99
    REQUIRE(dsa.allocateLower(10));
    REQUIRE(dsa.availableLowerSizeInBytes() == 0);
    REQUIRE(dsa.topLowerMarker() == 50);
    //                              U  LT
    // 0 -----------------40--------49-50-------------59-------------------- 99
    REQUIRE(dsa.allocateUpper(10));
    REQUIRE(dsa.topUpperMarker() == 49);
    REQUIRE(dsa.availableUpperSizeInBytes() == 0);
    REQUIRE(!dsa.allocateLower(1));
    REQUIRE(!dsa.allocateUpper(1));
  }//
}

TEST_CASE("PoolAllocator", "[memory]") {
  SECTION("sanity") {
    PoolAllocator pa(10, 10);
    REQUIRE(pa.capacity() == 10);
    REQUIRE(pa.capacityInBytes() == 100);
    REQUIRE(pa.objectSizeInBytes() == 10);
    REQUIRE(pa.size() == 0);
    void *ptrs[10];
    for (int i = 0; i < 10; ++i) {
      ptrs[i] = pa.allocate();
      REQUIRE(ptrs[i]);
      REQUIRE(pa.size() == i + 1);
    }
    REQUIRE(!pa.allocate());
    auto expected_size = pa.size();
    for (auto &ptr : ptrs) {
      pa.freeObject(ptr);
      REQUIRE(pa.size() == --expected_size);
    }
  }
}