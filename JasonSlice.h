#ifndef JASON_SLICE_H
#define JASON_SLICE_H 1

#include "JasonType.h"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <array>
#include <iostream>

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

        uint64_t byteSize () const;

        uint8_t const* start () const {
          return _start;
        }

        bool isNull () const {
          return TypeTable[*_start] == JasonType::Null;
        }

        bool isBool () const {
          return TypeTable[*_start] == JasonType::Bool;
        }

        bool isDouble () const {
          return TypeTable[*_start] == JasonType::Double;
        }
        
        bool isArray () const {
          return TypeTable[*_start] == JasonType::Array;
        }

        bool isObject () const {
          return TypeTable[*_start] == JasonType::Object;
        }

        bool isExternal () const {
          return TypeTable[*_start] == JasonType::External;
        }

        bool isID () const {
          return TypeTable[*_start] == JasonType::ID;
        }

        bool isArangoDB_id () const {
          return TypeTable[*_start] == JasonType::ArangoDB_id;
        }

        bool isUTCDate () const {
          return TypeTable[*_start] == JasonType::UTCDate;
        }

        bool isInt () const {
          return TypeTable[*_start] == JasonType::Int;
        }
        
        bool isUInt () const {
          return TypeTable[*_start] == JasonType::UInt;
        }

        bool isString () const {
          return TypeTable[*_start] == JasonType::String;
        }

        bool isBinary () const {
          return TypeTable[*_start] == JasonType::Binary;
        }

        bool getBool () const {
          ensureType(JasonType::Bool);
          return (*_start == 0x2);
        }

        double getDouble () const {
          ensureType(JasonType::Double);
          return extractValue<double>();
        }

        JasonSlice at (size_t index) {
          // TODO
          return *this;
        }

        JasonSlice operator[] (size_t index) {
          // TODO
          return *this;
        }

        size_t length () {
          // TODO
          return 0;
        }

        JasonSlice get (std::string& attribute) {
          // TODO
          return *this;
        }

        JasonSlice operator[] (std::string& attribute) {
          // TODO
          return *this;
        }

        uint64_t getUTCDate () const {
          // TODO
          return 0ul;
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

        char* getString (size_t& length) {
          // TODO
          return nullptr;
        }

        std::string copyString () {
          // TODO
          return std::string("Hello");
        }

        uint8_t* getBinary (size_t& length) {
          // TODO
          return nullptr;
        }

        std::vector<uint8_t> copyBinary () {
          // TODO
          return std::vector<uint8_t>();
        }

        void toJsonString (std::string& out) {
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
        T readInteger (size_t numBytes) const {
          return readInteger<T>(_start + 1, numBytes);
        }

        template <typename T>
        T readInteger (uint8_t const* start, size_t numBytes) const {
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
