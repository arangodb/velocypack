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

        JasonSlice (uint8_t const* start) : _start(start) {
        }

        // No destructor, does not take part in memory management,
        // standard copy, and move constructors, behaves like a pointer.

        JasonType type () const {
          return TypeTable[*_start];
        }

        JasonLength byteSize () const;

        uint8_t const* start () const {
          return _start;
        }

        uint8_t head () const {
          return *_start;
        }

        bool isType (JasonType type) const {
          return TypeTable[head()] == type;
        }

        bool isNull () const {
          return isType(JasonType::Null);
        }

        bool isBool () const {
          return isType(JasonType::Bool);
        }

        bool isDouble () const {
          return isType(JasonType::Double);
        }
        
        bool isArray () const {
          return isType(JasonType::Array) || isType(JasonType::ArrayLong);
        }

        bool isObject () const {
          return isType(JasonType::Object) || isType(JasonType::ObjectLong);
        }

        bool isExternal () const {
          return isType(JasonType::External);
        }

        bool isID () const {
          return isType(JasonType::ID);
        }

        bool isArangoDB_id () const {
          return isType(JasonType::ArangoDB_id);
        }

        bool isUTCDate () const {
          return isType(JasonType::UTCDate);
        }

        bool isInt () const {
          return isType(JasonType::Int);
        }
        
        bool isUInt () const {
          return isType(JasonType::UInt);
        }

        bool isNumber () const {
          return isType(JasonType::Int) || isType(JasonType::UInt) || isType(JasonType::Double);
        }

        bool isString () const {
          return isType(JasonType::String);
        }

        bool isBinary () const {
          return isType(JasonType::Binary);
        }

        bool getBool () const {
          assertType(JasonType::Bool);
          return (head() == 0x2);
        }

        double getDouble () const {
          assertType(JasonType::Double);
          return extractValue<double>();
        }

        JasonSlice at (JasonLength /*index*/) const {
          // TODO
          return *this;
        }

        JasonSlice operator[] (JasonLength index) const {
          return at(index);
        }

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
          return 0;
        }

        JasonSlice get (std::string const& /*attribute*/) const {
          // TODO
          return *this;
        }

        JasonSlice operator[] (std::string const& attribute) const {
          return get(attribute);
        }

        char const* getExternal () const {
          return extractValue<char const*>();
        }

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

        uint64_t getUInt () const {
          assertType(JasonType::UInt);
          return readInteger<uint64_t>(_start + 1, head() - 0x2f);
        }

        uint64_t getUTCDate () const {
          assertType(JasonType::UTCDate);
          return readInteger<uint64_t>(_start + 1, head() - 0x2f);
        }

        char const* getString (JasonLength& length) const {
          assertType(JasonType::String);
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

        std::string copyString () const {
          assertType(JasonType::String);
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

        static void Initialize ();
  
      private:
         
        void assertType (JasonType type) const {
          // can be used for debugging and removed in production
#if 1
          assert(this->type() == type);
#endif
        }

        template <typename T>
        T readInteger (JasonLength numBytes) const {
          return readInteger<T>(_start + 1, numBytes);
        }

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

        template<typename T> T extractValue () const {
          union {
            T value;
            char binary[sizeof(T)];
          }; 
          memcpy(&binary[0], _start + 1, sizeof(T));
          return value; 
        }

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief a lookup table for Jason types
////////////////////////////////////////////////////////////////////////////////

        static std::array<JasonType, 256> TypeTable;

    };

  }  // namespace triagens::basics
}  // namespace triagens

#endif
