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

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <iosfwd>
#include <iterator>
#include <limits>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <vector>

#include "velocypack/velocypack-common.h"
#include "velocypack/Exception.h"
#include "velocypack/HashedStringRef.h"
#include "velocypack/Options.h"
#include "velocypack/SliceStaticData.h"
#include "velocypack/StringRef.h"
#include "velocypack/Value.h"
#include "velocypack/ValueType.h"

namespace arangodb::velocypack {
struct Sink;

template<typename, typename = void>
struct Extractor;

template<typename DerivedType, typename SliceType = DerivedType>
struct SliceBase {
  friend class Builder;
  friend class ArrayIterator;
  friend class ObjectIterator;
  friend class ValueSlice;
  template<typename, typename>
  friend struct SliceBase;

  static constexpr uint64_t defaultSeed64 = 0xdeadbeef;
  static constexpr uint32_t defaultSeed32 = 0xdeadbeef;
  static constexpr uint64_t defaultSeed = defaultSeed64;

  std::vector<uint64_t> getTags() const {
    std::vector<uint64_t> ret;

    if (isTagged()) {
      // always need the actual first byte, so use ptr() directly
      uint8_t const* start = ptr();

      while (SliceStaticData::TypeMap[*start] == ValueType::Tagged) {
        uint8_t offset;
        uint64_t tag;

        if (*start == 0xee) {
          tag = readIntegerFixed<uint64_t, 1>(start + 1);
          offset = 2;
        } else if (*start == 0xef) {
          tag = readIntegerFixed<uint64_t, 8>(start + 1);
          offset = 9;
        } else {
          throw Exception(Exception::InternalError, "Invalid tag type ID");
        }

        ret.push_back(tag);
        start += offset;
      }
    }

    return ret;
  }

  bool hasTag(uint64_t tagId) const {
    // always need the actual first byte, so use ptr() directly
    uint8_t const* start = ptr();

    while (SliceStaticData::TypeMap[*start] == ValueType::Tagged) {
      uint8_t offset;
      uint64_t tag;

      if (*start == 0xee) {
        tag = readIntegerFixed<uint64_t, 1>(start + 1);
        offset = 2;
      } else if (*start == 0xef) {
        tag = readIntegerFixed<uint64_t, 8>(start + 1);
        offset = 9;
      } else {
        throw Exception(Exception::InternalError, "Invalid tag type ID");
      }

      if (tag == tagId) {
        return true;
      }

      start += offset;
    }

    return false;
  }

  // pointer to the head byte, excluding possible tags
  uint8_t const* valueStart() const noexcept {
    // always need the actual first byte, so use ptr() directly
    return ptr() + tagsOffset(ptr());
  }

  // pointer to the head byte, including possible tags
  // other implementations may exclude tags
  constexpr uint8_t const* start() const noexcept { return ptr(); }

  // pointer to the head byte
  template<typename T>
  T const* startAs() const {
    return reinterpret_cast<T const*>(start());
  }

  // value of the head byte
  constexpr inline uint8_t head() const noexcept { return *start(); }

  uint8_t const* begin() noexcept { return start(); }

  constexpr uint8_t const* begin() const noexcept { return start(); }

  uint8_t const* end() const { return start() + byteSize(); }

  // get the type for the slice
  constexpr inline ValueType type() const noexcept { return type(head()); }

  // get the type name for the slice
  char const* typeName() const { return valueTypeName(type()); }

  // hashes the binary representation of a value. this value is only suitable
  // to be stored in memory, but should not be persisted, as its implementation
  // may change in the future
  inline uint64_t volatileHash() const {
    std::size_t const size = checkOverflow(byteSize());
    if (size == 1) {
      uint64_t h =
          SliceStaticData::PrecalculatedHashesForDefaultSeedWYHash[head()];
      VELOCYPACK_ASSERT(h != 0);
      return h;
    }
    return VELOCYPACK_HASH_WYHASH(start(), size, defaultSeed64);
  }

  // hashes the binary representation of a value
  inline uint64_t hash(uint64_t seed = defaultSeed64) const {
    std::size_t const size = checkOverflow(byteSize());
    if (size == 1 && seed == defaultSeed64) {
      uint64_t h = SliceStaticData::PrecalculatedHashesForDefaultSeed[head()];
      VELOCYPACK_ASSERT(h != 0);
      return h;
    }
    return VELOCYPACK_HASH(start(), size, seed);
  }

  // hashes the binary representation of a value
  inline uint32_t hash32(uint32_t seed = defaultSeed32) const {
    size_t const size = checkOverflow(byteSize());
    return VELOCYPACK_HASH32(start(), size, seed);
  }

  // hashes the binary representation of a value, not using precalculated hash
  // values this is mainly here for testing purposes
  inline uint64_t hashSlow(uint64_t seed = defaultSeed64) const {
    std::size_t const size = checkOverflow(byteSize());
    return VELOCYPACK_HASH(start(), size, seed);
  }

  // hashes the value, normalizing different representations of
  // arrays, objects and numbers. this function may produce different
  // hash values than the binary hash() function
  uint64_t normalizedHash(uint64_t seed = defaultSeed64) const;

  // hashes the value, normalizing different representations of
  // arrays, objects and numbers. this function may produce different
  // hash values than the binary hash32() function
  uint32_t normalizedHash32(uint32_t seed = defaultSeed32) const;

  // hashes the binary representation of a String slice. No check
  // is done if the Slice value is actually of type String
  inline uint64_t hashString(uint64_t seed = defaultSeed64) const noexcept {
    return VELOCYPACK_HASH(start(),
                           static_cast<std::size_t>(stringSliceLength()), seed);
  }

  // hashes the binary representation of a String slice. No check
  // is done if the Slice value is actually of type String
  inline uint32_t hashString32(uint32_t seed = defaultSeed32) const noexcept {
    return VELOCYPACK_HASH32(
        start(), static_cast<std::size_t>(stringSliceLength()), seed);
  }

  // check if slice is of the specified type (including tags)
  // other implementations may be excluding tags
  constexpr inline bool isType(ValueType t) const {
    return SliceStaticData::TypeMap[*start()] == t;
  }

  // check if slice is a None object
  constexpr bool isNone() const noexcept { return isType(ValueType::None); }

  // check if slice is an Illegal object
  constexpr bool isIllegal() const noexcept {
    return isType(ValueType::Illegal);
  }

  // check if slice is a Null object
  constexpr bool isNull() const noexcept { return isType(ValueType::Null); }

  // check if slice is a Bool object
  constexpr bool isBool() const noexcept { return isType(ValueType::Bool); }

  // check if slice is a Bool object - this is an alias for isBool()
  constexpr bool isBoolean() const noexcept { return isBool(); }

  // check if slice is the Boolean value true
  constexpr bool isTrue() const noexcept { return head() == 0x1a; }

  // check if slice is the Boolean value false
  constexpr bool isFalse() const noexcept { return head() == 0x19; }

  // check if slice is an Array object
  constexpr bool isArray() const noexcept { return isType(ValueType::Array); }

  // check if slice is an Object object
  constexpr bool isObject() const noexcept { return isType(ValueType::Object); }

  // check if slice is a Double object
  constexpr bool isDouble() const noexcept { return isType(ValueType::Double); }

  // check if slice is a UTCDate object
  constexpr bool isUTCDate() const noexcept {
    return isType(ValueType::UTCDate);
  }

  // check if slice is an External object
  constexpr bool isExternal() const noexcept {
    return isType(ValueType::External);
  }

  // check if slice is a MinKey object
  constexpr bool isMinKey() const noexcept { return isType(ValueType::MinKey); }

  // check if slice is a MaxKey object
  constexpr bool isMaxKey() const noexcept { return isType(ValueType::MaxKey); }

  // check if slice is an Int object
  constexpr bool isInt() const noexcept { return isType(ValueType::Int); }

  // check if slice is a UInt object
  constexpr bool isUInt() const noexcept { return isType(ValueType::UInt); }

  // check if slice is a SmallInt object
  constexpr bool isSmallInt() const noexcept {
    return isType(ValueType::SmallInt);
  }

  // check if slice is a String object
  constexpr bool isString() const noexcept { return isType(ValueType::String); }

  // check if slice is a Binary object
  constexpr bool isBinary() const noexcept { return isType(ValueType::Binary); }

  // check if slice is a BCD
  constexpr bool isBCD() const noexcept { return isType(ValueType::BCD); }

  // check if slice is a Custom type
  constexpr bool isCustom() const noexcept { return isType(ValueType::Custom); }

  // check if slice is a Tagged type
  constexpr bool isTagged() const noexcept { return isType(ValueType::Tagged); }

  constexpr SliceType const value() const noexcept {
    return isTagged() ? make(valueStart()) : make(ptr());
  }

  constexpr uint64_t getFirstTag() const {
    // always need the actual first byte, so use ptr() directly
    return !isTagged() ? 0
                       : (*ptr() == 0xee
                              ? readIntegerFixed<uint64_t, 1>(ptr() + 1)
                              : (*ptr() == 0xef
                                     ? readIntegerFixed<uint64_t, 8>(ptr() + 1)
                                     : /* error */ 0));
  }

  // check if a slice is any number type
  constexpr bool isInteger() const noexcept {
    return (isInt() || isUInt() || isSmallInt());
  }

  // check if slice is any Number-type object
  constexpr bool isNumber() const noexcept { return isInteger() || isDouble(); }

  // check if slice is convertible to a variable of a certain
  // number type
  template<typename T>
  bool isNumber() const noexcept {
    try {
      if constexpr (std::is_integral_v<T>) {
        if constexpr (std::is_signed_v<T>) {
          // signed integral type
          if (isDouble()) {
            auto v = getDouble();
            // 9223372036854774784.0 then 9223372036854775808.0
            // (2^63 not represented in int64_t)
            constexpr auto kMax =
                sizeof(T) > 4
                    ? 9223372036854774784.0
                    : static_cast<double>((std::numeric_limits<T>::max)());
            return static_cast<double>((std::numeric_limits<T>::min)()) <= v &&
                   v <= kMax;
          }

          int64_t v = getInt();
          return (v >= static_cast<int64_t>((std::numeric_limits<T>::min)()) &&
                  v <= static_cast<int64_t>((std::numeric_limits<T>::max)()));
        } else {
          // unsigned integral type
          if (isDouble()) {
            auto v = getDouble();
            // 18446744073709549568.0 then 18446744073709551616.0
            // (2^64 not represented in uint64_t)
            constexpr auto kMax =
                sizeof(T) > 4
                    ? 18446744073709549568.0
                    : static_cast<double>((std::numeric_limits<T>::max)());
            return 0.0 <= v && v <= kMax;
          }

          // may throw if value is < 0
          uint64_t v = getUInt();
          return (v <= static_cast<uint64_t>((std::numeric_limits<T>::max)()));
        }
      } else {
        // floating point type
        return isNumber();
      }
    } catch (...) {
      // something went wrong
      return false;
    }
  }

  constexpr bool isSorted() const noexcept {
    return (head() >= 0x0b && head() <= 0x0e);
  }

  // return the value for a Bool object
  bool getBool() const {
    if (!isBool()) {
      throw Exception(Exception::InvalidValueType, "Expecting type Bool");
    }
    return isTrue();
  }

  // return the value for a Bool object - this is an alias for getBool()
  bool getBoolean() const { return getBool(); }

  // return the value for a Double object
  double getDouble() const {
    if (!isDouble()) {
      throw Exception(Exception::InvalidValueType, "Expecting type Double");
    }
    auto v = readIntegerFixed<uint64_t, 8>(start() + 1);
    double r;
    std::memcpy(&r, &v, sizeof(double));
    return r;
  }

  // extract the array value at the specified index
  // - 0x02      : array without index table (all subitems have the same
  //               byte length), bytelen 1 byte, no number of subvalues
  // - 0x03      : array without index table (all subitems have the same
  //               byte length), bytelen 2 bytes, no number of subvalues
  // - 0x04      : array without index table (all subitems have the same
  //               byte length), bytelen 4 bytes, no number of subvalues
  // - 0x05      : array without index table (all subitems have the same
  //               byte length), bytelen 8 bytes, no number of subvalues
  // - 0x06      : array with 1-byte index table entries
  // - 0x07      : array with 2-byte index table entries
  // - 0x08      : array with 4-byte index table entries
  // - 0x09      : array with 8-byte index table entries
  SliceType at(ValueLength index) const {
    if (VELOCYPACK_UNLIKELY(!isArray())) {
      throw Exception(Exception::InvalidValueType, "Expecting type Array");
    }

    return getNth(index);
  }

  SliceType operator[](ValueLength index) const { return at(index); }

  // return the number of members for an Array or Object object
  ValueLength length() const;

  // extract a key from an Object at the specified index
  // - 0x0a      : empty object
  // - 0x0b      : object with 1-byte index table entries, sorted by attribute
  // name
  // - 0x0c      : object with 2-byte index table entries, sorted by attribute
  // name
  // - 0x0d      : object with 4-byte index table entries, sorted by attribute
  // name
  // - 0x0e      : object with 8-byte index table entries, sorted by attribute
  // name
  // - 0x0f      : object with 1-byte index table entries, not sorted by
  // attribute name
  // - 0x10      : object with 2-byte index table entries, not sorted by
  // attribute name
  // - 0x11      : object with 4-byte index table entries, not sorted by
  // attribute name
  // - 0x12      : object with 8-byte index table entries, not sorted by
  // attribute name
  SliceType keyAt(ValueLength index, bool translate = true) const {
    if (VELOCYPACK_UNLIKELY(!isObject())) {
      throw Exception(Exception::InvalidValueType, "Expecting type Object");
    }

    return getNthKey(index, translate);
  }

  SliceType valueAt(ValueLength index) const;

  // extract the nth value from an Object
  SliceType getNthValue(ValueLength index) const;

  // look for the specified attribute path inside an Object
  // returns a Slice(ValueType::None) if not found
  template<typename ForwardIterator>
  typename std::enable_if<
      std::is_base_of<std::forward_iterator_tag,
                      typename std::iterator_traits<
                          ForwardIterator>::iterator_category>::value,
      SliceType>::type
  get(ForwardIterator begin, ForwardIterator end,
      bool resolveExternals = false) const {
    if (VELOCYPACK_UNLIKELY(begin == end)) {
      throw Exception(Exception::InvalidAttributePath);
    }
    // use ourselves as the starting point
    SliceType last(start());
    if (resolveExternals) {
      last = last.resolveExternal();
    }
    do {
      // fetch subattribute
      last = last.get(*begin);
      if (last.isExternal()) {
        last = last.resolveExternal();
      }
      // abort as early as possible
      if (last.isNone() || (++begin != end && !last.isObject())) {
        return SliceType();
      }
    } while (begin != end);
    return last;
  }

  // look for the specified attribute path inside an Object
  // returns a Slice(ValueType::None) if not found
  template<typename T>
  SliceType get(std::vector<T> const& attributes,
                bool resolveExternals = false) const {
    // forward to the iterator-based lookup
    return this->get(attributes.begin(), attributes.end(), resolveExternals);
  }

  // look for the specified attribute path inside an Object
  // returns a Slice(ValueType::None) if not found
  template<typename T>
  SliceType get(std::initializer_list<T> const& attributes,
                bool resolveExternals = false) const {
    // forward to the iterator-based lookup
    return this->get(attributes.begin(), attributes.end(), resolveExternals);
  }

  // look for the specified attribute inside an Object
  // returns a Slice(ValueType::None) if not found
  SliceType get(std::string_view attribute) const;

  [[deprecated]] SliceType get(HashedStringRef attribute) const {
    return get(std::string_view(attribute.data(), attribute.size()));
  }

  [[deprecated]] SliceType get(char const* attribute,
                               std::size_t length) const {
    return get(std::string_view(attribute, length));
  }

  SliceType operator[](std::string_view attribute) const {
    return get(attribute);
  }

  // whether or not an Object has a specific key
  template<typename T>
  bool hasKey(std::vector<T> const& attributes) const {
    return !this->get(attributes.begin(), attributes.end()).isNone();
  }

  // whether or not an Object has a specific key
  template<typename T>
  bool hasKey(std::initializer_list<T> const& attributes) const {
    return !this->get(attributes.begin(), attributes.end()).isNone();
  }

  // whether or not an Object has a specific key
  bool hasKey(std::string_view attribute) const {
    return !get(attribute).isNone();
  }

  // whether or not an Object has a specific key
  bool hasKey(HashedStringRef attribute) const {
    return hasKey(std::string_view(attribute.data(), attribute.size()));
  }

  [[deprecated]] bool hasKey(char const* attribute, std::size_t length) const {
    return hasKey(std::string_view(attribute, length));
  }

  // return the pointer to the data for an External object
  char const* getExternal() const {
    if (!isExternal()) {
      throw Exception(Exception::InvalidValueType, "Expecting type External");
    }
    return extractPointer();
  }

  // returns the Slice managed by an External or the Slice itself if it's not
  // an External
  SliceType resolveExternal() const {
    if (*start() == 0x1d) {
      return SliceType(reinterpret_cast<uint8_t const*>(extractPointer()));
    }
    return SliceType(start());
    ;
  }

  // returns the Slice managed by an External or the Slice itself if it's not
  // an External, recursive version
  SliceType resolveExternals() const {
    uint8_t const* current = start();
    while (*current == 0x1d) {
      current =
          reinterpret_cast<uint8_t const*>(SliceType(current).extractPointer());
    }
    return SliceType(current);
  }

  // tests whether the Slice is an empty array
  bool isEmptyArray() const { return isArray() && length() == 0; }

  // tests whether the Slice is an empty object
  bool isEmptyObject() const { return isObject() && length() == 0; }

  // translates an integer key into a string
  SliceType translate() const;

  // return the value for an Int object
  int64_t getInt() const;

  // return the value for a UInt object
  uint64_t getUInt() const;

  // return the value for a SmallInt object
  int64_t getSmallInt() const;

  template<typename T>
  T getNumber() const {
    if constexpr (std::is_integral_v<T>) {
      if constexpr (std::is_signed_v<T>) {
        // signed integral type
        if (isDouble()) {
          auto v = getDouble();
          // 9223372036854774784.0 then 9223372036854775808.0
          // (2^63 not represented in int64_t)
          constexpr auto kMax =
              sizeof(T) > 4
                  ? 9223372036854774784.0
                  : static_cast<double>((std::numeric_limits<T>::max)());
          if (v < static_cast<double>((std::numeric_limits<T>::min)()) ||
              kMax < v) {
            throw Exception(Exception::NumberOutOfRange);
          }
          return static_cast<T>(v);
        }

        int64_t v = getInt();
        if (v < static_cast<int64_t>((std::numeric_limits<T>::min)()) ||
            v > static_cast<int64_t>((std::numeric_limits<T>::max)())) {
          throw Exception(Exception::NumberOutOfRange);
        }
        return static_cast<T>(v);
      } else {
        // unsigned integral type
        if (isDouble()) {
          auto v = getDouble();
          // 18446744073709549568.0 then 18446744073709551616.0
          // (2^64 not represented in uint64_t)
          constexpr auto kMax =
              sizeof(T) > 4
                  ? 18446744073709549568.0
                  : static_cast<double>((std::numeric_limits<T>::max)());
          if (v < 0.0 || kMax < v) {
            throw Exception(Exception::NumberOutOfRange);
          }
          return static_cast<T>(v);
        }

        // may fail if number is < 0!
        uint64_t v = getUInt();
        if (v > static_cast<uint64_t>((std::numeric_limits<T>::max)())) {
          throw Exception(Exception::NumberOutOfRange);
        }
        return static_cast<T>(v);
      }
    } else {
      // floating point type

      if (isDouble()) {
        return static_cast<T>(getDouble());
      }
      if (isInt() || isSmallInt()) {
        return static_cast<T>(getIntUnchecked());
      }
      if (isUInt()) {
        return static_cast<T>(getUIntUnchecked());
      }

      throw Exception(Exception::InvalidValueType, "Expecting numeric type");
    }
  }

  // an alias for getNumber<T>
  template<typename T>
  T getNumericValue() const {
    return getNumber<T>();
  }

  // return the value for a UTCDate object
  int64_t getUTCDate() const {
    if (!isUTCDate()) {
      throw Exception(Exception::InvalidValueType, "Expecting type UTCDate");
    }
    uint64_t v = readIntegerFixed<uint64_t, sizeof(uint64_t)>(start() + 1);
    return toInt64(v);
  }

  // return the value for a String object
  char const* getString(ValueLength& length) const {
    uint8_t const h = head();
    if (h >= 0x40 && h <= 0xbe) {
      // short UTF-8 String
      length = h - 0x40;
      return reinterpret_cast<char const*>(start() + 1);
    }

    if (h == 0xbf) {
      // long UTF-8 String
      length = readIntegerFixed<ValueLength, 8>(start() + 1);
      checkOverflow(length);
      return reinterpret_cast<char const*>(start() + 1 + 8);
    }

    throw Exception(Exception::InvalidValueType, "Expecting type String");
  }

  char const* getStringUnchecked(ValueLength& length) const noexcept {
    uint8_t const h = head();
    if (h >= 0x40 && h <= 0xbe) {
      // short UTF-8 String
      length = h - 0x40;
      return reinterpret_cast<char const*>(start() + 1);
    }

    // long UTF-8 String
    length = readIntegerFixed<ValueLength, 8>(start() + 1);
    return reinterpret_cast<char const*>(start() + 1 + 8);
  }

  // return the length of the String slice
  ValueLength getStringLength() const {
    uint8_t const h = head();

    if (h >= 0x40 && h <= 0xbe) {
      // short UTF-8 String
      return h - 0x40;
    }

    if (h == 0xbf) {
      // long UTF-8 String
      return readIntegerFixed<ValueLength, 8>(start() + 1);
    }

    throw Exception(Exception::InvalidValueType, "Expecting type String");
  }

  // return a copy of the value for a String object
  std::string copyString() const {
    uint8_t h = head();
    if (h >= 0x40 && h <= 0xbe) {
      // short UTF-8 String
      ValueLength length = h - 0x40;
      return std::string(reinterpret_cast<char const*>(start() + 1),
                         static_cast<std::size_t>(length));
    }

    if (h == 0xbf) {
      ValueLength length = readIntegerFixed<ValueLength, 8>(start() + 1);
      return std::string(reinterpret_cast<char const*>(start() + 1 + 8),
                         checkOverflow(length));
    }

    throw Exception(Exception::InvalidValueType, "Expecting type String");
  }

  // return a copy of the value for a String object
  [[deprecated("use stringView")]] StringRef stringRef() const {
    auto sv = this->stringView();
    return StringRef(sv.data(), sv.size());
  }

  std::string_view stringView() const {
    uint8_t h = head();
    if (h >= 0x40 && h <= 0xbe) {
      // short UTF-8 String
      ValueLength length = h - 0x40;
      return std::string_view(reinterpret_cast<char const*>(start() + 1),
                              static_cast<std::size_t>(length));
    }

    if (h == 0xbf) {
      ValueLength length = readIntegerFixed<ValueLength, 8>(start() + 1);
      return std::string_view(reinterpret_cast<char const*>(start() + 1 + 8),
                              checkOverflow(length));
    }

    throw Exception(Exception::InvalidValueType, "Expecting type String");
  }

  // return the value for a Binary object
  uint8_t const* getBinary(ValueLength& length) const {
    if (!isBinary()) {
      throw Exception(Exception::InvalidValueType, "Expecting type Binary");
    }

    uint8_t const h = head();
    VELOCYPACK_ASSERT(h >= 0xc0 && h <= 0xc7);

    length = readIntegerNonEmpty<ValueLength>(start() + 1, h - 0xbf);
    checkOverflow(length);
    return start() + 1 + h - 0xbf;
  }

  // return the length of the Binary slice
  ValueLength getBinaryLength() const {
    if (!isBinary()) {
      throw Exception(Exception::InvalidValueType, "Expecting type Binary");
    }

    uint8_t const h = head();
    VELOCYPACK_ASSERT(h >= 0xc0 && h <= 0xc7);

    return readIntegerNonEmpty<ValueLength>(start() + 1, h - 0xbf);
  }

  // return a copy of the value for a Binary object
  std::vector<uint8_t> copyBinary() const {
    if (!isBinary()) {
      throw Exception(Exception::InvalidValueType, "Expecting type Binary");
    }

    uint8_t const h = head();
    VELOCYPACK_ASSERT(h >= 0xc0 && h <= 0xc7);

    std::vector<uint8_t> out;
    ValueLength length =
        readIntegerNonEmpty<ValueLength>(start() + 1, h - 0xbf);
    checkOverflow(length);
    out.reserve(static_cast<std::size_t>(length));
    out.insert(out.end(), start() + 1 + h - 0xbf,
               start() + 1 + h - 0xbf + length);
    return out;
  }

  // get the total byte size for the slice, including the head byte, including
  // tags
  ValueLength byteSize() const { return byteSize(start()); }

  // get the total byte size for the slice, including the head byte, excluding
  // tags
  ValueLength valueByteSize() const { return byteSize(valueStart()); }

  ValueLength findDataOffset(uint8_t head) const noexcept {
    // Must be called for a non-empty array or object at start():
    VELOCYPACK_ASSERT(head != 0x01 && head != 0x0a && head <= 0x14);
    unsigned int fsm = SliceStaticData::FirstSubMap[head];
    uint8_t const* start = this->start();
    if (fsm == 0) {
      // need to calculate the offset by reading the dynamic length
      VELOCYPACK_ASSERT(head == 0x13 || head == 0x14);
      return 1 + arangodb::velocypack::getVariableValueLength(
                     readVariableValueLength<false>(start + 1));
    }
    if (fsm <= 2 && start[2] != 0) {
      return 2;
    }
    if (fsm <= 3 && start[3] != 0) {
      return 3;
    }
    if (fsm <= 5 && start[5] != 0) {
      return 5;
    }
    return 9;
  }

  // get the offset for the nth member from an Array type
  ValueLength getNthOffset(ValueLength index) const;

  SliceType makeKey() const;

  int compareString(std::string_view value) const;

  [[deprecated]] int compareString(char const* value,
                                   std::size_t length) const {
    return compareString(std::string_view(value, length));
  }

  int compareStringUnchecked(std::string_view value) const noexcept;

  [[deprecated]] int compareStringUnchecked(char const* value,
                                            std::size_t length) const noexcept {
    return compareStringUnchecked(std::string_view(value, length));
  }

  bool isEqualString(std::string_view attribute) const;

  bool isEqualStringUnchecked(std::string_view attribute) const noexcept;

  // check if two Slices are equal on the binary level
  // please note that for several values there are multiple possible
  // representations, which differ on the binary level but will still resolve to
  // the same logical values. For example, smallint(1) and int(1) are logically
  // the same, but will resolve to either 0x31 or 0x28 0x01.
  template<typename S, typename T>
  bool binaryEquals(SliceBase<S, T> const& other) const {
    if (start() == other.start()) {
      // same underlying data, so the slices must be identical
      return true;
    }

    if (head() != other.head()) {
      return false;
    }

    ValueLength const size = byteSize();

    if (size != other.byteSize()) {
      return false;
    }

    return (std::memcmp(start(), other.start(), checkOverflow(size)) == 0);
  }

  static bool binaryEquals(uint8_t const* left, uint8_t const* right) {
    return SliceType(left).binaryEquals(SliceType(right));
  }

  // these operators are now deleted because they didn't do what people expected
  // these operators checked for _binary_ equality of the velocypack slice with
  // another. however, for several values there are multiple possible
  // representations, which differ on the binary level but will still resolve to
  // the same logical values. For example, smallint(1) and int(1) are logically
  // the same, but will resolve to either 0x31 or 0x28 0x01.
  bool operator==(Slice const& other) const = delete;
  bool operator!=(Slice const& other) const = delete;

  std::string toHex() const;
  std::string toJson(Options const* options = &Options::Defaults) const;
  std::string& toJson(std::string& out,
                      Options const* options = &Options::Defaults) const;
  void toJson(Sink* sink, Options const* options = &Options::Defaults) const;
  std::string toString(Options const* options = &Options::Defaults) const;
  std::string hexType() const;

  int64_t getIntUnchecked() const noexcept;

  // return the value for a UInt object, without checks
  // returns 0 for invalid values/types
  uint64_t getUIntUnchecked() const noexcept;

  // return the value for a SmallInt object, without checks
  // returns 0 for invalid values/types
  int64_t getSmallIntUnchecked() const noexcept;

  uint8_t const* getBCD(int8_t& sign, int32_t& exponent,
                        ValueLength& mantissaLength) const {
    if (VELOCYPACK_UNLIKELY(!isBCD())) {
      throw Exception(Exception::InvalidValueType, "Expecting type BCD");
    }

    uint64_t type = head();
    bool positive = type >= 0xc8 && type <= 0xcf;
    uint64_t mlenlen = type - (positive ? 0xc7 : 0xcf);

    sign = positive ? 1 : -1;
    exponent = static_cast<int32_t>(
        readIntegerFixed<uint32_t, 4>(valueStart() + 1 + mlenlen));
    mantissaLength =
        readIntegerNonEmpty<ValueLength>(valueStart() + 1, mlenlen);

    return valueStart() + 1 + mlenlen + 4;
  }

  template<typename... Ts>
  std::tuple<Ts...> unpackTuple() const {
    if (!isArray()) {
      throw Exception(Exception::InvalidValueType, "Expecting type Array");
    }
    auto offset = getNthOffset(0);
    auto length = arrayLength();
    if (length != sizeof...(Ts)) {
      throw Exception(Exception::BadTupleSize);
    }

    return unpackTupleInternal(unpack_helper<Ts...>{}, offset);
  }

  template<typename T>
  T extract() const {
    return Extractor<T>::extract(Slice(ptr()));
  }

 private:
  template<typename...>
  struct unpack_helper {};

  template<typename T, typename... Ts>
  std::tuple<T, Ts...> unpackTupleInternal(unpack_helper<T, Ts...>,
                                           std::size_t offset) const {
    auto slice = SliceType(this->ptr() + offset);
    return std::tuple_cat(
        std::make_tuple(slice.template extract<T>()),
        unpackTupleInternal(unpack_helper<Ts...>{}, offset + slice.byteSize()));
  }

  std::tuple<> unpackTupleInternal(unpack_helper<>, std::size_t) const {
    return std::make_tuple<>();
  }

  // get the type for the slice (including tags)
  static constexpr inline ValueType type(uint8_t h) {
    return SliceStaticData::TypeMap[h];
  }

  // return the number of members for an Array
  // must only be called for Slices that have been validated to be of type Array
  ValueLength arrayLength() const;

  // return the number of members for an Object
  // must only be called for Slices that have been validated to be of type
  // Object
  ValueLength objectLength() const {
    auto const h = head();
    VELOCYPACK_ASSERT(type(h) == ValueType::Object);

    if (h == 0x0a) {
      // special case: empty!
      return 0;
    }

    if (h == 0x14) {
      // compact Object
      ValueLength end = readVariableValueLength<false>(start() + 1);
      return readVariableValueLength<true>(start() + end - 1);
    }

    ValueLength const offsetSize = indexEntrySize(h);
    VELOCYPACK_ASSERT(offsetSize > 0);

    if (offsetSize < 8) {
      return readIntegerNonEmpty<ValueLength>(start() + offsetSize + 1,
                                              offsetSize);
    }

    ValueLength end = readIntegerNonEmpty<ValueLength>(start() + 1, offsetSize);
    return readIntegerNonEmpty<ValueLength>(start() + end - offsetSize,
                                            offsetSize);
  }

  // get the total byte size for a String slice, including the head byte.
  // no check is done if the type of the slice is actually String
  ValueLength stringSliceLength() const noexcept {
    // check if the type has a fixed length first
    auto const h = head();
    if (h == 0xbf) {
      // long UTF-8 String
      return static_cast<ValueLength>(
          1 + 8 + readIntegerFixed<ValueLength, 8>(start() + 1));
    }
    return static_cast<ValueLength>(1 + h - 0x40);
  }

  // translates an integer key into a string, without checks
  SliceType translateUnchecked() const;

  SliceType getFromCompactObject(std::string_view attribute) const;

  // extract the nth member from an Array
  SliceType getNth(ValueLength index) const;

  // extract the nth member from an Object, note that this is the nth
  // entry in the hash table for types 0x0b to 0x0e
  SliceType getNthKey(ValueLength index, bool translate) const;

  // extract the nth member from an Object, no translation
  Slice getNthKeyUntranslated(ValueLength index) const;

  // get the offset for the nth member from a compact Array or Object type
  ValueLength getNthOffsetFromCompact(ValueLength index) const;

  // get the offset for the first member from a compact Array or Object type
  // it is only valid to call this method for compact Array or Object values
  // with at least one member!!
  ValueLength getStartOffsetFromCompact() const {
    VELOCYPACK_ASSERT(head() == 0x13 || head() == 0x14);

    ValueLength end = readVariableValueLength<false>(start() + 1);
    return 1 + getVariableValueLength(end);
  }

  constexpr inline ValueLength indexEntrySize(uint8_t head) const noexcept {
    return static_cast<ValueLength>(SliceStaticData::WidthMap[head]);
  }

  // perform a linear search for the specified attribute inside an Object
  SliceType searchObjectKeyLinear(std::string_view attribute,
                                  ValueLength ieBase, ValueLength offsetSize,
                                  ValueLength n) const;

  // perform a binary search for the specified attribute inside an Object
  template<ValueLength offsetSize>
  SliceType searchObjectKeyBinary(std::string_view attribute,
                                  ValueLength ieBase, ValueLength n) const;

  // extracts a pointer from the slice and converts it into a
  // built-in pointer type
  char const* extractPointer() const {
    union Converter {
      char const* value;
      char binary[sizeof(char const*)];
    } converter;
    std::memcpy(&converter.binary[0], start() + 1, sizeof(char const*));
    return converter.value;
  }

  constexpr uint8_t tagOffset(uint8_t const* start) const noexcept {
    return SliceStaticData::TypeMap[*start] == ValueType::Tagged
               ? (*start == 0xee ? 2 : (*start == 0xef ? 9 : /* error */ 0))
               : 0;
  }

  uint8_t tagsOffset(uint8_t const* start) const noexcept {
    uint8_t ret = 0;

    while (SliceStaticData::TypeMap[*start] == ValueType::Tagged) {
      uint8_t offset = tagOffset(start);
      VELOCYPACK_ASSERT(offset != 0);
      if (VELOCYPACK_UNLIKELY(offset == 0)) {
        // prevent endless loop
        break;
      }
      ret += offset;
      start += offset;
    }

    return ret;
  }

  // get the total byte size for the slice, including the head byte
  ValueLength byteSize(uint8_t const* start) const {
    // check if the type has a fixed length first
    ValueLength l =
        static_cast<ValueLength>(SliceStaticData::FixedTypeLengths[*start]);
    if (l != 0) {
      // return fixed length
      return l;
    }
    return byteSizeDynamic(start);
  }

  ValueLength byteSizeDynamic(uint8_t const* start) const;

 private:
  DerivedType const* self() const noexcept {
    return static_cast<DerivedType const*>(this);
  }

  uint8_t const* ptr() const noexcept { return self()->getDataPtr(); }

  SliceType make(uint8_t const* mem) const {
    return SliceType::make_impl(self(), mem);
  }
  SliceType make() const {
    static_assert(std::is_default_constructible_v<SliceType>);
    return SliceType{};
  }

  template<typename T>
  static SliceType make_impl(T const*, uint8_t const* mem) {
    return SliceType{mem};
  }
};
}  // namespace arangodb::velocypack
