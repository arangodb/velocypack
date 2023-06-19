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
#include <velocypack/Slice.h>

namespace arangodb::velocypack {

template<typename Allocator = std::allocator<uint8_t>>
struct BasicString : SliceBase<BasicString<Allocator>, Slice> {
  using allocator_type = Allocator;

  BasicString() = default;
  BasicString(BasicString const&) = default;
  BasicString(BasicString&&) noexcept = default;

  explicit BasicString(uint8_t const* start)
      : _str(start, Slice(start).byteSize()) {}
  BasicString(Slice s) : _str(s.getDataPtr(), s.byteSize()) {}

  // allocator aware constructors
  BasicString(BasicString const& o, allocator_type alloc)
      : _str(o._str, alloc) {}
  BasicString(BasicString&& o, allocator_type alloc)
      : _str(std::move(o._str), alloc) {}

  explicit BasicString(uint8_t const* start, allocator_type a)
      : _str(start, Slice(start).byteSize(), a) {}
  BasicString(Slice s, allocator_type a)
      : _str(s.getDataPtr(), s.byteSize(), a) {}

  BasicString& operator=(BasicString const&) = default;
  BasicString& operator=(BasicString&&) noexcept = default;

  BasicString& operator=(Slice s) {
    _str.assign(s.getDataPtr(), s.byteSize());
    return *this;
  }
  BasicString& operator=(uint8_t const* s) { return *this = Slice(s); }

  uint8_t const* getDataPtr() const noexcept { return _str.c_str(); }

  // similar to std::string we decay into Slice
  operator Slice() const { return slice(); }

  Slice slice() const { return Slice{getDataPtr()}; }
  void set(uint8_t const* s) { *this = Slice(s); }

  using StringType =
      std::basic_string<uint8_t, std::char_traits<uint8_t>, allocator_type>;
  StringType const& getUnderlyingString() const noexcept { return _str; }

 private:
  StringType _str;
};

using String = BasicString<>;

extern template struct SliceBase<String, Slice>;
}  // namespace arangodb::velocypack

using VPackString = arangodb::velocypack::String;

namespace std {
// implementation of std::hash for a String object
template<typename Allocator>
struct hash<arangodb::velocypack::BasicString<Allocator>> {
  std::size_t operator()(
      arangodb::velocypack::BasicString<Allocator> const& slice) const {
#ifdef VELOCYPACK_32BIT
    return static_cast<size_t>(slice.hash32());
#else
    return static_cast<std::size_t>(slice.hash());
#endif
  }
};
}  // namespace std
