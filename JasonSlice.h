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

        bool isType (JasonType type) const {
          return TypeTable[*_start] == type;
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
          ensureType(JasonType::Bool);
          return (*_start == 0x2);
        }

        double getDouble () const {
          ensureType(JasonType::Double);
          return extractValue<double>();
        }

        JasonSlice at (JasonLength index) const {
          // TODO
          return *this;
        }

        JasonSlice operator[] (JasonLength index) const {
          // TODO
          return *this;
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

        JasonSlice get (std::string const& attribute) const {
          // TODO
          return *this;
        }

        JasonSlice operator[] (std::string const& attribute) const {
          // TODO
          return *this;
        }

        int64_t getInt () const {
          ensureType(JasonType::Int);
          if (*_start <= 0x27) {
            // positive int
            return readInteger<int64_t>(_start + 1, *_start - 0x1f);
          }
          // negative int
          return - readInteger<int64_t>(_start + 1, *_start - 0x27);
        }

        uint64_t getUInt () const {
          ensureType(JasonType::UInt);
          return readInteger<uint64_t>(_start + 1, *_start - 0x2f);
        }

        uint64_t getUTCDate () const {
          ensureType(JasonType::UTCDate);
          return readInteger<uint64_t>(_start + 1, *_start - 0x2f);
        }

        char const* getString (JasonLength& length) const {
          // TODO
          return nullptr;
        }

        std::string copyString () const {
          // TODO
          return std::string("Hello");
        }

        uint8_t const* getBinary (JasonLength& length) const {
          // TODO
          return nullptr;
        }

        std::vector<uint8_t> copyBinary () const {
          // TODO
          return std::vector<uint8_t>();
        }

        void toJsonString (std::string& out) const {
          // TODO
        }

        static void Initialize ();
  
      private:
         
        void ensureType (JasonType type) const {
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
