////////////////////////////////////////////////////////////////////////////////
/// @brief Library to build up VPack documents.
///
/// DISCLAIMER
///
/// Copyright 2015 ArangoDB GmbH, Cologne, Germany
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
/// @author Lauri Keel
/// @author Copyright 2015, ArangoDB GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef VELOCYPACK_VALUESLICE_H
#define VELOCYPACK_VALUESLICE_H 1

#include "velocypack/velocypack-common.h"
#include "velocypack/Slice.h"

namespace arangodb {
namespace velocypack {

class ValueSlice : public Slice {

  // This class provides a Slice implementation where tags are
  // transparent: they are ignored unless explicitly queried for.

 public:
  // constructor for an empty Value of type None
  constexpr ValueSlice() noexcept : Slice() {}

  // creates a ValueSlice from a pointer to a uint8_t array
  explicit constexpr ValueSlice(uint8_t const* start) noexcept
      : Slice(start) {}

  constexpr Slice const raw() const noexcept {
    return Slice(rawStart());
  }

  // pointer to the head byte, including possible tags
  constexpr uint8_t const* rawStart() const noexcept {
    return _start;
  }

  // pointer to the head byte, excluding tags
  constexpr uint8_t const* start() const noexcept {
    return valueStart();
  }

  // check if slice is a Tagged type
  constexpr bool isTagged() const noexcept {
    return SliceStaticData::TypeMap[*rawStart()] == ValueType::Tagged;
  }

  ValueSlice at(ValueLength index) const {
    return ValueSlice(Slice::at(index));
  }

  ValueSlice operator[](ValueLength index) const {
    return ValueSlice(Slice::operator[](index));
  }

  ValueSlice keyAt(ValueLength index, bool translate = true) const {
    return ValueSlice(Slice::keyAt(index, translate));
  }

  ValueSlice valueAt(ValueLength index) const {
    return ValueSlice(Slice::valueAt(index));
  }

  ValueSlice getNthValue(ValueLength index) const {
    return ValueSlice(Slice::getNthValue(index));
  }

  template<typename T>
  ValueSlice get(std::vector<T> const& attributes,
            bool resolveExternals = false) const {
    return ValueSlice(Slice::get(attributes, resolveExternals));
  }

  ValueSlice get(StringRef const& attribute) const {
    return ValueSlice(Slice::get(attribute));
  }

  ValueSlice get(std::string const& attribute) const {
    return ValueSlice(Slice::get(attribute));
  }

  ValueSlice get(char const* attribute) const {
    return ValueSlice(Slice::get(attribute));
  }

  ValueSlice get(char const* attribute, std::size_t length) const {
    return ValueSlice(Slice::get(attribute, length));
  }

  ValueSlice operator[](StringRef const& attribute) const {
    return ValueSlice(Slice::operator[](attribute));
  }

  ValueSlice operator[](std::string const& attribute) const {
    return ValueSlice(Slice::operator[](attribute));
  }

  ValueSlice resolveExternal() const {
    return ValueSlice(Slice::resolveExternal());
  }

  ValueSlice resolveExternals() const {
    return ValueSlice(Slice::resolveExternals());
  }

  ValueSlice translate() const {
    return ValueSlice(Slice::translate());
  }

  ValueSlice makeKey() const {
    return ValueSlice(Slice::makeKey());
  }

  bool binaryEquals(Slice const& other) const {
    return Slice::binaryEquals(ValueSlice(other));
  }

  bool binaryEquals(ValueSlice const& other) const {
    return Slice::binaryEquals(other);
  }

  static bool binaryEquals(uint8_t const* left, uint8_t const* right) {
    return ValueSlice(left).binaryEquals(ValueSlice(right));
  }
};

}
}

#endif

