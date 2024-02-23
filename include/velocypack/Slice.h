////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2024 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Business Source License 1.1 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     https://github.com/arangodb/arangodb/blob/devel/LICENSE
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Max Neunhoeffer
/// @author Jan Steemann
////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <velocypack/SliceBase.h>

namespace arangodb::velocypack {

// This class provides read only access to a VPack value, it is
// intentionally light-weight (only one pointer value), such that
// it can easily be used to traverse larger VPack values.

// A Slice does not own the VPack data it points to!
class Slice : public SliceBase<Slice> {
  friend class Builder;
  friend class ArrayIterator;
  friend class ObjectIterator;
  friend class ValueSlice;

  // ptr() is the pointer to the first byte of the value. It should always be
  // accessed through the start() method as that allows subclasses to adjust
  // the start position should it be necessary. For example, the ValueSlice
  // class uses this to make tags transparent and behaves as if they did not
  // exist, unless explicitly queried for.
  uint8_t const* _start;

 public:
  static uint8_t const noneSliceData[];
  static uint8_t const illegalSliceData[];
  static uint8_t const nullSliceData[];
  static uint8_t const falseSliceData[];
  static uint8_t const trueSliceData[];
  static uint8_t const zeroSliceData[];
  static uint8_t const emptyStringSliceData[];
  static uint8_t const emptyArraySliceData[];
  static uint8_t const emptyObjectSliceData[];
  static uint8_t const minKeySliceData[];
  static uint8_t const maxKeySliceData[];

  // constructor for an empty Value of type None
  constexpr Slice() noexcept : Slice(noneSliceData) {}

  // creates a Slice from a pointer to a uint8_t array
  explicit constexpr Slice(uint8_t const* start) noexcept : _start(start) {}

  // No destructor, does not take part in memory management

  // Set new memory position
  void set(uint8_t const* s) { _start = s; }

  // creates a slice of type None
  static constexpr Slice noneSlice() noexcept { return Slice(noneSliceData); }

  // creates a slice of type Illegal
  static constexpr Slice illegalSlice() noexcept {
    return Slice(illegalSliceData);
  }

  // creates a slice of type Null
  static constexpr Slice nullSlice() noexcept { return Slice(nullSliceData); }

  // creates a slice of type Boolean with the relevant value
  static constexpr Slice booleanSlice(bool value) noexcept {
    return value ? trueSlice() : falseSlice();
  }

  // creates a slice of type Boolean with false value
  static constexpr Slice falseSlice() noexcept { return Slice(falseSliceData); }

  // creates a slice of type Boolean with true value
  static constexpr Slice trueSlice() noexcept { return Slice(trueSliceData); }

  // creates a slice of type Smallint(0)
  static constexpr Slice zeroSlice() noexcept { return Slice(zeroSliceData); }

  // creates a slice of type String, empty
  static constexpr Slice emptyStringSlice() noexcept {
    return Slice(emptyStringSliceData);
  }

  // creates a slice of type Array, empty
  static constexpr Slice emptyArraySlice() noexcept {
    return Slice(emptyArraySliceData);
  }

  // creates a slice of type Object, empty
  static constexpr Slice emptyObjectSlice() noexcept {
    return Slice(emptyObjectSliceData);
  }

  // creates a slice of type MinKey
  static constexpr Slice minKeySlice() noexcept {
    return Slice(minKeySliceData);
  }

  // creates a slice of type MaxKey
  static constexpr Slice maxKeySlice() noexcept {
    return Slice(maxKeySliceData);
  }

  uint8_t const* getDataPtr() const noexcept { return _start; }
};

static_assert(!std::is_polymorphic<Slice>::value,
              "Slice must not be polymorphic");
static_assert(!std::has_virtual_destructor<Slice>::value,
              "Slice must not have virtual dtor");

extern template struct SliceBase<Slice, Slice>;

template<typename T, typename>
struct Extractor {
  template<typename>
  struct always_false : std::false_type {};
  static_assert(always_false<T>::value, "There is no extractor for this type.");
};

template<>
struct Extractor<Slice> {
  static Slice extract(Slice slice) { return slice; }
};

template<>
struct Extractor<std::string> {
  static std::string extract(Slice slice) { return slice.copyString(); }
};

template<>
struct Extractor<std::string_view> {
  static std::string_view extract(Slice slice) { return slice.stringView(); }
};

template<>
struct Extractor<bool> {
  static bool extract(Slice slice) { return slice.getBool(); }
};

template<typename T>
struct Extractor<T,
                 typename std::enable_if<std::is_arithmetic<T>::value>::type> {
  static T extract(Slice slice) { return slice.template getNumericValue<T>(); }
};

template<typename... Ts>
struct Extractor<std::tuple<Ts...>> {
  static std::tuple<Ts...> extract(Slice slice) {
    return slice.unpackTuple<Ts...>();
  }
};

std::ostream& operator<<(std::ostream&, Slice const*);
std::ostream& operator<<(std::ostream&, Slice const&);

}  // namespace arangodb::velocypack

namespace std {
// implementation of std::hash for a Slice object
template<>
struct hash<arangodb::velocypack::Slice> {
  std::size_t operator()(arangodb::velocypack::Slice const& slice) const {
#ifdef VELOCYPACK_32BIT
    return static_cast<size_t>(slice.hash32());
#else
    return static_cast<std::size_t>(slice.hash());
#endif
  }
};
}  // namespace std

using VPackSlice = arangodb::velocypack::Slice;
