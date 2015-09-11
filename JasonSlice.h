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
          return *this;
        }

        JasonSlice operator[] (size_t index) {
          return *this;
        }

        size_t length () {
          return 0;
        }

        JasonSlice get (std::string& attribute) {
          return *this;
        }

        JasonSlice operator[] (std::string& attribute) {
          return *this;
        }

        uint64_t getUTCDate () {
          return 0ul;
        }

        int64_t getInt (size_t bytes) const {
          return 0l;
        }

        uint64_t getUInt (size_t& bytes) {
          return 0ul;
        }

        char* getString (size_t& length) {
          return nullptr;
        }

        std::string copyString () {
          return std::string("Hello");
        }

        uint8_t* getBinary (size_t& length) {
          return nullptr;
        }

        std::vector<uint8_t> copyBinary () {
          return std::vector<uint8_t>();
        }

        void toJsonString (std::string& out) {
        }

        static void Initialize ();
  
      private:
         
        void ensureType (JasonType type) const {
          // can be used for debugging and removed in production
#if 1
          assert(this->type() == type);
#endif
        }

        uint64_t readLength (size_t numBytes) const {
          return readLength(_start, numBytes);
        }

        uint64_t readLength (uint8_t const* start, size_t numBytes) const {
          uint64_t length = 0;
          uint8_t const* p = start + 1;
          uint8_t const* e = p + numBytes;
          int digit = 0;

          while (p < e) {
            length += *p << Powers[digit];
            ++digit;
            ++p;
          }

          return length;
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

        static std::array<uint64_t, 8> Powers;

    };

  }  // namespace triagens::basics
}  // namespace triagens

#endif
