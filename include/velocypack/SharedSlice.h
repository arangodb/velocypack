////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2020 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Tobias Gödderz
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <velocypack/Buffer.h>
#include <velocypack/Slice.h>

#include <memory>

namespace arangodb::velocypack {

/**
 * @brief SharedSlice is similar to a Slice and has the same methods available.
 * The difference is that SharedSlice owns the memory it points to (via a
 * shared_ptr).
 *
 * It will *always* point to a valid Slice. Even after default construction, or
 * after a move, or when constructing it with a nullptr, it will point to a
 * (static) None-Slice.
 *
 * All methods of Slice that return a Slice have an equivalent method here, but
 * return a SharedSlice instead, which shares ownership of the same memory (but
 * may point to a different location).
 *
 * Similarly, all methods of Slice that return a raw pointer have an equivalent
 * method returning a shared_ptr, which also shares memory ownership with
 * SharedSlice.
 *
 * Additional methods are
 *   std::shared_ptr<uint8_t const> const& buffer();
 * which is an accessor of the underlying buffer pointer, and
 *   Slice slice();
 * which returns a normal Slice using the same memory.
 *
 * The only method missing, in comparison to Slice, is set(uint8_t const*). If
 * necessary, it should probably be implemented as
 * set(std::shared_ptr<uint8_t const>) instead.
 */
class SharedSlice : public SliceBase<SharedSlice, SharedSlice> {
 public:
  explicit SharedSlice(uint8_t const* start) {
    auto size = Slice(start).byteSize();
    auto ptr = std::make_shared<uint8_t[]>(size);
    std::memcpy(ptr.get(), start, size);
    _mem = std::move(ptr);
  }
  SharedSlice(std::shared_ptr<uint8_t const[]> mem, uint8_t const* start)
      : _mem(std::move(mem), start) {}
  SharedSlice(std::shared_ptr<uint8_t const[]> mem, Slice start)
      : _mem(std::move(mem), start.getDataPtr()) {}
  SharedSlice() = default;

  SharedSlice(Buffer<uint8_t> buffer) {
    if (buffer.usesLocalMemory()) {
      _mem = std::move(SharedSlice(buffer.data())._mem);
    } else {
      _mem = std::shared_ptr<uint8_t const[]>(buffer.steal());
    }
  }

  uint8_t const* getDataPtr() const noexcept {
    return _mem ? _mem.get() : Slice::noneSliceData;
  }

  Slice slice() const { return Slice(getDataPtr()); }
  std::shared_ptr<uint8_t const[]> buffer() const { return _mem; }

 private:
  friend struct SliceBase<SharedSlice, SharedSlice>;
  std::shared_ptr<uint8_t const[]> _mem;

  static SharedSlice make_impl(SharedSlice const* parent, uint8_t const* mem) {
    return SharedSlice{parent->_mem, mem};
  }
};

}  // namespace arangodb::velocypack
