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
    };
    REQUIRE(sizeof(S) == 8);
    REQUIRE(mem::alignTo(15, sizeof(S)) == 16);
    REQUIRE(mem::alignTo(17, sizeof(S)) == 24);
  }//
  SECTION("left and right alignments") {
    REQUIRE(mem::leftAlignShift(100, 64) == 100 - 64);
    REQUIRE(mem::rightAlignShift(100, 64) == 128 - 100);

    REQUIRE(mem::leftAlignShift(100, 1) == 0);
    REQUIRE(mem::rightAlignShift(100, 1) == 0);
  }//
  SECTION("allocAligned") {
    auto *ptr = mem::allocAligned(10, 1);
    mem::freeAligned(ptr);
  }//
  SECTION("sanity") {
    REQUIRE(mem::availableSize() == 0);
    REQUIRE(mem::init(160 + sizeof(StackAllocator)) == OdResult::SUCCESS);
    REQUIRE(mem::availableSize() == 160 + sizeof(StackAllocator));
    REQUIRE(mem::pushContext<StackAllocator>(160) == OdResult::SUCCESS);
    REQUIRE(mem::availableSize() == 0);
    auto &sa = mem::getContext<StackAllocator>(0);

    for (int i = 0; i < 40; ++i)
      sa.allocateAligned<int>(i + 1);
    ODYSSEUS_DEBUG_CODE(mem::dump();)
  } //
}

TEST_CASE("StackAllocator", "[memory]") {
  SECTION("empty") {
    StackAllocator stack_allocator;
    REQUIRE(stack_allocator.capacityInBytes() == 0);
    REQUIRE(stack_allocator.availableSizeInBytes() == 0);
    REQUIRE(stack_allocator.allocate(10).id == 0);
    REQUIRE(stack_allocator.allocateAligned<int>().id == 0);
    REQUIRE(stack_allocator.freeTo({}) == OdResult::BAD_OPERATION);
  }//
  SECTION("sanity") {
    StackAllocator stack_allocator;
    REQUIRE(stack_allocator.resize(100) == OdResult::SUCCESS);
    REQUIRE(stack_allocator.capacityInBytes() == 100);
    REQUIRE(stack_allocator.availableSizeInBytes() == 100);
    auto p = stack_allocator.allocate(50);
    REQUIRE(p.id == 1);
    REQUIRE(stack_allocator.availableSizeInBytes() == 50);
    stack_allocator.clear();
    REQUIRE(stack_allocator.availableSizeInBytes() == 100);
    stack_allocator.resize(200);
    REQUIRE(stack_allocator.capacityInBytes() == 200);
    auto p1 = stack_allocator.allocate(180);
    REQUIRE(p1.id == 1);
    auto p2 = stack_allocator.allocate(40);
    REQUIRE(p2.id == 0);
    REQUIRE(stack_allocator.availableSizeInBytes() == 20);
    REQUIRE(stack_allocator.freeTo(p1) == OdResult::SUCCESS);
    REQUIRE(stack_allocator.availableSizeInBytes() == 200);
  }//
  SECTION("debug") {
#ifdef ODYSSEUS_DEBUG
    StackAllocator stack_allocator(200);
    stack_allocator.allocate(10);
    stack_allocator.allocate(50);
    stack_allocator.allocate(80, 64);
    stack_allocator.dump();
#endif
  }//
  SECTION("set get") {
    StackAllocator stack_allocator(60);
    std::vector<MemHandle> handles(20);
    for (int i = 0; i < 20; ++i)
      // PAREI AQUI....HANDLE DANDO ZERO!?
      handles.emplace_back(stack_allocator.allocateAligned<int>(0));
    for (int i = 0; i < handles.size(); ++i) {
     std::cerr << "HANDLE " << handles[i].id << std::endl;
//      REQUIRE(stack_allocator.set(handles[i], i + 1) == OdResult::SUCCESS);
    }
#ifdef ODYSSEUS_DEBUG
    stack_allocator.dump();
#endif
  }
}

TEST_CASE("DoubleStackAllocator", "[memory]") {
  SECTION("sanity") {
    // L                                                                     U
    // 0 ------------------------------------------------------------------- 99
    DoubleStackAllocator dsa(100);
    REQUIRE(dsa.capacityInBytes() == 100);
    REQUIRE(dsa.availableLowerSizeInBytes() == 100);
    REQUIRE(dsa.availableUpperSizeInBytes() == 100);
//    REQUIRE(dsa.topLowerMarker() == 0);
//    REQUIRE(dsa.topUpperMarker() == 99);
    //                     L                                                 U
    // 0 -----------------40------------------------------------------------ 99
    REQUIRE(dsa.allocateLower(40).isValid());
//    REQUIRE(dsa.topLowerMarker() == 40);
    REQUIRE(dsa.availableLowerSizeInBytes() == 60);
    REQUIRE(dsa.availableUpperSizeInBytes() == 60);
    //                     L                          U
    // 0 -----------------40--------------------------59-------------------- 99
    REQUIRE(dsa.allocateUpper(40).isValid());
//    REQUIRE(dsa.topUpperMarker() == 59);
    REQUIRE(dsa.availableLowerSizeInBytes() == 20);
    REQUIRE(dsa.availableUpperSizeInBytes() == 20);
    //                     L            T             U
    // 0 -----------------40-----------50-------------59-------------------- 99
    dsa.setThreshold(50);
    REQUIRE(dsa.availableLowerSizeInBytes() == 10);
    REQUIRE(dsa.availableUpperSizeInBytes() == 10);
    REQUIRE(!dsa.allocateLower(11).isValid());
    REQUIRE(!dsa.allocateUpper(11).isValid());
    //                                 LT             U
    // 0 -----------------40-----------50-------------59-------------------- 99
    REQUIRE(dsa.allocateLower(10).isValid());
    REQUIRE(dsa.availableLowerSizeInBytes() == 0);
//    REQUIRE(dsa.topLowerMarker() == 50);
    //                              U  LT
    // 0 -----------------40--------49-50-------------59-------------------- 99
    REQUIRE(dsa.allocateUpper(10).isValid());
//    REQUIRE(dsa.topUpperMarker() == 49);
    REQUIRE(dsa.availableUpperSizeInBytes() == 0);
    REQUIRE(!dsa.allocateLower(1).isValid());
    REQUIRE(!dsa.allocateUpper(1).isValid());
  }//
  SECTION("debug") {
#ifdef ODYSSEUS_DEBUG
    DoubleStackAllocator stack_allocator(100);
    for (int i = 1; i <= 10; ++i) {
      stack_allocator.allocateAlignedLower<int>(i);
      stack_allocator.allocateAlignedUpper<int>(i);
    }
    stack_allocator.dump();
    REQUIRE(stack_allocator.resize(200) == OdResult::SUCCESS);
    auto p1 = stack_allocator.allocateLower(4, 64);
    REQUIRE(p1.isValid());
    stack_allocator.set(p1, 0xffffffff);
    auto p2 = stack_allocator.allocateUpper(4, 64);
    REQUIRE(p2.isValid());
    stack_allocator.set(p2, 0xffffffff);
    stack_allocator.dump();
#endif
  }//
}

TEST_CASE("PoolAllocator", "[memory]") {
  return;
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