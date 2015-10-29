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

#ifndef JASON_H
#define JASON_H 1

#include <cstdint>
#include <string>
#include <exception>

// debug mode
#ifdef JASON_DEBUG
#ifndef DEBUG
#define DEBUG
#endif
#include <cassert>
#define JASON_ASSERT(x) assert(x)

#else

#ifndef NDEBUG
#define NDEBUG
#endif
#define JASON_ASSERT(x) 
#endif

// check for environment type (32 or 64 bit)
// if the environment type cannot be determined reliably, then this will
// abort compilation. this will abort on systems that neither have 32 bit
// nor 64 bit pointers!
#if INTPTR_MAX == INT32_MAX
#define JASON_32BIT

#elif INTPTR_MAX == INT64_MAX
#define JASON_64BIT

#else
#error "Could not determine environment type (32 or 64 bits)"
#endif

#include "JasonException.h"
#include "JasonType.h"

namespace arangodb {
  namespace jason {

    // unified size type for Jason, can be used on 32 and 64 bit
    // though no Jason values exceeded the bounds of 32 bit can be
    // used on a 32 bit system
    typedef uint64_t JasonLength;

    static_assert(sizeof(JasonLength) >= sizeof(SIZE_MAX), "invalid value for SIZE_MAX");

#ifndef JASON_64BIT
    // check if the length is beyond the size of a SIZE_MAX on this platform
    static inline void JasonCheckSize (JasonLength length) {
      if (length > static_cast<JasonLength>(SIZE_MAX)) {
        throw JasonException("JasonLength out of bounds.");
      }  
    }
#else
    static inline void JasonCheckSize (JasonLength) { 
      // do nothing on a 64 bit platform 
    }
#endif

    // returns current value for UTCDate
    int64_t CurrentUTCDateValue ();

    class Jason {
      // Convenience class for more compact notation

      friend class JasonBuilder;

      public:

        enum CType {
          None     = 0,
          Bool     = 1,
          Double   = 2,
          Int64    = 3,
          UInt64   = 4,
          String   = 5,
          CharPtr  = 6,
          VoidPtr  = 7
        };
       
      private:

        JasonType _jasonType;
        CType     _cType;    // denotes variant used, 0: none

        union {
          bool b;                // 1: bool
          double d;              // 2: double
          int64_t i;             // 3: int64_t
          uint64_t u;            // 4: uint64_t
          std::string const* s;  // 5: std::string
          char const* c;         // 6: char const*
          void const* e;         // external
        } _value;

      public:

        explicit Jason (JasonType t = JasonType::Null) 
          : _jasonType(t), _cType(CType::None) {
        }
        explicit Jason (bool b, JasonType t = JasonType::Bool) 
          : _jasonType(t), _cType(CType::Bool) {
          _value.b = b;
        }
        explicit Jason (double d, JasonType t = JasonType::Double) 
          : _jasonType(t), _cType(CType::Double) {
          _value.d = d;
        }
        explicit Jason (void const* e, JasonType t = JasonType::External)
          : _jasonType(t), _cType(CType::VoidPtr) {
          _value.e = e;
        }
        explicit Jason (char const* c, JasonType t = JasonType::String)
          : _jasonType(t), _cType(CType::CharPtr) {
          _value.c = c;
        }
        explicit Jason (int32_t i, JasonType t = JasonType::Int)
          : _jasonType(t), _cType(CType::Int64) {
          _value.i = static_cast<int64_t>(i);
        }
        explicit Jason (uint32_t u, JasonType t = JasonType::UInt)
          : _jasonType(t), _cType(CType::UInt64) {
          _value.u = static_cast<uint64_t>(u);
        }
        explicit Jason (int64_t i, JasonType t = JasonType::Int)
          : _jasonType(t), _cType(CType::Int64) {
          _value.i = i;
        }
        explicit Jason (uint64_t u, JasonType t = JasonType::UInt)
          : _jasonType(t), _cType(CType::UInt64) {
          _value.u = u;
        }
        explicit Jason (std::string const& s, JasonType t = JasonType::String)
          : _jasonType(t), _cType(CType::String) {
          _value.s = &s;
        }

        JasonType jasonType () const {
          return _jasonType;
        }

        CType cType () const {
          return _cType;
        }

        bool isString () const {
          return _jasonType == JasonType::String;
        }

        bool getBool () const {
          JASON_ASSERT(_cType == Bool);
          return _value.b;
        }

        double getDouble () const {
          JASON_ASSERT(_cType == Double);
          return _value.d;
        }

        int64_t getInt64 () const {
          JASON_ASSERT(_cType == Int64);
          return _value.i;
        }

        uint64_t getUInt64 () const {
          JASON_ASSERT(_cType == UInt64);
          return _value.u;
        }

        std::string const* getString () const {
          JASON_ASSERT(_cType == String);
          return _value.s;
        }

        void const* getExternal () const {
          JASON_ASSERT(_cType == VoidPtr);
          return _value.e;
        }

        char const* getCharPtr () const {
          JASON_ASSERT(_cType == CharPtr);
          return _value.c;
        }

    };

    class JasonPair {
        uint8_t const* _start;
        uint64_t       _size;
        JasonType      _type;

      public:

        explicit JasonPair (uint8_t const* start, uint64_t size,
                            JasonType type = JasonType::Binary)
          : _start(start), _size(size), _type(type) {
        }

        explicit JasonPair (char const* start, uint64_t size,
                            JasonType type = JasonType::Binary)
          : _start(reinterpret_cast<uint8_t const*>(start)),
            _size(size), _type(type) {
        }

        explicit JasonPair (uint64_t size,
                            JasonType type = JasonType::Binary)
          : _start(nullptr), _size(size), _type(type) {
        }

        uint8_t const* getStart () const {
          return _start;
        }

        uint64_t getSize () const {
          return _size;
        }

        JasonType jasonType () const {
          return _type;
        }

        bool isString () const {
          return _type == JasonType::String;
        }
    };

    static inline uint64_t toUInt64 (int64_t v) {
      // If v is negative, we need to add 2^63 to make it positive,
      // before we can cast it to an uint64_t:
      uint64_t shift2 = 1ULL << 63;
      int64_t shift = static_cast<int64_t>(shift2 - 1);
      return v >= 0 ? static_cast<uint64_t>(v)
                    : static_cast<uint64_t>((v + shift) + 1) + shift2;
      // Note that g++ and clang++ with -O3 compile this away to
      // nothing. Further note that a plain cast from int64_t to
      // uint64_t is not guaranteed to work for negative values!
    }

    static inline int64_t toInt64 (uint64_t v) {
      uint64_t shift2 = 1ULL << 63;
      int64_t shift = static_cast<int64_t>(shift2 - 1);
      return v >= shift2 ? (static_cast<int64_t>(v - shift2) - shift) - 1
                         : static_cast<int64_t>(v);
    }
       
  }  // namespace arangodb::jason
}  // namespace arangodb

#endif
