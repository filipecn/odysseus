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
///\file pool_allocator.cpp.c
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-03-07
///
///\brief

#include <odysseus/memory/pool_allocator.h>

namespace odysseus {
/******************************************************************************
 *                                 DEBUG
******************************************************************************/
void dumpAvailableList(void *ptr, u32 head, u32 object_count, u32 object_size_in_bytes) {
  auto *p = reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(ptr) + head * object_size_in_bytes);
  auto *end = reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(ptr) + object_count * object_size_in_bytes);
  int i = 0;
  while (p < end) {
    printf("free object %d: next %u address %p < sentinel %p\n", i, *p, (void *) p, (void *) end);
    p = reinterpret_cast<u32 *> (reinterpret_cast<u8 *>(ptr) + (*p) * object_size_in_bytes);
    if (i++ > 12)
      break;
  }
}

PoolAllocator::PoolAllocator(u32 object_size_in_bytes, u32 object_count, mem::Context context)
    : capacity_{object_count}, object_size_in_bytes_{object_size_in_bytes} {
  ASSERT(object_size_in_bytes >= sizeof(u32));
  data_ = mem::allocateBlock(object_size_in_bytes * object_count, context);
  // create linked list for free objects
  auto *p = reinterpret_cast<u32 *>(data_.ptr);
  for (uintptr_t i = 0; i < object_count; i++) {
    *p = i + 1;
    p = reinterpret_cast<u32 *> (reinterpret_cast<u8 *>(data_.ptr) + (i + 1) * object_size_in_bytes);
  }
}

PoolAllocator::~PoolAllocator() {
  mem::freeBlock(data_);
}

u32 PoolAllocator::capacityInBytes() const {
  return capacity_ * object_size_in_bytes_;
}

u32 PoolAllocator::capacity() const {
  return capacity_;
}

u32 PoolAllocator::size() const {
  return size_;
}

u32 PoolAllocator::objectSizeInBytes() const {
  return object_size_in_bytes_;
}

void *PoolAllocator::allocate() {
  if (head_ >= capacity_)
    return nullptr;
  size_++;
  // get head pointer
  auto *p = reinterpret_cast<u8 *>(data_.ptr) + head_ * object_size_in_bytes_;
  // move head
  head_ = *reinterpret_cast<u32 *>(p);
  return reinterpret_cast<void *>(p);
}

void PoolAllocator::freeObject(void *ptr) {
  ASSERT(size_);
  ptrdiff_t d = reinterpret_cast<u8 *>(ptr) - reinterpret_cast<u8 *>(data_.ptr);
  auto *p = reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(data_.ptr) + d);
  *p = head_;
  head_ = d / object_size_in_bytes_;
  size_--;
}

}
