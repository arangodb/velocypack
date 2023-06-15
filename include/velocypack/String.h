////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2023 ArangoDB GmbH, Cologne, Germany
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
/// @author Lars Maier
////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <memory_resource>

#include <velocypack/Slice.h>

namespace arangodb::velocypack {

template<typename Allocator = std::allocator<uint8_t>>
class BasicString : public SliceBase<BasicString<Allocator>, Slice> {
 public:
  explicit BasicString(uint8_t const* start)
      : _mem(start, Slice(start).byteSize()) {}
  explicit BasicString(Slice s) : _mem(s.getDataPtr(), s.byteSize()) {}
  BasicString(BasicString const&) = default;
  BasicString(BasicString&&) noexcept = default;

  uint8_t const* getDataPtr() const noexcept { return _mem.c_str(); }

  // similar to std::string we decay into Slice
  Slice slice() { return Slice{getDataPtr()}; }
  operator Slice() { return slice(); }

 private:
  std::basic_string<uint8_t, std::char_traits<uint8_t>, Allocator> _mem;
};

using String = BasicString<>;

namespace pmr {
using String = BasicString<std::pmr::polymorphic_allocator<uint8_t>>;
}

using VPackString = arangodb::velocypack::String;

extern template struct SliceBase<String, Slice>;
extern template struct SliceBase<pmr::String, Slice>;
}  // namespace arangodb::velocypack

namespace std {
// implementation of std::hash for a String object
template<>
struct hash<arangodb::velocypack::String> {
  std::size_t operator()(arangodb::velocypack::String const& slice) const {
#ifdef VELOCYPACK_32BIT
    return static_cast<size_t>(slice.hash32());
#else
    return static_cast<std::size_t>(slice.hash());
#endif
  }
};
}  // namespace std
