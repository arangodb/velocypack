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
/// @author Max Neunhoeffer
/// @author Jan Steemann
////////////////////////////////////////////////////////////////////////////////

#include <ostream>

#include "velocypack/velocypack-common.h"
#include "velocypack/AttributeTranslator.h"
#include "velocypack/Builder.h"
#include "velocypack/Dumper.h"
#include "velocypack/HexDump.h"
#include "velocypack/Iterator.h"
#include "velocypack/Parser.h"
#include "velocypack/Sink.h"
#include "velocypack/Slice.h"
#include "velocypack/ValueType.h"

namespace {

// maximum values for integers of different byte sizes
constexpr int64_t maxValues[] = {
    128,          32768,           8388608,          2147483648,
    549755813888, 140737488355328, 36028797018963968};

}  // namespace

namespace arangodb::velocypack {

// translates an integer key into a string
template<typename DerivedType, typename SliceType>
SliceType SliceBase<DerivedType, SliceType>::translate() const {
  if (VELOCYPACK_UNLIKELY(!isSmallInt() && !isUInt())) {
    throw Exception(Exception::InvalidValueType,
                    "Cannot translate key of this type");
  }
  if (Options::Defaults.attributeTranslator == nullptr) {
    throw Exception(Exception::NeedAttributeTranslator);
  }
  return translateUnchecked();
}

// return the value for a UInt object, without checks!
// returns 0 for invalid values/types
template<typename DerivedType, typename SliceType>
uint64_t SliceBase<DerivedType, SliceType>::getUIntUnchecked() const noexcept {
  uint8_t const h = head();
  if (h >= 0x28 && h <= 0x2f) {
    // UInt
    return readIntegerNonEmpty<uint64_t>(start() + 1, h - 0x27);
  }

  if (h >= 0x30 && h <= 0x39) {
    // Smallint >= 0
    return static_cast<uint64_t>(h - 0x30);
  }
  return 0;
}

// return the value for a SmallInt object
template<typename DerivedType, typename SliceType>
int64_t SliceBase<DerivedType, SliceType>::getSmallIntUnchecked()
    const noexcept {
  uint8_t const h = head();

  if (h >= 0x30 && h <= 0x39) {
    // Smallint >= 0
    return static_cast<int64_t>(h - 0x30);
  }

  if (h >= 0x3a && h <= 0x3f) {
    // Smallint < 0
    return static_cast<int64_t>(h - 0x3a) - 6;
  }

  if ((h >= 0x20 && h <= 0x27) || (h >= 0x28 && h <= 0x2f)) {
    // Int and UInt
    // we'll leave it to the compiler to detect the two ranges above are
    // adjacent
    return getIntUnchecked();
  }

  return 0;
}

// translates an integer key into a string, without checks
template<typename DerivedType, typename SliceType>
SliceType SliceBase<DerivedType, SliceType>::translateUnchecked() const {
  uint8_t const* result =
      Options::Defaults.attributeTranslator->translate(getUIntUnchecked());
  if (VELOCYPACK_LIKELY(result != nullptr)) {
    return SliceType(result);
  }
  return SliceType();
}

template<typename DerivedType, typename SliceType>
std::string SliceBase<DerivedType, SliceType>::toHex() const {
  HexDump dump{Slice(start())};
  return dump.toString();
}

template<typename DerivedType, typename SliceType>
std::string SliceBase<DerivedType, SliceType>::toJson(
    Options const* options) const {
  std::string buffer;
  StringSink sink(&buffer);
  toJson(&sink, options);
  return buffer;
}

template<typename DerivedType, typename SliceType>
std::string& SliceBase<DerivedType, SliceType>::toJson(
    std::string& out, Options const* options) const {
  // reserve some initial space in the output string to avoid
  // later reallocations. we use the Slice's byteSize as a first
  // approximation for the needed output buffer size.
  out.reserve(out.size() + byteSize());
  StringSink sink(&out);
  toJson(&sink, options);
  return out;
}

template<typename DerivedType, typename SliceType>
void SliceBase<DerivedType, SliceType>::toJson(Sink* sink,
                                               Options const* options) const {
  Dumper dumper(sink, options);
  dumper.dump(Slice{start()});
}

template<typename DerivedType, typename SliceType>
std::string SliceBase<DerivedType, SliceType>::toString(
    Options const* options) const {
  if (isString()) {
    return copyString();
  }

  // copy options and set prettyPrint in copy
  Options prettyOptions = *options;
  prettyOptions.prettyPrint = true;

  std::string buffer;
  // reserve some initial space in the output string to avoid
  // later reallocations. we use the Slice's byteSize as a first
  // approximation for the needed output buffer size.
  buffer.reserve(buffer.size() + byteSize());

  StringSink sink(&buffer);
  Dumper::dump(Slice(start()), &sink, &prettyOptions);
  return buffer;
}

template<typename DerivedType, typename SliceType>
std::string SliceBase<DerivedType, SliceType>::hexType() const {
  return HexDump::toHex(head());
}

template<typename DerivedType, typename SliceType>
uint64_t SliceBase<DerivedType, SliceType>::normalizedHash(
    uint64_t seed) const {
  uint64_t value;

  if (isNumber()) {
    // upcast integer values to double
    double v = getNumericValue<double>();
    value = VELOCYPACK_HASH(&v, sizeof(v), seed);
  } else if (isArray()) {
    // normalize arrays by hashing array length and iterating
    // over all array members
    ArrayIterator it{Slice(start())};
    uint64_t const n = it.size() ^ 0xba5bedf00d;
    value = VELOCYPACK_HASH(&n, sizeof(n), seed);
    while (it.valid()) {
      value ^= it.value().normalizedHash(value);
      it.next();
    }
  } else if (isObject()) {
    // normalize objects by hashing object length and iterating
    // over all object members
    ObjectIterator it(Slice(start()), true);
    uint64_t const n = it.size() ^ 0xf00ba44ba5;
    uint64_t seed2 = VELOCYPACK_HASH(&n, sizeof(n), seed);
    value = seed2;
    while (it.valid()) {
      auto current = (*it);
      uint64_t seed3 = current.key.normalizedHash(seed2);
      value ^= seed3;
      value ^= current.value.normalizedHash(seed3);
      it.next();
    }
  } else {
    // fall back to regular hash function
    value = hash(seed);
  }

  return value;
}

template<typename DerivedType, typename SliceType>
uint32_t SliceBase<DerivedType, SliceType>::normalizedHash32(
    uint32_t seed) const {
  uint32_t value;

  if (isNumber()) {
    // upcast integer values to double
    double v = getNumericValue<double>();
    value = VELOCYPACK_HASH32(&v, sizeof(v), seed);
  } else if (isArray()) {
    // normalize arrays by hashing array length and iterating
    // over all array members
    ArrayIterator it{Slice(start())};
    uint64_t const n = it.size() ^ 0xba5bedf00d;
    value = VELOCYPACK_HASH32(&n, sizeof(n), seed);
    while (it.valid()) {
      value ^= it.value().normalizedHash32(value);
      it.next();
    }
  } else if (isObject()) {
    // normalize objects by hashing object length and iterating
    // over all object members
    ObjectIterator it(Slice(start()), true);
    uint32_t const n = static_cast<uint32_t>(it.size() ^ 0xf00ba44ba5);
    uint32_t seed2 = VELOCYPACK_HASH32(&n, sizeof(n), seed);
    value = seed2;
    while (it.valid()) {
      auto current = (*it);
      uint32_t seed3 = current.key.normalizedHash32(seed2);
      value ^= seed3;
      value ^= current.value.normalizedHash32(seed3);
      it.next();
    }
  } else {
    // fall back to regular hash function
    value = hash32(seed);
  }

  return value;
}

// look for the specified attribute inside an Object
// returns a Slice(ValueType::None) if not found
template<typename DerivedType, typename SliceType>
SliceType SliceBase<DerivedType, SliceType>::get(
    std::string_view attribute) const {
  if (VELOCYPACK_UNLIKELY(!isObject())) {
    throw Exception(Exception::InvalidValueType, "Expecting Object");
  }

  auto const h = head();
  if (h == 0x0a) {
    // special case, empty object
    return SliceType();
  }

  if (h == 0x14) {
    // compact Object
    return getFromCompactObject(attribute);
  }

  ValueLength const offsetSize = indexEntrySize(h);
  VELOCYPACK_ASSERT(offsetSize > 0);
  ValueLength end = readIntegerNonEmpty<ValueLength>(start() + 1, offsetSize);

  // read number of items
  ValueLength n;
  ValueLength ieBase;
  if (offsetSize < 8) {
    n = readIntegerNonEmpty<ValueLength>(start() + 1 + offsetSize, offsetSize);
    ieBase = end - n * offsetSize;
  } else {
    n = readIntegerNonEmpty<ValueLength>(start() + end - offsetSize,
                                         offsetSize);
    ieBase = end - n * offsetSize - offsetSize;
  }

  if (n == 1) {
    // Just one attribute, there is no index table!
    Slice key(start() + findDataOffset(h));

    if (key.isString()) {
      if (key.isEqualStringUnchecked(attribute)) {
        return make(key.start() + key.byteSize());
      }
      // fall through to returning None Slice below
    } else if (key.isSmallInt() || key.isUInt()) {
      // translate key
      if (Options::Defaults.attributeTranslator == nullptr) {
        throw Exception(Exception::NeedAttributeTranslator);
      }
      if (key.translateUnchecked().isEqualString(attribute)) {
        return make(key.start() + key.byteSize());
      }
    }

    // no match or invalid key type
    return make();
  }

  // only use binary search for attributes if we have at least this many entries
  // otherwise we'll always use the linear search
  constexpr ValueLength SortedSearchEntriesThreshold = 4;

  if (n >= SortedSearchEntriesThreshold && (h >= 0x0b && h <= 0x0e)) {
    switch (offsetSize) {
      case 1:
        return searchObjectKeyBinary<1>(attribute, ieBase, n);
      case 2:
        return searchObjectKeyBinary<2>(attribute, ieBase, n);
      case 4:
        return searchObjectKeyBinary<4>(attribute, ieBase, n);
      case 8:
        return searchObjectKeyBinary<8>(attribute, ieBase, n);
      default: {
      }
    }
  }

  return searchObjectKeyLinear(attribute, ieBase, offsetSize, n);
}

// return the value for an Int object
template<typename DerivedType, typename SliceType>
int64_t SliceBase<DerivedType, SliceType>::getIntUnchecked() const noexcept {
  uint8_t const h = head();

  if (h >= 0x20 && h <= 0x27) {
    // Int  T
    uint64_t v = readIntegerNonEmpty<uint64_t>(start() + 1, h - 0x1f);
    if (h == 0x27) {
      return toInt64(v);
    } else {
      int64_t vv = static_cast<int64_t>(v);
      int64_t shift = ::maxValues[h - 0x20];
      return vv < shift ? vv : vv - (shift << 1);
    }
  }

  // SmallInt
  VELOCYPACK_ASSERT(h >= 0x30 && h <= 0x3f);
  return getSmallIntUnchecked();
}

// return the value for an Int object
template<typename DerivedType, typename SliceType>
int64_t SliceBase<DerivedType, SliceType>::getInt() const {
  uint8_t const h = head();

  if (h >= 0x20 && h <= 0x27) {
    // Int  T
    uint64_t v = readIntegerNonEmpty<uint64_t>(start() + 1, h - 0x1f);
    if (h == 0x27) {
      return toInt64(v);
    } else {
      int64_t vv = static_cast<int64_t>(v);
      int64_t shift = ::maxValues[h - 0x20];
      return vv < shift ? vv : vv - (shift << 1);
    }
  }

  if (h >= 0x28 && h <= 0x2f) {
    // UInt
    uint64_t v = getUIntUnchecked();
    if (v > static_cast<uint64_t>(INT64_MAX)) {
      throw Exception(Exception::NumberOutOfRange);
    }
    return static_cast<int64_t>(v);
  }

  if (h >= 0x30 && h <= 0x3f) {
    // SmallInt
    return getSmallIntUnchecked();
  }

  throw Exception(Exception::InvalidValueType, "Expecting type Int");
}

// return the value for a UInt object
template<typename DerivedType, typename SliceType>
uint64_t SliceBase<DerivedType, SliceType>::getUInt() const {
  uint8_t const h = head();
  if (h == 0x28) {
    // single byte integer
    return readIntegerFixed<uint64_t, 1>(start() + 1);
  }

  if (h >= 0x29 && h <= 0x2f) {
    // UInt
    return readIntegerNonEmpty<uint64_t>(start() + 1, h - 0x27);
  }

  if (h >= 0x20 && h <= 0x27) {
    // Int
    int64_t v = getInt();
    if (v < 0) {
      throw Exception(Exception::NumberOutOfRange);
    }
    return static_cast<int64_t>(v);
  }

  if (h >= 0x30 && h <= 0x39) {
    // Smallint >= 0
    return static_cast<uint64_t>(h - 0x30);
  }

  if (h >= 0x3a && h <= 0x3f) {
    // Smallint < 0
    throw Exception(Exception::NumberOutOfRange);
  }

  throw Exception(Exception::InvalidValueType, "Expecting type UInt");
}

// return the value for a SmallInt object
template<typename DerivedType, typename SliceType>
int64_t SliceBase<DerivedType, SliceType>::getSmallInt() const {
  uint8_t const h = head();

  if (h >= 0x30 && h <= 0x39) {
    // Smallint >= 0
    return static_cast<int64_t>(h - 0x30);
  }

  if (h >= 0x3a && h <= 0x3f) {
    // Smallint < 0
    return static_cast<int64_t>(h - 0x3a) - 6;
  }

  if ((h >= 0x20 && h <= 0x27) || (h >= 0x28 && h <= 0x2f)) {
    // Int and UInt
    // we'll leave it to the compiler to detect the two ranges above are
    // adjacent
    return getInt();
  }

  throw Exception(Exception::InvalidValueType, "Expecting type SmallInt");
}

template<typename DerivedType, typename SliceType>
int SliceBase<DerivedType, SliceType>::compareString(
    std::string_view value) const {
  std::size_t const length = value.size();
  ValueLength keyLength;
  char const* k = getString(keyLength);
  std::size_t const compareLength =
      (std::min)(static_cast<std::size_t>(keyLength), length);
  int res = std::memcmp(k, value.data(), compareLength);

  if (res == 0) {
    return static_cast<int>(keyLength - length);
  }
  return res;
}

template<typename DerivedType, typename SliceType>
int SliceBase<DerivedType, SliceType>::compareStringUnchecked(
    std::string_view value) const noexcept {
  std::size_t const length = value.size();
  ValueLength keyLength;
  char const* k = getStringUnchecked(keyLength);
  std::size_t const compareLength =
      (std::min)(static_cast<std::size_t>(keyLength), length);
  int res = std::memcmp(k, value.data(), compareLength);

  if (res == 0) {
    return static_cast<int>(keyLength - length);
  }
  return res;
}

template<typename DerivedType, typename SliceType>
bool SliceBase<DerivedType, SliceType>::isEqualString(
    std::string_view attribute) const {
  ValueLength keyLength;
  char const* k = getString(keyLength);
  return (static_cast<std::size_t>(keyLength) == attribute.size()) &&
         (std::memcmp(k, attribute.data(), attribute.size()) == 0);
}

template<typename DerivedType, typename SliceType>
bool SliceBase<DerivedType, SliceType>::isEqualStringUnchecked(
    std::string_view attribute) const noexcept {
  ValueLength keyLength;
  char const* k = getStringUnchecked(keyLength);
  return (static_cast<std::size_t>(keyLength) == attribute.size()) &&
         (std::memcmp(k, attribute.data(), attribute.size()) == 0);
}

template<typename DerivedType, typename SliceType>
SliceType SliceBase<DerivedType, SliceType>::getFromCompactObject(
    std::string_view attribute) const {
  ObjectIterator it(Slice(start()), /*useSequentialIteration*/ false);
  while (it.valid()) {
    Slice key = it.key(false);
    if (key.makeKey().isEqualString(attribute)) {
      return SliceType(key.start() + key.byteSize());
    }

    it.next();
  }
  // not found
  return SliceType();
}

// get the offset for the nth member from an Array or Object type
template<typename DerivedType, typename SliceType>
ValueLength SliceBase<DerivedType, SliceType>::getNthOffset(
    ValueLength index) const {
  VELOCYPACK_ASSERT(isArray() || isObject());

  auto const h = head();

  if (h == 0x13 || h == 0x14) {
    // compact Array or Object
    return getNthOffsetFromCompact(index);
  }

  if (VELOCYPACK_UNLIKELY(h == 0x01 || h == 0x0a)) {
    // special case: empty Array or empty Object
    throw Exception(Exception::IndexOutOfBounds);
  }

  ValueLength const offsetSize = indexEntrySize(h);
  ValueLength end = readIntegerNonEmpty<ValueLength>(start() + 1, offsetSize);

  ValueLength dataOffset = 0;

  // find the number of items
  ValueLength n;
  if (h <= 0x05) {  // No offset table or length, need to compute:
    VELOCYPACK_ASSERT(h != 0x00 && h != 0x01);
    dataOffset = findDataOffset(h);
    Slice first(start() + dataOffset);
    ValueLength s = first.byteSize();
    if (VELOCYPACK_UNLIKELY(s == 0)) {
      throw Exception(Exception::InternalError,
                      "Invalid data for compact object");
    }
    n = (end - dataOffset) / s;
  } else if (offsetSize < 8) {
    n = readIntegerNonEmpty<ValueLength>(start() + 1 + offsetSize, offsetSize);
  } else {
    n = readIntegerNonEmpty<ValueLength>(start() + end - offsetSize,
                                         offsetSize);
  }

  if (index >= n) {
    throw Exception(Exception::IndexOutOfBounds);
  }

  // empty array case was already covered
  VELOCYPACK_ASSERT(n > 0);

  if (h <= 0x05 || n == 1) {
    // no index table, but all array items have the same length
    // now fetch first item and determine its length
    if (dataOffset == 0) {
      VELOCYPACK_ASSERT(h != 0x00 && h != 0x01);
      dataOffset = findDataOffset(h);
    }
    return dataOffset + index * Slice(start() + dataOffset).byteSize();
  }

  ValueLength const ieBase =
      end - n * offsetSize + index * offsetSize - (offsetSize == 8 ? 8 : 0);
  return readIntegerNonEmpty<ValueLength>(start() + ieBase, offsetSize);
}

// extract the nth member from an Array
template<typename DerivedType, typename SliceType>
SliceType SliceBase<DerivedType, SliceType>::getNth(ValueLength index) const {
  VELOCYPACK_ASSERT(isArray());

  return make(start() + getNthOffset(index));
}

// extract the nth member from an Object
template<typename DerivedType, typename SliceType>
SliceType SliceBase<DerivedType, SliceType>::getNthKey(ValueLength index,
                                                       bool translate) const {
  VELOCYPACK_ASSERT(type() == ValueType::Object);

  Slice s(start() + getNthOffset(index));

  if (translate) {
    return make(s.makeKey().getDataPtr());
  }

  return make(s.getDataPtr());
}

template<typename DerivedType, typename SliceType>
SliceType SliceBase<DerivedType, SliceType>::makeKey() const {
  if (isString()) {
    return SliceType(start());
  }
  if (isSmallInt() || isUInt()) {
    if (VELOCYPACK_UNLIKELY(Options::Defaults.attributeTranslator == nullptr)) {
      throw Exception(Exception::NeedAttributeTranslator);
    }
    return translateUnchecked();
  }

  throw Exception(Exception::InvalidValueType,
                  "Cannot translate key of this type");
}

// get the offset for the nth member from a compact Array or Object type
template<typename DerivedType, typename SliceType>
ValueLength SliceBase<DerivedType, SliceType>::getNthOffsetFromCompact(
    ValueLength index) const {
  auto const h = head();
  VELOCYPACK_ASSERT(h == 0x13 || h == 0x14);

  ValueLength end = readVariableValueLength<false>(start() + 1);
  ValueLength n = readVariableValueLength<true>(start() + end - 1);
  if (VELOCYPACK_UNLIKELY(index >= n)) {
    throw Exception(Exception::IndexOutOfBounds);
  }

  ValueLength offset = 1 + getVariableValueLength(end);
  ValueLength current = 0;
  while (current != index) {
    uint8_t const* s = start() + offset;
    offset += Slice(s).byteSize();
    if (h == 0x14) {
      offset += Slice(start() + offset).byteSize();
    }
    ++current;
  }
  return offset;
}

// perform a linear search for the specified attribute inside an Object
template<typename DerivedType, typename SliceType>
SliceType SliceBase<DerivedType, SliceType>::searchObjectKeyLinear(
    std::string_view attribute, ValueLength ieBase, ValueLength offsetSize,
    ValueLength n) const {
  bool const useTranslator = (Options::Defaults.attributeTranslator != nullptr);

  for (ValueLength index = 0; index < n; ++index) {
    ValueLength offset = ieBase + index * offsetSize;
    Slice key(start() +
              readIntegerNonEmpty<ValueLength>(start() + offset, offsetSize));

    if (key.isString()) {
      if (!key.isEqualStringUnchecked(attribute)) {
        continue;
      }
    } else if (key.isSmallInt() || key.isUInt()) {
      // translate key
      if (VELOCYPACK_UNLIKELY(!useTranslator)) {
        // no attribute translator
        throw Exception(Exception::NeedAttributeTranslator);
      }
      if (!key.translateUnchecked().isEqualString(attribute)) {
        continue;
      }
    } else {
      // invalid key type
      return make();
    }

    // key is identical. now return value
    return make(key.start() + key.byteSize());
  }

  // nothing found
  return make();
}

// perform a binary search for the specified attribute inside an Object
template<typename DerivedType, typename SliceType>
template<ValueLength offsetSize>
SliceType SliceBase<DerivedType, SliceType>::searchObjectKeyBinary(
    std::string_view attribute, ValueLength ieBase, ValueLength n) const {
  bool const useTranslator = (Options::Defaults.attributeTranslator != nullptr);
  VELOCYPACK_ASSERT(n > 0);

  int64_t l = 0;
  int64_t r = static_cast<int64_t>(n) - 1;
  int64_t index = r / 2;

  do {
    ValueLength offset = ieBase + index * offsetSize;
    Slice key(start() +
              readIntegerFixed<ValueLength, offsetSize>(start() + offset));

    int res;
    if (key.isString()) {
      res = key.compareStringUnchecked(attribute);
    } else {
      VELOCYPACK_ASSERT(key.isSmallInt() || key.isUInt());
      // translate key
      if (VELOCYPACK_UNLIKELY(!useTranslator)) {
        // no attribute translator
        throw Exception(Exception::NeedAttributeTranslator);
      }
      res = key.translateUnchecked().compareString(attribute);
    }

    if (res > 0) {
      r = index - 1;
    } else if (res == 0) {
      // found. now return a Slice pointing at the value
      return make(key.start() + key.byteSize());
    } else {
      l = index + 1;
    }

    // determine new midpoint
    index = l + ((r - l) / 2);
  } while (r >= l);

  // not found
  return SliceType();
}

template<typename DerivedType, typename SliceType>
Slice SliceBase<DerivedType, SliceType>::getNthKeyUntranslated(
    ValueLength index) const {
  VELOCYPACK_ASSERT(type() == ValueType::Object);
  return Slice(start() + getNthOffset(index));
}

template<typename DerivedType, typename SliceType>
SliceType SliceBase<DerivedType, SliceType>::getNthValue(
    ValueLength index) const {
  Slice key = getNthKeyUntranslated(index);
  return make(key.start() + key.byteSize());
}

template<typename DerivedType, typename SliceType>
SliceType SliceBase<DerivedType, SliceType>::valueAt(ValueLength index) const {
  if (VELOCYPACK_UNLIKELY(!isObject())) {
    throw Exception(Exception::InvalidValueType, "Expecting type Object");
  }

  Slice key = getNthKeyUntranslated(index);
  return make(key.start() + key.byteSize());
}

template<typename DerivedType, typename SliceType>
ValueLength SliceBase<DerivedType, SliceType>::byteSizeDynamic(
    uint8_t const* start) const {
  uint8_t h = *start;

  // types with dynamic lengths need special treatment:
  switch (type(h)) {
    case ValueType::Array:
    case ValueType::Object: {
      if (h == 0x13 || h == 0x14) {
        // compact Array or Object
        return readVariableValueLength<false>(start + 1);
      }

      VELOCYPACK_ASSERT(h > 0x01 && h <= 0x0e && h != 0x0a);
      if (VELOCYPACK_UNLIKELY(h >= sizeof(SliceStaticData::WidthMap) /
                                       sizeof(SliceStaticData::WidthMap[0]))) {
        throw Exception(Exception::InternalError, "invalid Array/Object type");
      }
      return readIntegerNonEmpty<ValueLength>(start + 1,
                                              SliceStaticData::WidthMap[h]);
    }

    case ValueType::String: {
      VELOCYPACK_ASSERT(h == 0xbf);

      if (VELOCYPACK_UNLIKELY(h < 0xbf)) {
        // we cannot get here, because the FixedTypeLengths lookup
        // above will have kicked in already. however, the compiler
        // claims we'll be reading across the bounds of the input
        // here...
        return h - 0x40;
      }

      // long UTF-8 String
      return static_cast<ValueLength>(
          1 + 8 + readIntegerFixed<ValueLength, 8>(start + 1));
    }

    case ValueType::Binary: {
      VELOCYPACK_ASSERT(h >= 0xc0 && h <= 0xc7);
      return static_cast<ValueLength>(
          1 + h - 0xbf + readIntegerNonEmpty<ValueLength>(start + 1, h - 0xbf));
    }

    case ValueType::BCD: {
      if (h <= 0xcf) {
        // positive BCD
        VELOCYPACK_ASSERT(h >= 0xc8 && h < 0xcf);
        return static_cast<ValueLength>(
            1 + h - 0xc7 +
            readIntegerNonEmpty<ValueLength>(start + 1, h - 0xc7));
      }

      // negative BCD
      VELOCYPACK_ASSERT(h >= 0xd0 && h < 0xd7);
      return static_cast<ValueLength>(
          1 + h - 0xcf + readIntegerNonEmpty<ValueLength>(start + 1, h - 0xcf));
    }

    case ValueType::Tagged: {
      uint8_t offset = tagsOffset(start);
      if (VELOCYPACK_UNLIKELY(offset == 0)) {
        throw Exception(Exception::InternalError,
                        "Invalid tag data in byteSize()");
      }
      return byteSize(start + offset) + offset;
    }

    case ValueType::Custom: {
      VELOCYPACK_ASSERT(h >= 0xf4);
      switch (h) {
        case 0xf4:
        case 0xf5:
        case 0xf6: {
          return 2 + readIntegerFixed<ValueLength, 1>(start + 1);
        }

        case 0xf7:
        case 0xf8:
        case 0xf9: {
          return 3 + readIntegerFixed<ValueLength, 2>(start + 1);
        }

        case 0xfa:
        case 0xfb:
        case 0xfc: {
          return 5 + readIntegerFixed<ValueLength, 4>(start + 1);
        }

        case 0xfd:
        case 0xfe:
        case 0xff: {
          return 9 + readIntegerFixed<ValueLength, 8>(start + 1);
        }

        default: {
          // fallthrough intentional
        }
      }
    }
    default: {
      // fallthrough intentional
    }
  }

  throw Exception(Exception::InternalError, "Invalid type for byteSize()");
}

template<typename DerivedType, typename SliceType>
ValueLength SliceBase<DerivedType, SliceType>::arrayLength() const {
  auto const h = head();
  VELOCYPACK_ASSERT(type(h) == ValueType::Array);

  if (h == 0x01) {
    // special case: empty!
    return 0;
  }

  if (h == 0x13) {
    // compact Array
    ValueLength end = readVariableValueLength<false>(start() + 1);
    return readVariableValueLength<true>(start() + end - 1);
  }

  ValueLength const offsetSize = indexEntrySize(h);
  VELOCYPACK_ASSERT(offsetSize > 0);

  // find number of items
  if (h <= 0x05) {  // No offset table or length, need to compute:
    VELOCYPACK_ASSERT(h != 0x00 && h != 0x01);
    ValueLength firstSubOffset = findDataOffset(h);
    Slice first(start() + firstSubOffset);
    ValueLength s = first.byteSize();
    if (VELOCYPACK_UNLIKELY(s == 0)) {
      throw Exception(Exception::InternalError, "Invalid data for Array");
    }
    ValueLength end = readIntegerNonEmpty<ValueLength>(start() + 1, offsetSize);
    return (end - firstSubOffset) / s;
  } else if (offsetSize < 8) {
    return readIntegerNonEmpty<ValueLength>(start() + offsetSize + 1,
                                            offsetSize);
  }

  ValueLength end = readIntegerNonEmpty<ValueLength>(start() + 1, offsetSize);
  return readIntegerNonEmpty<ValueLength>(start() + end - offsetSize,
                                          offsetSize);
}

template<typename DerivedType, typename SliceType>
ValueLength SliceBase<DerivedType, SliceType>::length() const {
  if (VELOCYPACK_UNLIKELY(!isArray() && !isObject())) {
    throw Exception(Exception::InvalidValueType,
                    "Expecting type Array or Object");
  }

  auto const h = head();
  if (h == 0x01 || h == 0x0a) {
    // special case: empty!
    return 0;
  }

  if (h == 0x13 || h == 0x14) {
    // compact Array or Object
    ValueLength end = readVariableValueLength<false>(start() + 1);
    return readVariableValueLength<true>(start() + end - 1);
  }

  ValueLength const offsetSize = indexEntrySize(h);
  VELOCYPACK_ASSERT(offsetSize > 0);
  ValueLength end = readIntegerNonEmpty<ValueLength>(start() + 1, offsetSize);

  // find number of items
  if (h <= 0x05) {  // No offset table or length, need to compute:
    VELOCYPACK_ASSERT(h != 0x00 && h != 0x01);
    ValueLength firstSubOffset = findDataOffset(h);
    Slice first(start() + firstSubOffset);
    ValueLength s = first.byteSize();
    if (VELOCYPACK_UNLIKELY(s == 0)) {
      throw Exception(Exception::InternalError, "Invalid data for Array");
    }
    return (end - firstSubOffset) / s;
  } else if (offsetSize < 8) {
    return readIntegerNonEmpty<ValueLength>(start() + offsetSize + 1,
                                            offsetSize);
  }

  return readIntegerNonEmpty<ValueLength>(start() + end - offsetSize,
                                          offsetSize);
}
}  // namespace arangodb::velocypack
#define INSTANTIATE_TYPE(Derived, SliceType)                                  \
  template struct SliceBase<Derived, SliceType>;                              \
  template SliceType SliceBase<Derived, SliceType>::searchObjectKeyBinary<1>( \
      std::string_view attribute, ValueLength ieBase, ValueLength n) const;   \
  template SliceType SliceBase<Derived, SliceType>::searchObjectKeyBinary<2>( \
      std::string_view attribute, ValueLength ieBase, ValueLength n) const;   \
  template SliceType SliceBase<Derived, SliceType>::searchObjectKeyBinary<4>( \
      std::string_view attribute, ValueLength ieBase, ValueLength n) const;   \
  template SliceType SliceBase<Derived, SliceType>::searchObjectKeyBinary<8>( \
      std::string_view attribute, ValueLength ieBase, ValueLength n) const;
