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
///\file double_stack_allocator.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-03-07
///
///\brief

#ifndef ODYSSEUS_ODYSSEUS_MEMORY_DOUBLE_STACK_ALLOCATOR_H
#define ODYSSEUS_ODYSSEUS_MEMORY_DOUBLE_STACK_ALLOCATOR_H

#include <odysseus/memory/mem.h>

namespace odysseus {

/// RAII Double Stack Allocator
/// Manages two stacks stored in a single memory block, the LOWER stack and
/// the UPPER stack. Each stack is stored in one end of the memory block,
/// the LOWER stack is stored in lower addresses while the UPPER stack is
/// stored in the upper end. Both stacks advance towards each other. The
/// following diagram depicts the double stack allocator structure:
///
///         0              L        T      U            N
///         [**** LOWER ***)--------|------(** UPPER ** ]
///
/// In a memory block of N + 1 bytes, the LOWER stack occupies the range
/// [0, L) and the UPPER stack occupies (U, N]. It is possible to define a
/// limiting threshold T to limit the individual capacity of both stacks.
class DoubleStackAllocator {
public:
  /****************************************************************************
                                 CONSTRUCTORS
  ****************************************************************************/
  /// \param capacity_in_bytes total memory capacity
  /// \param context
  explicit DoubleStackAllocator(u32 capacity_in_bytes, mem::Context context = mem::Context::HEAP);
  ///
  ~DoubleStackAllocator();
  /****************************************************************************
                                    SIZE
  ****************************************************************************/
  /// \return total stack capacity (in bytes)
  [[nodiscard]] u32 capacityInBytes() const;
  /// \return available size that can be allocated in the lower stack
  [[nodiscard]] u32 availableLowerSizeInBytes() const;
  /// \return available size that can be allocated in the upper stack
  [[nodiscard]] u32 availableUpperSizeInBytes() const;
  /// All previous data is deleted and markers get invalid
  /// \param size_in_bytes total memory capacity
  /// \param context
  void resize(u32 size_in_bytes, mem::Context context);
  /// \param lower_stack_size_in_bytes a value grater than capacity removes the
  /// threshold
  void setThreshold(u32 lower_stack_size_in_bytes);
  /****************************************************************************
                                    ALLOCATION
  ****************************************************************************/
  /// Allocates a new block from lower stack top
  /// \param block_size_in_bytes
  /// \return pointer to the new allocated block
  void *allocateLower(u64 block_size_in_bytes);
  /// Allocates a new block from upper stack top
  /// \param block_size_in_bytes
  /// \return pointer to the new allocated block
  void *allocateUpper(u64 block_size_in_bytes);
  /// \return a marker to the current lower stack top
  [[nodiscard]] u32 topLowerMarker() const;
  /// \return a marker to the current upper stack top
  [[nodiscard]] u32 topUpperMarker() const;
  /// \param upper_marker
  void freeToUpperMarker(u32 upper_marker);
  /// \param lower_marker
  void freeToLowerMarker(u32 lower_marker);

private:
  void* data_{};
  u32 capacity_{0};
  u32 lower_marker_{0};
  u32 upper_marker_{0};
  u32 threshold_{0};
};

}

#endif //ODYSSEUS_ODYSSEUS_MEMORY_DOUBLE_STACK_ALLOCATOR_H
