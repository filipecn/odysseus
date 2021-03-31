//
// Created by filipecn on 11/03/2021.
//


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
///\file object_pool.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-03-11
///
///\brief

#ifndef ODYSSEUS_ODYSSEUS_CONTAINERS_OBJECT_POOL_H
#define ODYSSEUS_ODYSSEUS_CONTAINERS_OBJECT_POOL_H

#include <odysseus/memory/mem.h>

namespace odysseus {

/// RAII Object Pool
/// An object pool stores a set of objects that can be created and destroyed in
/// any order while still providing fast iteration to them.
/// The pool has a limit number of active objects it can hold that can be
/// increased at the cost of memory copies and possible allocations.
/// \tparam O
template<typename O>
class ObjectPool {
public:
  struct Handle {
    u32 index;
  };
  /****************************************************************************
                                 CONSTRUCTORS
  ****************************************************************************/
  ///
  /// \param max_object_count
  /// \param context
  explicit ObjectPool(u32 max_object_count, mem::ContextType context = mem::ContextType::HEAP) {
    std::size_t size_in_bytes = max_object_count * sizeof(u32);


  }
  /****************************************************************************
                                    SIZE
  ****************************************************************************/
  ///
  /// \return
  [[nodiscard]] u32 sizeInBytes() const {}
  [[nodiscard]] u32 capacity() const {}

  /****************************************************************************
                                    ALLOCATION
  ****************************************************************************/
  template<class... P>
  Handle allocate(P &&... params) {
    //
  }

private:
};

}

#endif //ODYSSEUS_ODYSSEUS_CONTAINERS_OBJECT_POOL_H
