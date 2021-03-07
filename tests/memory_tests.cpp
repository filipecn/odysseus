//
// Created by filipecn on 06/03/2021.
//
#include <catch2/catch.hpp>
#include <odysseus/memory/mem.h>

using namespace odysseus;

TEST_CASE("mem", "[memory]") {
  SECTION("allocAligned") {
    auto *ptr = mem::allocAligned(10, 1);
    mem::freeAligned(ptr);
  }//
}
