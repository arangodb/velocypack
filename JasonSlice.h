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
          return isType(JasonType::Array);
        }

        // check if slice is an Object object
        bool isObject () const {
          return isType(JasonType::Object);
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

        bool isInteger () const {
          return isType(JasonType::Int) || isType(JasonType::UInt) || isType(JasonType::SmallInt);
        }

        // check if slice is any Number-type object
        bool isNumber () const {
          return isInteger() || isDouble();
        }

        // return the value for a Bool object
        bool getBool () const {
          assertType(JasonType::Bool);
          return (head() == 0x03); // 0x02 == false, 0x03 == true
        }

        // return the value for a Double object
        double getDouble () const {
          assertType(JasonType::Double);
          return extractValue<double>();
        }

        JasonSlice at (JasonLength index) const {
          if (! isType(JasonType::Array)) {
            throw JasonTypeError("unexpected type. expecting array");
          }

          JasonLength offsetSize, sizeSize;
          if (head() == 0x05) {
            // short array
            offsetSize = 2;
            sizeSize = 1;
          }
          else {
            // long array
            offsetSize = 8;
            sizeSize = 8;
          }

          JasonLength end;
          uint8_t b = _start[1];
          if (b == 0x02) {
            throw JasonTypeError("index out of bounds");
          }
          else if (b == 0x00) {
            end = readInteger<JasonLength>(_start + 2, 8);
          }
          else {
            // 1 byte length: already got the length
            end = static_cast<JasonLength>(b);
          }
          JasonLength const n = readInteger<JasonLength>(_start + end - sizeSize, sizeSize);
          if (index >= n) {
            throw JasonTypeError("index out of bounds");
          }
          if (index == 0) {
            // special case for first array element
            if (b == 0x00) {
              return JasonSlice(_start + 1 + 1 + 8);
            }
            return JasonSlice(_start + 1 + 1);
          }
          assert(n > 0);
          JasonLength const indexBase = end - sizeSize - (n - 1) * offsetSize;
          JasonLength const offset = indexBase + (index - 1) * offsetSize;
          return JasonSlice(_start + readInteger<JasonLength>(_start + offset, offsetSize));
        }

        JasonSlice operator[] (JasonLength index) const {
          return at(index);
        }

        // return the number of members for an Array or Object object
        JasonLength length () const {
          if (type() != JasonType::Array && type() != JasonType::Object) {
            throw JasonTypeError("unexpected type. expecting array or object");
          }

          uint8_t b = _start[1];
          if (b == 0x02) {
            // special case
            return 0;
          }
          JasonLength sizeSize;
          auto h = head();
          if (h == 0x05 || h == 0x07) {
            // short array or object
            sizeSize = 1;
          }
          else {
            // long array or object
            sizeSize = 8;
          }
          JasonLength end;
          if (b == 0x00) {
            end = readInteger<JasonLength>(_start + 2, 8);
          }
          else {
            end = static_cast<JasonLength>(b);
          }
          return readInteger<JasonLength>(_start + end - sizeSize, sizeSize);
        }

        JasonSlice keyAt (JasonLength index) const {
          if (! isType(JasonType::Object)) {
            throw JasonTypeError("unexpected type. expecting object");
          }
          JasonLength offsetSize, sizeSize;
          if (head() == 0x07) {
            // short object
            offsetSize = 2;
            sizeSize = 1;
          }
          else {
            // long object
            offsetSize = 8;
            sizeSize = 8;
          }

          uint8_t b = _start[1];
          JasonLength end;
          if (b == 0x02) {
            // special case
            throw JasonTypeError("index out of bounds");
          }
          else if (b == 0x00) {
            end = readInteger<JasonLength>(_start + 2, 8);
          }
          else {
            // 1 byte length: already got the length
            end = static_cast<JasonLength>(b);
          }
          JasonLength const n = readInteger<JasonLength>(_start + end - sizeSize, sizeSize);
          if (index >= n) {
            throw JasonTypeError("index out of bounds");
          }
          if (index == 0) {
            // special case for first key
            if (b == 0x00) {
              return JasonSlice(_start + 1 + 1 + 8);
            }
            return JasonSlice(_start + 1 + 1);
          }
          assert(n > 0);
          JasonLength const indexBase = end - sizeSize - 2 * (n - 1) * offsetSize;
          JasonLength const offset = indexBase + 2 * (index - 1) * offsetSize;
          return JasonSlice(_start + readInteger<JasonLength>(_start + offset, offsetSize));
        }

        JasonSlice valueAt (JasonLength index) const {
          JasonSlice key = keyAt(index);
          return JasonSlice(key.start() + key.byteSize());
        }

        // look for the specified attribute path inside an object
        // returns a JasonSlice(Jason::None) if not found
        JasonSlice get (std::vector<std::string> const& attributes) const { 
          size_t const n = attributes.size();
          if (n == 0) {
            throw JasonTypeError("got empty attribute path");
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

        // look for the specified attribute inside an object
        // returns a JasonSlice(Jason::None) if not found
        JasonSlice get (std::string const& attribute) const {
          if (! isType(JasonType::Object)) {
            throw JasonTypeError("unexpected type. expecting object");
          }

          JasonLength offsetSize, sizeSize;
          if (head() == 0x07) {
            // short object
            offsetSize = 2;
            sizeSize = 1;
          }
          else {
            // long object
            offsetSize = 8;
            sizeSize = 8;
          }

          JasonLength end;
          JasonLength firstOffset;
          uint8_t b = _start[1];
          if (b == 0x02) {
            // special case
            return JasonSlice();
          }
          else if (b == 0x00) {
            end = readInteger<JasonLength>(_start + 2, 8);
            firstOffset = 1 + 1 + 8; 
          }
          else {
            // 1 byte length: already got the length
            end = static_cast<JasonLength>(b);
            firstOffset = 1 + 1; 
          }
          JasonLength const n = readInteger<JasonLength>(_start + end - sizeSize, sizeSize);
          JasonLength const indexBase = end - sizeSize - 2 * (n - 1) * offsetSize;
          if (n < MaxLengthForLinearSearch) {
            return searchObjectKeyLinear(attribute, firstOffset, indexBase, offsetSize, n);
          }
          return searchObjectKeyBinary(attribute, firstOffset, indexBase, offsetSize, n);
        }

        JasonSlice operator[] (std::string const& attribute) const {
          return get(attribute);
        }

        // return the pointer to the data for an External object
        char const* getExternal () const {
          return extractValue<char const*>();
        }

        // return the value for an Int object
        int64_t getInt () const {
          assertType(JasonType::Int);
          uint8_t h = head();
          if (h <= 0x1f) {
            // positive int
            return readInteger<int64_t>(_start + 1, h - 0x17);
          }
          // negative int
          return - readInteger<int64_t>(_start + 1, h - 0x1f);
        }

        // return the value for a UInt object
        uint64_t getUInt () const {
          assertType(JasonType::UInt);
          return readInteger<uint64_t>(_start + 1, head() - 0x27);
        }

        // return the value for a SmallInt object
        int64_t getSmallInt () const {
          assertType(JasonType::SmallInt);
          uint8_t h = head();
          if (h >= 0x30 && h <= 0x37) {
            return static_cast<int64_t>(h - 0x30);
          }
          else if (h >= 0x38 && h <= 0x3f) {
            return static_cast<int64_t>(h - 0x38) - 8;
          }
          throw JasonTypeError("unexpected type. expecting smallint");
        }

        // return the value for a UTCDate object
        uint64_t getUTCDate () const {
          assertType(JasonType::UTCDate);
          return readInteger<uint64_t>(_start + 1, head() - 0x0f);
        }

        // return the value for a String object
        char const* getString (JasonLength& length) const {
          uint8_t h = head();
          if (h >= 0x40 && h <= 0xbf) {
            // short string
            length = h - 0x40;
            return reinterpret_cast<char const*>(_start + 1);
          }
          if (h == 0x0c) {
            length = readInteger<JasonLength>(_start + 1, 8);
            return reinterpret_cast<char const*>(_start + 1 + 8);
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
          if (h == 0x0c) {
            JasonLength length = readInteger<JasonLength>(_start + 1, 8);
            JasonUtils::CheckSize(length);
            return std::string(reinterpret_cast<char const*>(_start + 1 + 8), length);
          }
          throw JasonTypeError("unexpected type. expecting string");
        }

        // return the value for a Binary object
        uint8_t const* getBinary (JasonLength& length) const {
          assertType(JasonType::Binary);
          uint8_t h = head();
          if (h >= 0xc0 && h <= 0xc7) {
            length = readInteger<JasonLength>(_start + 1, h - 0xbf); 
            JasonUtils::CheckSize(length);
            return _start + 1 + h - 0xbf;
          }
          throw JasonTypeError("unexpected type. expecting binary");
        }

        // return a copy of the value for a Binary object
        std::vector<uint8_t> copyBinary () const {
          assertType(JasonType::Binary);
          uint8_t h = head();
          if (h >= 0xc0 && h <= 0xc7) {
            std::vector<uint8_t> out;
            JasonLength length = readInteger<JasonLength>(_start + 1, h - 0xbf); 
            JasonUtils::CheckSize(length);
            out.reserve(static_cast<size_t>(length));
            out.insert(out.end(), _start + 1 + h - 0xbf, _start + 1 + h - 0xbf + length);
            return out; 
          }
          throw JasonTypeError("unexpected type. expecting binary");
        }

        // get the total byte size for the slice, including the head byte
        JasonLength byteSize () const;

        // initialize the JasonSlice handling 
        static void Initialize ();
  
      private:

        // perform a linear search for the specified attribute inside an object
        JasonSlice searchObjectKeyLinear (std::string const& attribute, 
                                          JasonLength firstOffset, 
                                          JasonLength indexBase, 
                                          JasonLength offsetSize, 
                                          JasonLength n) const {
          for (JasonLength index = 0; index < n; ++index) {
            JasonLength offset;
            if (index == 0) { 
              offset = firstOffset;
            }
            else {
              assert(index > 0);
              offset = indexBase + 2 * (index - 1) * offsetSize;
            }
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

        // perform a binary search for the specified attribute inside an object
        JasonSlice searchObjectKeyBinary (std::string const& attribute, 
                                          JasonLength firstOffset,
                                          JasonLength indexBase,
                                          JasonLength offsetSize, 
                                          JasonLength n) const {
          assert(n > 0);
            
          JasonLength const attributeLength = static_cast<JasonLength>(attribute.size());

          JasonLength l = 0;
          JasonLength r = n - 1;

          while (true) {
            // midpoint
            JasonLength index = l + ((r - l) / 2);

            JasonLength offset;
            if (index == 0) { 
              offset = firstOffset;
            }
            else {
              assert(index > 0);
              offset = indexBase + 2 * (index - 1) * offsetSize;
            }
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
#ifndef NDEBUG
        void assertType (JasonType type) const {
          assert(this->type() == type);
        }
#else
        void assertType (JasonType) const {
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
