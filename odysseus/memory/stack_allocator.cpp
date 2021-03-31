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

#define SA_EXTRACT_MARKER(HANDLE) \
  ((HANDLE & 0xffffff) - 1)

#define SA_BUILD_HANDLE(MARKER, SHIFT) \
  ((MARKER + 1u) | (SHIFT << 24u))

StackAllocator::StackAllocator(std::size_t size_in_bytes) {
  resize(size_in_bytes);
}

StackAllocator::StackAllocator(std::size_t size_in_bytes, byte *buffer) :
    data_(buffer), capacity_(size_in_bytes), using_extern_memory_{true} {
}

StackAllocator::~StackAllocator() {
  if (!using_extern_memory_)
    delete[] data_;
}

std::size_t StackAllocator::capacityInBytes() const {
  return capacity_;
}

std::size_t StackAllocator::availableSizeInBytes() const {
  return capacity_ - marker_;
}

OdResult StackAllocator::resize(std::size_t size_in_bytes) {
  if (!using_extern_memory_)
    delete[] data_;
  else
    return OdResult::BAD_OPERATION;
  marker_ = 0;
  capacity_ = size_in_bytes;
  if (size_in_bytes)
    data_ = new u8[size_in_bytes];
  ODYSSEUS_DEBUG_CODE(db_handles.clear();
                          db_regions.clear();)
  return OdResult::SUCCESS;
}

MemHandle StackAllocator::allocate(std::size_t block_size_in_bytes, std::size_t align) {
  std::size_t
      actual_size = block_size_in_bytes + mem::rightAlignShift(reinterpret_cast<uintptr_t >(data_ ) + marker_, align);
  std::size_t shift = actual_size - block_size_in_bytes;
  if (actual_size > capacity_ - marker_)
    return {0};
  const auto marker = marker_;
  marker_ += actual_size;
  ODYSSEUS_DEBUG_CODE(db_handles.emplace_back(marker);
                          db_regions.push_back({
                                                   marker,
                                                   actual_size,
                                                   1,
                                                   ponos::ConsoleColors::color(db_handles.size()),
                                                   {}
                                               });
  )
  return {SA_BUILD_HANDLE(marker + shift, shift)};
}

OdResult StackAllocator::freeTo(MemHandle handle) {
  if (!marker_)
    return OdResult::BAD_OPERATION;
  if (!handle.id)
    return OdResult::INVALID_INPUT;
  marker_ = SA_EXTRACT_MARKER(handle.id);
  ODYSSEUS_DEBUG_CODE(
      std::size_t db_i = 0;
      for (std::size_t i = 0; i < db_handles.size(); ++i)
        if (db_handles[i] == marker_)
          db_i = i;
      db_handles.resize(db_i);
      db_regions.resize(db_i);)
  return OdResult::SUCCESS;
}

void StackAllocator::clear() {
  ODYSSEUS_DEBUG_CODE(db_handles.clear();
                          db_regions.clear();)
  marker_ = 0;
}

#ifdef ODYSSEUS_DEBUG
void StackAllocator::dump(std::size_t start, std::size_t size) const {
  ponos::MemoryDumper::dump(data_ + start, size ? size : capacity_ - start,
                            64, ponos::memory_dumper_options::colored_output
                                | ponos::memory_dumper_options::cache_align,
                            db_regions);
}

std::vector<ponos::MemoryDumper::Region> StackAllocator::getRegions() {
  std::vector<ponos::MemoryDumper::Region> regions = {
      { // data_
          0,
          sizeof(data_),
          1,
          ponos::ConsoleColors::color(1),
          {}
      },
      { // capacity_
          sizeof(data_),
          sizeof(capacity_),
          1,
          ponos::ConsoleColors::color(2),
          {}
      },
      { // marker_
          sizeof(data_) + sizeof(capacity_),
          sizeof(marker_),
          1,
          ponos::ConsoleColors::color(3),
          {}
      },
      { // capacity
          sizeof(data_) + sizeof(capacity_) + sizeof(marker_),
          sizeof(using_extern_memory_),
          1,
          ponos::ConsoleColors::color(4),
          {}
      },
      { // db_handles
          offsetof(StackAllocator, db_handles),
          sizeof(db_handles),
          1,
          ponos::ConsoleColors::color(5),
          {}
      },
      { // db_regions
          offsetof(StackAllocator, db_regions),
          sizeof(db_regions),
          1,
          ponos::ConsoleColors::color(6),
          {}
      },
  };
  return std::move(regions);
}

std::vector<ponos::MemoryDumper::Region> StackAllocator::getDataRegions() {
  return db_regions;
}
#endif

#undef SA_EXTRACT_MARKER
#undef SA_BUILD_HANDLE

}
