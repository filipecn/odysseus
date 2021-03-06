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
///\file stack_allocator.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-03-06
///
///\brief

#ifndef ODYSSEUS_ODYSSEUS_MEMORY_STACK_ALLOCATOR_H
#define ODYSSEUS_ODYSSEUS_MEMORY_STACK_ALLOCATOR_H

#include <ponos/common/defs.h>

namespace odysseus {

/// RAII Stack Allocator
class StackAllocator {
public:
  /****************************************************************************
                                 CONSTRUCTORS
  ****************************************************************************/
  /// \param size_in_bytes total memory capacity
  explicit StackAllocator(u32 size_in_bytes);
  ///
  ~StackAllocator();
  /****************************************************************************
                                    METHODS
  ****************************************************************************/
  /// \return total stack capacity (in bytes)
  [[nodiscard]] u32 capacityInBytes() const;
  /// \return available size that can be allocated
  [[nodiscard]] u32 availableSizeInBytes() const;
  /// All previous data is deleted and markers get invalid
  /// \param size_in_bytes total memory capacity
  void resize(u32 size_in_bytes);
  /// Allocates a new block from stack top
  /// \param block_size_in_bytes
  /// \return pointer to the new allocated block
  void *allocate(u64 block_size_in_bytes);
  /// \return a marker to the current stack top
  [[nodiscard]] u32 topMarker() const;
  /// Roll the stack back to a previous marker point
  /// \param marker
  void freeToMarker(u32 marker);
  /// Roll stack back to zero
  void clear();

private:
  u8 *data_{nullptr};
  u32 capacity_{0};
  u32 marker_{0};

};

}

#endif //ODYSSEUS_ODYSSEUS_MEMORY_STACK_ALLOCATOR_H
