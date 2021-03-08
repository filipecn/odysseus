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
///\file pool_allocator.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-03-07
///
///\brief

#ifndef ODYSSEUS_ODYSSEUS_MEMORY_POOL_ALLOCATOR_H
#define ODYSSEUS_ODYSSEUS_MEMORY_POOL_ALLOCATOR_H

#include <odysseus/memory/mem.h>

namespace odysseus {

/// RAII Pool Allocator
/// Stores a pool of objects of same size and allows arbitrary destruction order.
class PoolAllocator {
public:
  /****************************************************************************
                                 CONSTRUCTORS
  ****************************************************************************/
  ///
  /// \param object_size_in_bytes
  /// \param object_count
  /// \param context
  PoolAllocator(u32 object_size_in_bytes, u32 object_count, mem::Context context = mem::Context::HEAP);
  ///
  ~PoolAllocator();
  /****************************************************************************
                                    SIZE
  ****************************************************************************/
  /// \return total memory size in bytes
  [[nodiscard]] u32 capacityInBytes() const;
  /// \return capacity in number of objects
  [[nodiscard]] u32 capacity() const;
  /// \return number of allocated objects
  [[nodiscard]] u32 size() const;
  /// \return
  [[nodiscard]] u32 objectSizeInBytes() const;
  /****************************************************************************
                                    ALLOCATION
  ****************************************************************************/
  ///
  /// \return
  void *allocate();
  ///
  /// \param ptr
  void freeObject(void *ptr);
private:
  u32 size_{0};
  u32 capacity_{0};
  u32 object_size_in_bytes_{0};
  u32 head_{0};
  mem::MemPtr data_{};
};

}

#endif //ODYSSEUS_ODYSSEUS_MEMORY_POOL_ALLOCATOR_H
