#ifndef JASON_H
#define JASON_H 1

#include <cstdint>
#include <string>
#include <cassert>

#include "JasonType.h"

namespace triagens {
  namespace basics {

    class Jason {
      // Convenience class for more compact notation

      friend class JasonBuilder;

      public:

        enum CType {
          None = 0,
          Bool = 1,
          Double = 2,
          Int64 = 3,
          UInt64 = 4,
          String = 5
        };
       
      private:

        JasonType _jasonType;
        CType     _cType;    // denotes variant used, 0: none

        union {
          bool b;          // 1: bool
          double d;        // 2: double
          int64_t i;       // 3: int64_t
          uint64_t u;      // 4: uint64_t
          std::string* s;  // 5: std::string
        } _value;

      public:

        Jason (JasonType t = JasonType::Null) 
          : _jasonType(t), _cType(CType::None) {
        }
        Jason (bool b, JasonType t = JasonType::Bool) 
          : _jasonType(t), _cType(CType::Bool) {
          _value.b = b;
        }
        Jason (double d, JasonType t = JasonType::Double) 
          : _jasonType(t), _cType(CType::Double) {
          _value.d = d;
        }
        Jason (int64_t i, JasonType t = JasonType::Int)
          : _jasonType(t), _cType(CType::Int64) {
          _value.i = i;
        }
        Jason (uint64_t u, JasonType t = JasonType::UInt)
          : _jasonType(t), _cType(CType::UInt64) {
          _value.u = u;
        }
        Jason (std::string& s, JasonType t = JasonType::String)
          : _jasonType(t), _cType(CType::String) {
          _value.s = &s;
        }

        JasonType jasonType () {
          return _jasonType;
        }

        CType cType () {
          return _cType;
        }

        bool getBool () {
          assert(_cType == Bool);
          return _value.b;
        }

        double getDouble () {
          assert(_cType == Double);
          return _value.d;
        }

        int64_t getInt64 () {
          assert(_cType == Int64);
          return _value.i;
        }

        uint64_t getUInt64 () {
          assert(_cType == UInt64);
          return _value.u;
        }

        std::string* getString () {
          assert(_cType == String);
          return _value.s;
        }

    };

  }  // namespace triagens::basics
}  // namespace triagens

#endif
