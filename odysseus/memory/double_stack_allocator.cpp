/// Copyright (c) 2021, FilipeCN.
///
/// The MIT License (MIT)
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
///\file double_stack_allocator.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-03-07
///
///\brief

#include <odysseus/memory/double_stack_allocator.h>

namespace odysseus {

DoubleStackAllocator::DoubleStackAllocator(u32 capacity_in_bytes, mem::Context context) {
  resize(capacity_in_bytes, context);
}

DoubleStackAllocator::~DoubleStackAllocator() {
//  mem::freeBlock(data_);
}

u32 DoubleStackAllocator::capacityInBytes() const {
  return capacity_;
}

u32 DoubleStackAllocator::availableLowerSizeInBytes() const {
  if (threshold_ < capacity_)
    return threshold_ - lower_marker_;
  return upper_marker_ - lower_marker_ + 1;
}

u32 DoubleStackAllocator::availableUpperSizeInBytes() const {
  if (threshold_ < capacity_)
    return upper_marker_ - threshold_ + 1;
  return upper_marker_ - lower_marker_ + 1;
}

void DoubleStackAllocator::resize(u32 size_in_bytes, mem::Context context) {
//  data_ = mem::allocateBlock(size_in_bytes, context);
  capacity_ = size_in_bytes;
  threshold_ = capacity_ + 1;
  lower_marker_ = 0;
  upper_marker_ = size_in_bytes - 1;
}

void DoubleStackAllocator::setThreshold(u32 lower_stack_size_in_bytes) {
  threshold_ = lower_stack_size_in_bytes;
}

void *DoubleStackAllocator::allocateLower(u64 block_size_in_bytes) {
  if (lower_marker_ + block_size_in_bytes > threshold_ ||
      lower_marker_ + block_size_in_bytes > capacity_)
    return nullptr;
  const auto marker = lower_marker_;
  lower_marker_ += block_size_in_bytes;
  return reinterpret_cast<void *>(reinterpret_cast<u8 *>(data_) + marker);
}

void *DoubleStackAllocator::allocateUpper(u64 block_size_in_bytes) {
  if (upper_marker_ - block_size_in_bytes > capacity_ ||
      (threshold_ < capacity_ && upper_marker_ - block_size_in_bytes < threshold_ - 1))
    return nullptr;
  const auto marker = upper_marker_;
  upper_marker_ -= block_size_in_bytes;
  return reinterpret_cast<void *>(reinterpret_cast<u8 *>(data_) + marker);
}

u32 DoubleStackAllocator::topLowerMarker() const {
  return lower_marker_;
}

u32 DoubleStackAllocator::topUpperMarker() const {
  return upper_marker_;
}

void DoubleStackAllocator::freeToUpperMarker(u32 upper_marker) {
  upper_marker_ = upper_marker;
}

void DoubleStackAllocator::freeToLowerMarker(u32 lower_marker) {
  lower_marker_ = lower_marker;
}

}