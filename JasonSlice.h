#ifndef JASON_SLICE_H
#define JASON_SLICE_H 1

#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <array>
#include <iostream>

#include "Jason.h"
#include "JasonUtils.h"
#include "JasonType.h"

namespace triagens {
  namespace basics {

    class JasonSlice {

      // This class provides read only access to a Jason value, it is
      // intentionally light-weight (only one pointer value), such that
      // it can easily be used to traverse larger Jason values.

        friend class JasonBuilder;

        uint8_t const* _start;

      public:
  
        explicit JasonSlice () : _start(NotFoundSliceData) {
        }

        explicit JasonSlice (uint8_t const* start) : _start(start) {
        }

        explicit JasonSlice (char const* start) : _start(reinterpret_cast<uint8_t const*>(start)) {
        }

        // No destructor, does not take part in memory management,
        // standard copy, and move constructors, behaves like a pointer.

        // get the type for the slice
        inline JasonType type () const {
          return TypeTable[head()];
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
        inline bool isType (JasonType type) const {
          return TypeTable[head()] == type;
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

        // check if slice is a Double object
        bool isDouble () const {
          return isType(JasonType::Double);
        }
        
        // check if slice is an Array object
        bool isArray () const {
          return isType(JasonType::Array) || isType(JasonType::ArrayLong);
        }

        // check if slice is an Object object
        bool isObject () const {
          return isType(JasonType::Object) || isType(JasonType::ObjectLong);
        }

        // check if slice is an External object
        bool isExternal () const {
          return isType(JasonType::External);
        }

        // check if slice is an ID object
        bool isID () const {
          return isType(JasonType::ID);
        }

        // check if slice is an ArangoDB_id object
        bool isArangoDB_id () const {
          return isType(JasonType::ArangoDB_id);
        }

        // check if slice is a UTCDate object
        bool isUTCDate () const {
          return isType(JasonType::UTCDate);
        }

        // check if slice is an Int object
        bool isInt () const {
          return isType(JasonType::Int);
        }
        
        // check if slice is a UInt object
        bool isUInt () const {
          return isType(JasonType::UInt);
        }

        // check if slice is a String object
        bool isString () const {
          return isType(JasonType::String) || isType(JasonType::StringLong);
        }

        // check if slice is a Binary object
        bool isBinary () const {
          return isType(JasonType::Binary);
        }

        // check if slice is any Number-type object
        bool isNumber () const {
          return isType(JasonType::Int) || isType(JasonType::UInt) || isType(JasonType::Double);
        }

        // return the value for a Bool object
        bool getBool () const {
          assertType(JasonType::Bool);
          return (head() == 0x2);
        }

        // return the value for a Double object
        double getDouble () const {
          assertType(JasonType::Double);
          return extractValue<double>();
        }

        JasonSlice at (JasonLength index) const {
          JasonLength offsetSize;
          if (isType(JasonType::Array)) {
            // short array
            offsetSize = 2;
          }
          else if (isType(JasonType::ArrayLong)) {
            // long array
            offsetSize = 8;
          }
          else {
            throw JasonTypeError("unexpected type - expecting array");
          }

          JasonLength const n = readInteger<JasonLength>(offsetSize - 1);
          if (index >= n) {
            throw JasonTypeError("index out of bounds");
          }
          // TODO: check if this works with long arrays
          if (index == 0) {
            return JasonSlice(start() + (n + 1) * offsetSize);
          }
          JasonLength const offsetPosition = (index + 1) * offsetSize;
          return JasonSlice(start() + readInteger<JasonLength>(start() + offsetPosition, offsetSize));
        }

        JasonSlice operator[] (JasonLength index) const {
          return at(index);
        }

        // return the number of members for an Array or Object object
        JasonLength length () const {
          switch (type()) {
            case JasonType::Array:
            case JasonType::Object:
              return readInteger<JasonLength>(1);
            case JasonType::ArrayLong:
            case JasonType::ObjectLong:
              return readInteger<JasonLength>(7);
            default:
              throw JasonTypeError("unexpected type. expecting array or object");
          }
        }

        JasonSlice keyAt (JasonLength index) const {
          JasonLength offsetSize;
          if (isType(JasonType::Object)) {
            // short object
            offsetSize = 2;
          }
          else if (isType(JasonType::ObjectLong)) {
            // long object
            offsetSize = 8;
          }
          else {
            throw JasonTypeError("unexpected type - expecting object");
          }

          JasonLength const n = readInteger<JasonLength>(offsetSize - 1);
          if (index >= n) {
            throw JasonTypeError("index out of bounds");
          }
          JasonLength const offsetPosition = (index + 2) * offsetSize;
          return JasonSlice(start() + readInteger<JasonLength>(start() + offsetPosition, offsetSize));
        }

        JasonSlice valueAt (JasonLength index) const {
          JasonSlice key = keyAt(index);
          return JasonSlice(key.start() + key.byteSize());
        }

        JasonSlice get (std::string const& attribute) const {
          JasonLength offsetSize;
          if (isType(JasonType::Object)) {
            // short object
            offsetSize = 2;
          }
          else if (isType(JasonType::ObjectLong)) {
            // long object
            offsetSize = 8;
          }
          else {
            throw JasonTypeError("unexpected type - expecting object");
          }

          JasonLength const n = readInteger<JasonLength>(offsetSize - 1);
          if (n < MaxLengthForLinearSearch) {
            return searchObjectKeyLinear(attribute, offsetSize, n);
          }
          return searchObjectKeyBinary(attribute, offsetSize, n);
        }

        JasonSlice operator[] (std::string const& attribute) const {
          return get(attribute);
        }

        JasonSlice searchObjectKeyLinear (std::string const& attribute, JasonLength offsetSize, JasonLength n) const {
          for (JasonLength index = 0; index < n; ++index) {
            JasonLength const offsetPosition = (index + 2) * offsetSize;
            JasonSlice key(_start + readInteger<JasonLength>(_start + offsetPosition, offsetSize));
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

        JasonSlice searchObjectKeyBinary (std::string const& attribute, JasonLength offsetSize, JasonLength n) const {
          assert(n > 0);
            
          JasonLength const attributeLength = static_cast<JasonLength>(attribute.size());

          JasonLength l = 0;
          JasonLength r = n - 1;

          while (true) {
            // midpoint
            JasonLength index = l + ((r - l) / 2);
            JasonLength const offsetPosition = (index + 2) * offsetSize;

            JasonSlice key(_start + readInteger<JasonLength>(_start + offsetPosition, offsetSize));
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

        // return the pointer to the data for an External object
        char const* getExternal () const {
          return extractValue<char const*>();
        }

        // return the value for an Int object
        int64_t getInt () const {
          assertType(JasonType::Int);
          uint8_t h = head();
          if (h <= 0x27) {
            // positive int
            return readInteger<int64_t>(_start + 1, h - 0x1f);
          }
          // negative int
          return - readInteger<int64_t>(_start + 1, h - 0x27);
        }

        // return the value for a UInt object
        uint64_t getUInt () const {
          assertType(JasonType::UInt);
          return readInteger<uint64_t>(_start + 1, head() - 0x2f);
        }

        // return the value for a UTCDate object
        uint64_t getUTCDate () const {
          assertType(JasonType::UTCDate);
          return readInteger<uint64_t>(_start + 1, head() - 0x2f);
        }

        // return the value for a String or StringLong object
        char const* getString (JasonLength& length) const {
          uint8_t h = head();
          if (h >= 0x40 && h <= 0xbf) {
            // short string
            length = h - 0x40;
            return reinterpret_cast<char const*>(_start + 1);
          }
          if (h >= 0xc0 && h <= 0xc7) {
            length = readInteger<JasonLength>(h - 0xbf); 
            return reinterpret_cast<char const*>(_start + 1 + h - 0xbf);
          }
          throw JasonTypeError("unexpected type. expecting string");
        }

        // return a copy of the value for a String object
        std::string copyString () const {
          uint8_t h = head();
          if (h >= 0x40 && h <= 0xbf) {
            // short string
            JasonLength length = h - 0x40;
            JasonUtils::CheckSize(length);
            return std::string(reinterpret_cast<char const*>(_start + 1), static_cast<size_t>(length));
          }
          if (h >= 0xc0 && h <= 0xc7) {
            JasonLength length = readInteger<JasonLength>(h - 0xbf); 
            JasonUtils::CheckSize(length);
            return std::string(reinterpret_cast<char const*>(_start + 1 + h - 0xbf), length);
          }
          throw JasonTypeError("unexpected type. expecting string");
        }

        // return the value for a Binary object
        uint8_t const* getBinary (JasonLength& length) const {
          assertType(JasonType::Binary);
          uint8_t h = head();
          if (h >= 0xd0 && h <= 0xd7) {
            length = readInteger<JasonLength>(h - 0xcf); 
            JasonUtils::CheckSize(length);
            return _start + 1 + h - 0xcf;
          }
          throw JasonTypeError("unexpected type. expecting binary");
        }

        // return a copy of the value for a Binary object
        std::vector<uint8_t> copyBinary () const {
          assertType(JasonType::Binary);
          uint8_t h = head();
          if (h >= 0xd0 && h <= 0xd7) {
            std::vector<uint8_t> out;
            JasonLength length = readInteger<JasonLength>(h - 0xcf); 
            JasonUtils::CheckSize(length);
            out.reserve(static_cast<size_t>(length));
            out.insert(out.end(), _start + 1 + h - 0xcf, _start + 1 + h - 0xcf + length);
            return out; 
          }
          throw JasonTypeError("unexpected type. expecting binary");
        }

        void toJsonString (std::string& /*out*/) const {
          // TODO
        }

        // get the total byte size for the slice, including the head byte
        JasonLength byteSize () const;

        // initialize the JasonSlice handling 
        static void Initialize ();
  
      private:
         
        // assert that the slice is of a specific type
        // can be used for debugging and removed in production
        void assertType (JasonType type) const {
#if 1
          assert(this->type() == type);
#endif
        }

        // read an unsigned little endian integer value of the
        // specified length, starting at the byte following the head byte 
        template <typename T>
        T readInteger (JasonLength numBytes) const {
          return readInteger<T>(_start + 1, numBytes);
        }

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

        // a lookup table for Jason types
        static std::array<JasonType, 256> TypeTable;

        // a built-in "not found" value slice
        static uint8_t const* NotFoundSliceData;

        // maximum number of attributes in an object for which a linear
        // search is performed
        static JasonLength const MaxLengthForLinearSearch;

    };

  }  // namespace triagens::basics
}  // namespace triagens

#endif
