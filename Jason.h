#ifndef JASON_H
#define JASON_H 1

#include <cstdint>
#include <string>
#include <cassert>

#include "JasonType.h"

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

namespace triagens {
  namespace basics {

    // unified size type for Jason, can be used on 32 and 64 bit
    // though no Jason values exceeded the bounds of 32 bit can be
    // used on a 32 bit system
    typedef uint64_t JasonLength;

    static_assert(sizeof(JasonLength) >= sizeof(SIZE_MAX), "invalid value for SIZE_MAX");

    // base exception class
    struct JasonException : std::exception {
      private:
        std::string _msg;
      public:
        JasonException (std::string const& msg) : _msg(msg) {
        }
        char const* what() const noexcept {
          return _msg.c_str();
        }
    };

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
          assert(_cType == Bool);
          return _value.b;
        }

        double getDouble () const {
          assert(_cType == Double);
          return _value.d;
        }

        int64_t getInt64 () const {
          assert(_cType == Int64);
          return _value.i;
        }

        uint64_t getUInt64 () const {
          assert(_cType == UInt64);
          return _value.u;
        }

        std::string const* getString () const {
          assert(_cType == String);
          return _value.s;
        }

        void const* getExternal () const {
          assert(_cType == VoidPtr);
          return _value.e;
        }

        char const* getCharPtr () const {
          assert(_cType == CharPtr);
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

    struct JasonOptions {
      bool checkAttributeUniqueness = false;
    };
          
  }  // namespace triagens::basics
}  // namespace triagens

#endif
