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
///\file stack_allocator.cpp.c
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-03-06
///
///\brief

#include <odysseus/memory/stack_allocator.h>

#include <odysseus/memory/mem.h>

namespace odysseus {

StackAllocator::StackAllocator(u32 size_in_bytes) {
  resize(size_in_bytes);
}

StackAllocator::~StackAllocator() {
  delete[] data_;
}

u32 StackAllocator::capacityInBytes() const {
  return capacity_;
}

u32 StackAllocator::availableSizeInBytes() const {
  return capacity_ - marker_;
}

void StackAllocator::resize(u32 size_in_bytes) {
  delete[] data_;
  data_ = new u8[size_in_bytes];
  capacity_ = size_in_bytes;
}

void *StackAllocator::allocate(u64 block_size_in_bytes) {
  if (block_size_in_bytes > capacity_ - marker_)
    return nullptr;
  const auto marker = marker_;
  marker_ += block_size_in_bytes;
  return reinterpret_cast<void *>(data_ + marker);
}

u32 StackAllocator::topMarker() const {
  return marker_;
}

void StackAllocator::freeToMarker(u32 marker) {
  marker_ = marker;
}
void StackAllocator::clear() {
  marker_ = 0;
}

}
