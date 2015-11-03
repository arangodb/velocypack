////////////////////////////////////////////////////////////////////////////////
/// @brief Library to build up Jason documents.
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
/// @author Max Neunhoeffer
/// @author Jan Steemann
/// @author Copyright 2015, ArangoDB GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef JASON_SLICE_H
#define JASON_SLICE_H 1

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <ostream>
#include <functional>
#include <algorithm>

#include "Jason.h"
#include "JasonException.h"
#include "JasonType.h"

namespace arangodb {
  namespace jason {

    class JasonSlice {

      // This class provides read only access to a Jason value, it is
      // intentionally light-weight (only one pointer value), such that
      // it can easily be used to traverse larger Jason values.

        friend class JasonBuilder;

        uint8_t const* _start;

      public:
 
        // constructor for an empty Jason of type None 
        JasonSlice () 
          : JasonSlice("\x00") {
        }

        explicit JasonSlice (uint8_t const* start) 
          : _start(start) {
        }

        explicit JasonSlice (char const* start) 
          : _start(reinterpret_cast<uint8_t const*>(start)) {
        }

        // No destructor, does not take part in memory management,
        // standard copy, and move constructors, behaves like a pointer.

        // get the type for the slice
        inline JasonType type () const {
          return TypeMap[head()];
        }

        char const* typeName () const {
          return JasonTypeName(type());
        }

        // pointer to the head byte
        uint8_t const* start () const {
          return _start;
        }

        // value of the head byte
        inline uint8_t head () const {
          return *_start;
        }

        // check if slice is of the specified type
        inline bool isType (JasonType t) const {
          return type() == t;
        }

        // check if slice is a None object
        bool isNone () const {
          return isType(JasonType::None);
        }

        // check if slice is a Null object
        bool isNull () const {
          return isType(JasonType::Null);
        }

        // check if slice is a Bool object
        bool isBool () const {
          return isType(JasonType::Bool);
        }

        // check if slice is a Bool object - this is an alias for isBool()
        bool isBoolean () const {
          return isBool();
        }

        // check if slice is an Array object
        bool isArray () const {
          return isType(JasonType::Array);
        }

        // check if slice is an Object object
        bool isObject () const {
          return isType(JasonType::Object);
        }
        
        // check if slice is a Double object
        bool isDouble () const {
          return isType(JasonType::Double);
        }
        
        // check if slice is a UTCDate object
        bool isUTCDate () const {
          return isType(JasonType::UTCDate);
        }

        // check if slice is an External object
        bool isExternal () const {
          return isType(JasonType::External);
        }

        // check if slice is a MinKey object
        bool isMinKey () const {
          return isType(JasonType::MinKey);
        }
        
        // check if slice is a MaxKey object
        bool isMaxKey () const {
          return isType(JasonType::MaxKey);
        }

        // check if slice is an Int object
        bool isInt () const {
          return isType(JasonType::Int);
        }
        
        // check if slice is a UInt object
        bool isUInt () const {
          return isType(JasonType::UInt);
        }

        // check if slice is a SmallInt object
        bool isSmallInt () const {
          return isType(JasonType::SmallInt);
        }

        // check if slice is a String object
        bool isString () const {
          return isType(JasonType::String);
        }

        // check if slice is a Binary object
        bool isBinary () const {
          return isType(JasonType::Binary);
        }

        // check if slice is a BCD
        bool isBCD () const {
          return isType(JasonType::BCD);
        }
        
        // check if slice is a custom type
        bool isCustom () const {
          return isType(JasonType::Custom);
        }
        
        // check if a slice is any number type
        bool isInteger () const {
          return isType(JasonType::Int) || isType(JasonType::UInt) || isType(JasonType::SmallInt);
        }

        // check if slice is any Number-type object
        bool isNumber () const {
          return isInteger() || isDouble();
        }
        
        bool isSorted () const {
          auto const h = head();
          return (h >= 0x0b && h <= 0x0e);
        }

        // return the value for a Bool object
        bool getBool () const {
          assertType(JasonType::Bool);
          return (head() == 0x1a); // 0x19 == false, 0x1a == true
        }

        // return the value for a Bool object - this is an alias for getBool()
        bool getBoolean () const {
          return getBool();
        }

        // return the value for a Double object
        double getDouble () const {
          assertType(JasonType::Double);
          union {
            uint64_t dv;
            double d;
          } v;
          v.dv = readInteger<uint64_t>(_start + 1, 8);
          return v.d; 
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
        JasonSlice at (JasonLength index) const {
          if (! isType(JasonType::Array)) {
            throw JasonException(JasonException::InvalidValueType, "Expecting Array");
          }

          return getNth(index);
        }

        JasonSlice operator[] (JasonLength index) const {
          return at(index);
        }

        // return the number of members for an Array or Object object
        JasonLength length () const {
          if (type() != JasonType::Array && type() != JasonType::Object) {
            throw JasonException(JasonException::InvalidValueType, "Expecting Array or Object");
          }

          auto const h = head();
          if (h == 0x01 || h == 0x0a) {
            // special case: empty!
            return 0;
          }

          JasonLength const offsetSize = indexEntrySize(h);
          JasonLength end = readInteger<JasonLength>(_start + 1, offsetSize);

          // find number of items
          if (h <= 0x05) {    // No offset table or length, need to compute:
            JasonLength firstSubOffset = findDataOffset(h);
            JasonSlice first(_start + firstSubOffset);
            return (end - firstSubOffset) / first.byteSize();
          } 
          else if (offsetSize < 8) {
            return readInteger<JasonLength>(_start + offsetSize + 1, offsetSize);
          }

          return readInteger<JasonLength>(_start + end - offsetSize, offsetSize);
        }

        // extract a key from an Object at the specified index
        // - 0x0a      : empty object
        // - 0x0b      : object with 1-byte index table entries, sorted by attribute name
        // - 0x0c      : object with 2-byte index table entries, sorted by attribute name
        // - 0x0d      : object with 4-byte index table entries, sorted by attribute name
        // - 0x0e      : object with 8-byte index table entries, sorted by attribute name
        // - 0x0f      : object with 1-byte index table entries, not sorted by attribute name
        // - 0x10      : object with 2-byte index table entries, not sorted by attribute name
        // - 0x11      : object with 4-byte index table entries, not sorted by attribute name
        // - 0x12      : object with 8-byte index table entries, not sorted by attribute name
        JasonSlice keyAt (JasonLength index) const {
          if (! isType(JasonType::Object)) {
            throw JasonException(JasonException::InvalidValueType, "Expecting Object");
          }
          
          return getNth(index);
        }

        JasonSlice valueAt (JasonLength index) const {
          JasonSlice key = keyAt(index);
          return JasonSlice(key.start() + key.byteSize());
        }

        // look for the specified attribute path inside an Object
        // returns a JasonSlice(Jason::None) if not found
        JasonSlice get (std::vector<std::string> const& attributes) const { 
          size_t const n = attributes.size();
          if (n == 0) {
            throw JasonException(JasonException::InvalidAttributePath);
          }

          // use ourselves as the starting point
          JasonSlice last = JasonSlice(start());
          for (size_t i = 0; i < attributes.size(); ++i) {
            // fetch subattribute
            last = last.get(attributes[i]);

            // abort as early as possible
            if (last.isNone() || (i + 1 < n && ! last.isObject())) {
              return JasonSlice();
            }
          }

          return last;
        }

        // look for the specified attribute inside an Object
        // returns a JasonSlice(Jason::None) if not found
        JasonSlice get (std::string const& attribute) const {
          if (! isType(JasonType::Object)) {
            throw JasonException(JasonException::InvalidValueType, "Expecting Object");
          }

          auto const h = head();
          if (h == 0xa) {
            // special case, empty object
            return JasonSlice();
          }
          
          JasonLength const offsetSize = indexEntrySize(h);
          JasonLength end = readInteger<JasonLength>(_start + 1, offsetSize);
          JasonLength dataOffset = 0;

          // read number of items
          JasonLength n;
          if (h <= 0x05) {    // No offset table or length, need to compute:
            dataOffset = findDataOffset(h);
            JasonSlice first(_start + dataOffset);
            n = (end - dataOffset) / first.byteSize();
          } 
          else if (offsetSize < 8) {
            n = readInteger<JasonLength>(_start + 1 + offsetSize, offsetSize);
          }
          else {
            n = readInteger<JasonLength>(_start + end - offsetSize, offsetSize);
          }
          
          if (n == 1) {
            // Just one attribute, there is no index table!
            if (dataOffset == 0) {
              dataOffset = findDataOffset(h);
            }
            JasonSlice attrName = JasonSlice(_start + dataOffset);
            if (! attrName.isString()) {
              return JasonSlice();
            }
            JasonLength attrLength;
            char const* k = attrName.getString(attrLength); 
            if (attrLength != static_cast<JasonLength>(attribute.size())) {
              // key must have the exact same length as the attribute we search for
              return JasonSlice();
            }
            if (memcmp(k, attribute.c_str(), attribute.size()) != 0) {
              return JasonSlice();
            }
            return JasonSlice(attrName.start() + attrName.byteSize());
          }

          JasonLength const ieBase = end - n * offsetSize 
                                     - (offsetSize == 8 ? offsetSize : 0);

          // only use binary search for attributes if we have at least this many entries
          // otherwise we'll always use the linear search
          static JasonLength const SortedSearchEntriesThreshold = 4;

          if (isSorted() && n >= SortedSearchEntriesThreshold) {
            // This means, we have to handle the special case n == 1 only
            // in the linear search!
            return searchObjectKeyBinary(attribute, ieBase, offsetSize, n);
          }

          return searchObjectKeyLinear(attribute, ieBase, offsetSize, n);
        }

        JasonSlice operator[] (std::string const& attribute) const {
          return get(attribute);
        }

        void iterateArray (std::function<bool(JasonSlice const&)> const& callback) const {
          JasonLength const n = length(); 
          for (JasonLength i = 0; i < n; ++i) {
            if (! callback(at(i))) {
              return;
            }
          }
        }

        void iterateObject (std::function<bool(JasonSlice const&, JasonSlice const&)> const& callback) const {
          JasonLength const n = length(); 
          for (JasonLength i = 0; i < n; ++i) {
            if (! callback(keyAt(i), valueAt(i))) {
              return;
            }
          }
        }
        
        std::vector<std::string> keys () const {
          std::vector<std::string> keys;
          JasonLength const n = length(); 
          keys.reserve(n);
          for (JasonLength i = 0; i < n; ++i) {
            keys.emplace_back(keyAt(i).copyString());
          }
          return keys;
        }

        void keys (std::vector<std::string>& keys) const {
          JasonLength const n = length(); 
          if (! keys.empty()) {
            keys.clear();
          }
          keys.reserve(n);
          for (JasonLength i = 0; i < n; ++i) {
            keys.emplace_back(keyAt(i).copyString());
          }
        }

        // return the pointer to the data for an External object
        char const* getExternal () const {
          return extractValue<char const*>();
        }

        // return the value for an Int object
        int64_t getInt () const {
          uint8_t const h = head();
          if (h >= 0x20 && h <= 0x27) {
            // Int  T
            uint64_t v = readInteger<uint64_t>(_start + 1, h - 0x1f);
            if (h == 0x27) {
              return toInt64(v);
            }
            else {
              int64_t vv = static_cast<int64_t>(v);
              int64_t shift = 1LL << ((h - 0x1f) * 8 - 1);
              return vv < shift ? vv : vv - (shift << 1);
            }
          }

          if (h >= 0x28 && h <= 0x2f) { 
            // UInt
            uint64_t v = getUInt();
            if (v > static_cast<uint64_t>(INT64_MAX)) {
              throw JasonException(JasonException::NumberOutOfRange);
            }
            return static_cast<int64_t>(v);
          }
          
          if (h >= 0x30 && h <= 0x3f) {
            // SmallInt
            return getSmallInt();
          }

          throw JasonException(JasonException::InvalidValueType, "Expecting type Int");
        }

        // return the value for a UInt object
        uint64_t getUInt () const {
          uint8_t const h = head();
          if (h >= 0x28 && h <= 0x2f) {
            // UInt
            return readInteger<uint64_t>(_start + 1, h - 0x27);
          }
          
          if (h >= 0x20 && h <= 0x27) {
            // Int 
            int64_t v = getInt();
            if (v < 0) {
              throw JasonException(JasonException::NumberOutOfRange);
            }
            return static_cast<int64_t>(v);
          }

          if (h >= 0x30 && h <= 0x39) {
            // Smallint >= 0
            return static_cast<uint64_t>(h - 0x30);
          }
          
          throw JasonException(JasonException::InvalidValueType, "Expecting type UInt");
        }

        // return the value for a SmallInt object
        int64_t getSmallInt () const {
          uint8_t const h = head();

          if (h >= 0x30 && h <= 0x39) {
            // Smallint >= 0
            return static_cast<int64_t>(h - 0x30);
          }

          if (h >= 0x3a && h <= 0x3f) {
            // Smallint < 0
            return static_cast<int64_t>(h - 0x3a) - 6;
          }

          if ((h >= 0x20 && h <= 0x27) ||
              (h >= 0x28 && h <= 0x2f)) {
            // Int and UInt
            // we'll leave it to the compiler to detect the two ranges above are adjacent
            return getInt();
          }

          throw JasonException(JasonException::InvalidValueType, "Expecting type Smallint");
        }

        // return the value for a UTCDate object
        int64_t getUTCDate () const {
          assertType(JasonType::UTCDate);
          uint64_t v = readInteger<uint64_t>(_start + 1, sizeof(uint64_t));
          return toInt64(v);
        }

        // return the value for a String object
        char const* getString (JasonLength& length) const {
          uint8_t const h = head();
          if (h >= 0x40 && h <= 0xbe) {
            // short UTF-8 String
            length = h - 0x40;
            return reinterpret_cast<char const*>(_start + 1);
          }

          if (h == 0xbf) {
            // long UTF-8 String
            length = readInteger<JasonLength>(_start + 1, 8);
            JasonCheckSize(length);
            return reinterpret_cast<char const*>(_start + 1 + 8);
          }

          throw JasonException(JasonException::InvalidValueType, "Expecting type String");
        }

        // return a copy of the value for a String object
        std::string copyString () const {
          uint8_t h = head();
          if (h >= 0x40 && h <= 0xbe) {
            // short UTF-8 String
            JasonLength length = h - 0x40;
            return std::string(reinterpret_cast<char const*>(_start + 1), static_cast<size_t>(length));
          }

          if (h == 0xbf) {
            JasonLength length = readInteger<JasonLength>(_start + 1, 8);
            JasonCheckSize(length);
            return std::string(reinterpret_cast<char const*>(_start + 1 + 8), length);
          }

          throw JasonException(JasonException::InvalidValueType, "Expecting type String");
        }

        // return the value for a Binary object
        uint8_t const* getBinary (JasonLength& length) const {
          assertType(JasonType::Binary);
          uint8_t const h = head();

          if (h >= 0xc0 && h <= 0xc7) {
            length = readInteger<JasonLength>(_start + 1, h - 0xbf); 
            JasonCheckSize(length);
            return _start + 1 + h - 0xbf;
          }

          throw JasonException(JasonException::InvalidValueType, "Expecting type Binary");
        }

        // return a copy of the value for a Binary object
        std::vector<uint8_t> copyBinary () const {
          assertType(JasonType::Binary);
          uint8_t const h = head();

          if (h >= 0xc0 && h <= 0xc7) {
            std::vector<uint8_t> out;
            JasonLength length = readInteger<JasonLength>(_start + 1, h - 0xbf); 
            JasonCheckSize(length);
            out.reserve(static_cast<size_t>(length));
            out.insert(out.end(), _start + 1 + h - 0xbf, _start + 1 + h - 0xbf + length);
            return out; 
          }

          throw JasonException(JasonException::InvalidValueType, "Expecting type Binary");
        }

        // get the total byte size for the slice, including the head byte
        JasonLength byteSize () const {
          switch (type()) {
            case JasonType::None:
            case JasonType::Null:
            case JasonType::Bool: 
            case JasonType::MinKey: 
            case JasonType::MaxKey: 
            case JasonType::SmallInt: {
              return 1; 
            }

            case JasonType::Double: {
              return 1 + sizeof(double);
            }

            case JasonType::Array:
            case JasonType::Object: {
              auto const h = head();
              if (h == 0x01 || h == 0x0a) {
                // empty array or object
                return 1;
              }

              return readInteger<JasonLength>(_start + 1, WidthMap[h]);
            }

            case JasonType::External: {
              return 1 + sizeof(char*);
            }

            case JasonType::UTCDate: {
              return 1 + sizeof(int64_t);
            }

            case JasonType::Int: {
              return static_cast<JasonLength>(1 + (head() - 0x1f));
            }

            case JasonType::UInt: {
              return static_cast<JasonLength>(1 + (head() - 0x27));
            }

            case JasonType::String: {
              auto const h = head();
              if (h == 0xbf) {
                // long UTF-8 String
                return static_cast<JasonLength>(1 + 8 + readInteger<JasonLength>(_start + 1, 8));
              }

              // short UTF-8 String
              return static_cast<JasonLength>(1 + h - 0x40);
            }

            case JasonType::Binary: {
              auto const h = head();
              return static_cast<JasonLength>(1 + h - 0xbf + readInteger<JasonLength>(_start + 1, h - 0xbf));
            }

            case JasonType::BCD: {
              auto const h = head();
              if (h <= 0xcf) {
                // positive BCD
                return static_cast<JasonLength>(1 + h - 0xc7 + readInteger<JasonLength>(_start + 1, h - 0xc7));
              } 

              // negative BCD
              return static_cast<JasonLength>(1 + h - 0xcf + readInteger<JasonLength>(_start + 1, h - 0xcf));
            }

            case JasonType::Custom: {
              return 0; // TODO 
            }
          }

          JASON_ASSERT(false);
          return 0;
        }

        std::string toString () const;
        std::string hexType () const;

      private:
        
        JasonLength findDataOffset (uint8_t const head) const {
          // Must be called for a nonempty array or object at start():
          unsigned int fsm = FirstSubMap[head];
          if (fsm <= 2 && _start[2] != 0) {
            return 2;
          }
          if (fsm <= 3 && _start[3] != 0) {
            return 3;
          }
          if (fsm <= 5 && _start[5] != 0) {
            return 5;
          }
          return 9;
        }
          
        // extract the nth member from an Array or Object type
        JasonSlice getNth (JasonLength index) const {
          JASON_ASSERT(type() == JasonType::Array || type() == JasonType::Object);

          auto const h = head();
          if (h == 0x01 || h == 0x0a) {
            // special case. empty array or object
            throw JasonException(JasonException::IndexOutOfBounds);
          }

          JasonLength const offsetSize = indexEntrySize(h);
          JasonLength end = readInteger<JasonLength>(_start + 1, offsetSize);

          JasonLength dataOffset = findDataOffset(h);
          
          // find the number of items
          JasonLength n;
          if (h <= 0x05) {    // No offset table or length, need to compute:
            JasonSlice first(_start + dataOffset);
            n = (end - dataOffset) / first.byteSize();
          }
          else if (offsetSize < 8) {
            n = readInteger<JasonLength>(_start + 1 + offsetSize, offsetSize);
          }
          else {
            n = readInteger<JasonLength>(_start + end - offsetSize, offsetSize);
          }

          if (index >= n) {
            throw JasonException(JasonException::IndexOutOfBounds);
          }

          // empty array case was already covered
          JASON_ASSERT(n > 0);

          if (h <= 0x05 || n == 1) {
            // no index table, but all array items have the same length
            // now fetch first item and determine its length
            if (dataOffset == 0) {
              dataOffset = findDataOffset(h);
            }
            JasonSlice firstItem(_start + dataOffset);
            return JasonSlice(_start + dataOffset + index * firstItem.byteSize());
          }
          
          JasonLength const ieBase = end - n * offsetSize + index * offsetSize
                                     - (offsetSize == 8 ? 8 : 0);
          return JasonSlice(_start + readInteger<JasonLength>(_start + ieBase, offsetSize));
        }

        JasonLength indexEntrySize (uint8_t head) const {
          return static_cast<JasonLength>(WidthMap[head]);
        }

        // perform a linear search for the specified attribute inside an Object
        JasonSlice searchObjectKeyLinear (std::string const& attribute, 
                                          JasonLength ieBase, 
                                          JasonLength offsetSize, 
                                          JasonLength n) const {
          for (JasonLength index = 0; index < n; ++index) {
            JasonLength offset = ieBase + index * offsetSize;
            JasonSlice key(_start + readInteger<JasonLength>(_start + offset, offsetSize));
            if (! key.isString()) {
              // invalid object
              return JasonSlice();
            }

            JasonLength keyLength;
            char const* k = key.getString(keyLength); 
            if (keyLength != static_cast<JasonLength>(attribute.size())) {
              // key must have the exact same length as the attribute we search for
              continue;
            }

            if (memcmp(k, attribute.c_str(), attribute.size()) != 0) {
              continue;
            }
            // key is identical. now return value
            return JasonSlice(key.start() + key.byteSize());
          }

          // nothing found
          return JasonSlice();
        }

        // perform a binary search for the specified attribute inside an Object
        JasonSlice searchObjectKeyBinary (std::string const& attribute, 
                                          JasonLength ieBase,
                                          JasonLength offsetSize, 
                                          JasonLength n) const {
          JASON_ASSERT(n > 0);
            
          JasonLength const attributeLength = static_cast<JasonLength>(attribute.size());

          JasonLength l = 0;
          JasonLength r = n - 1;

          while (true) {
            // midpoint
            JasonLength index = l + ((r - l) / 2);

            JasonLength offset = ieBase + index * offsetSize;
            JasonSlice key(_start + readInteger<JasonLength>(_start + offset, offsetSize));
            if (! key.isString()) {
              // invalid object
              return JasonSlice();
            }

            JasonLength keyLength;
            char const* k = key.getString(keyLength); 
            size_t const compareLength = static_cast<size_t>((std::min)(keyLength, attributeLength));
            int res = memcmp(k, attribute.c_str(), compareLength);

            if (res == 0 && keyLength == attributeLength) {
              // key is identical. now return value
              return JasonSlice(key.start() + key.byteSize());
            }

            if (res > 0 || (res == 0 && keyLength > attributeLength)) {
              if (index == 0) {
                return JasonSlice();
              }
              r = index - 1;
            }
            else {
              l = index + 1;
            }
            if (r < l) {
              return JasonSlice();
            }
          }
        }
         
        // assert that the slice is of a specific type
        // can be used for debugging and removed in production
#ifdef JASON_ASSERT
        inline void assertType (JasonType) const {
        }
#else
        inline void assertType (JasonType type) const {
          JASON_ASSERT(this->type() == type);
        }
#endif
          
        // read an unsigned little endian integer value of the
        // specified length, starting at the specified byte offset
        template <typename T>
        T readInteger (uint8_t const* start, JasonLength numBytes) const {
          T value = 0;
          uint8_t const* p = start;
          uint8_t const* e = p + numBytes;
          T digit = 0;

          while (p < e) {
            value += static_cast<T>(*p) << (digit * 8);
            ++digit;
            ++p;
          }

          return value;
        }

        // extracts a value from the slice and converts it into a 
        // built-in type
        template<typename T> T extractValue () const {
          union {
            T value;
            char binary[sizeof(T)];
          }; 
          memcpy(&binary[0], _start + 1, sizeof(T));
          return value; 
        }

      private:

        static JasonType const TypeMap[256];
        static unsigned int const WidthMap[0x13];
        static unsigned int const FirstSubMap[0x13];
    };

    static_assert(sizeof(JasonSlice) == sizeof(void*), "JasonSlice has an unexpected size");

  }  // namespace arangodb::jason
}  // namespace arangodb
        
std::ostream& operator<< (std::ostream&, arangodb::jason::JasonSlice const*);

std::ostream& operator<< (std::ostream&, arangodb::jason::JasonSlice const&);

#endif
