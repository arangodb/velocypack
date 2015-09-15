#ifndef JASON_H
#define JASON_H 1

#include <cstdint>
#include <string>
#include <cassert>

#include "JasonType.h"

namespace triagens {
  namespace basics {

    // unified size type for Jason, can be used on 32 and 64 bit
    // though no Jason values exceeded the bounds of 32 bit can be
    // used on a 32 bit system
    typedef uint64_t JasonLength;

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
          if (_jasonType == JasonType::String &&
              static_cast<JasonLength>(s.size()) > Jason::MaxLengthString) {
            // make it a long string automatically if size too long
            _jasonType = JasonType::StringLong;
          }
          _value.s = &s;
        }

        JasonType jasonType () const {
          return _jasonType;
        }

        CType cType () const {
          return _cType;
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

      public:   

        static JasonLength const MaxLengthString;
    };

    class JasonPair {
        uint8_t* _start;
        uint64_t _size;
        JasonType _type;

      public:

        explicit JasonPair (uint8_t* start, uint64_t size,
                            JasonType type = JasonType::Binary)
          : _start(start), _size(size), _type(type) {
        }

        uint8_t* getStart () {
          return _start;
        }

        uint64_t getSize () {
          return _size;
        }

        JasonType getType () {
          return _type;
        }
    };

  }  // namespace triagens::basics
}  // namespace triagens

#endif
