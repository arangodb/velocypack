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

      using JT = JasonType;

      // This class provides read only access to a Jason value, it is
      // intentionally light-weight (only one pointer value), such that
      // it can easily be used to traverse larger Jason values.

        friend class JasonBuilder;

        uint8_t const* _start;

      public:
  
        JasonSlice () 
          : JasonSlice("\x00") {
        }

        explicit JasonSlice (uint8_t const* start) 
          : _start(start) {
        }

        explicit JasonSlice (char const* start) 
          : _start(reinterpret_cast<uint8_t const*>(start)) {
        }

        // No destructor, does not take part in memory management,
        // standard copy, and move constructors, behaves like a pointer.

        // get the type for the slice
        inline JasonType type () const {
          static JasonType const Types[256] = {
            /* 0x00 */  JT::None,        /* 0x01 */  JT::Null,        /* 0x02 */  JT::Bool,        /* 0x03 */  JT::Bool,        
            /* 0x04 */  JT::Double,      /* 0x05 */  JT::Array,       /* 0x06 */  JT::Array,       /* 0x07 */  JT::Object,      
            /* 0x08 */  JT::Object,      /* 0x09 */  JT::External,    /* 0x0a */  JT::ID,          /* 0x0b */  JT::ArangoDB_id, 
            /* 0x0c */  JT::String,      /* 0x0d */  JT::None,        /* 0x0e */  JT::None,        /* 0x0f */  JT::None,        
            /* 0x10 */  JT::UTCDate,     /* 0x11 */  JT::UTCDate,     /* 0x12 */  JT::UTCDate,     /* 0x13 */  JT::UTCDate,     
            /* 0x14 */  JT::UTCDate,     /* 0x15 */  JT::UTCDate,     /* 0x16 */  JT::UTCDate,     /* 0x17 */  JT::UTCDate,     
            /* 0x18 */  JT::Int,         /* 0x19 */  JT::Int,         /* 0x1a */  JT::Int,         /* 0x1b */  JT::Int,         
            /* 0x1c */  JT::Int,         /* 0x1d */  JT::Int,         /* 0x1e */  JT::Int,         /* 0x1f */  JT::Int,         
            /* 0x20 */  JT::Int,         /* 0x21 */  JT::Int,         /* 0x22 */  JT::Int,         /* 0x23 */  JT::Int,         
            /* 0x24 */  JT::Int,         /* 0x25 */  JT::Int,         /* 0x26 */  JT::Int,         /* 0x27 */  JT::Int,         
            /* 0x28 */  JT::UInt,        /* 0x29 */  JT::UInt,        /* 0x2a */  JT::UInt,        /* 0x2b */  JT::UInt,        
            /* 0x2c */  JT::UInt,        /* 0x2d */  JT::UInt,        /* 0x2e */  JT::UInt,        /* 0x2f */  JT::UInt,        
            /* 0x30 */  JT::SmallInt,    /* 0x31 */  JT::SmallInt,    /* 0x32 */  JT::SmallInt,    /* 0x33 */  JT::SmallInt,    
            /* 0x34 */  JT::SmallInt,    /* 0x35 */  JT::SmallInt,    /* 0x36 */  JT::SmallInt,    /* 0x37 */  JT::SmallInt,    
            /* 0x38 */  JT::SmallInt,    /* 0x39 */  JT::SmallInt,    /* 0x3a */  JT::SmallInt,    /* 0x3b */  JT::SmallInt,    
            /* 0x3c */  JT::SmallInt,    /* 0x3d */  JT::SmallInt,    /* 0x3e */  JT::SmallInt,    /* 0x3f */  JT::SmallInt,    
            /* 0x40 */  JT::String,      /* 0x41 */  JT::String,      /* 0x42 */  JT::String,      /* 0x43 */  JT::String,      
            /* 0x44 */  JT::String,      /* 0x45 */  JT::String,      /* 0x46 */  JT::String,      /* 0x47 */  JT::String,      
            /* 0x48 */  JT::String,      /* 0x49 */  JT::String,      /* 0x4a */  JT::String,      /* 0x4b */  JT::String,      
            /* 0x4c */  JT::String,      /* 0x4d */  JT::String,      /* 0x4e */  JT::String,      /* 0x4f */  JT::String,      
            /* 0x50 */  JT::String,      /* 0x51 */  JT::String,      /* 0x52 */  JT::String,      /* 0x53 */  JT::String,      
            /* 0x54 */  JT::String,      /* 0x55 */  JT::String,      /* 0x56 */  JT::String,      /* 0x57 */  JT::String,      
            /* 0x58 */  JT::String,      /* 0x59 */  JT::String,      /* 0x5a */  JT::String,      /* 0x5b */  JT::String,      
            /* 0x5c */  JT::String,      /* 0x5d */  JT::String,      /* 0x5e */  JT::String,      /* 0x5f */  JT::String,      
            /* 0x60 */  JT::String,      /* 0x61 */  JT::String,      /* 0x62 */  JT::String,      /* 0x63 */  JT::String,      
            /* 0x64 */  JT::String,      /* 0x65 */  JT::String,      /* 0x66 */  JT::String,      /* 0x67 */  JT::String,      
            /* 0x68 */  JT::String,      /* 0x69 */  JT::String,      /* 0x6a */  JT::String,      /* 0x6b */  JT::String,      
            /* 0x6c */  JT::String,      /* 0x6d */  JT::String,      /* 0x6e */  JT::String,      /* 0x6f */  JT::String,      
            /* 0x70 */  JT::String,      /* 0x71 */  JT::String,      /* 0x72 */  JT::String,      /* 0x73 */  JT::String,      
            /* 0x74 */  JT::String,      /* 0x75 */  JT::String,      /* 0x76 */  JT::String,      /* 0x77 */  JT::String,      
            /* 0x78 */  JT::String,      /* 0x79 */  JT::String,      /* 0x7a */  JT::String,      /* 0x7b */  JT::String,      
            /* 0x7c */  JT::String,      /* 0x7d */  JT::String,      /* 0x7e */  JT::String,      /* 0x7f */  JT::String,      
            /* 0x80 */  JT::String,      /* 0x81 */  JT::String,      /* 0x82 */  JT::String,      /* 0x83 */  JT::String,      
            /* 0x84 */  JT::String,      /* 0x85 */  JT::String,      /* 0x86 */  JT::String,      /* 0x87 */  JT::String,      
            /* 0x88 */  JT::String,      /* 0x89 */  JT::String,      /* 0x8a */  JT::String,      /* 0x8b */  JT::String,      
            /* 0x8c */  JT::String,      /* 0x8d */  JT::String,      /* 0x8e */  JT::String,      /* 0x8f */  JT::String,      
            /* 0x90 */  JT::String,      /* 0x91 */  JT::String,      /* 0x92 */  JT::String,      /* 0x93 */  JT::String,      
            /* 0x94 */  JT::String,      /* 0x95 */  JT::String,      /* 0x96 */  JT::String,      /* 0x97 */  JT::String,      
            /* 0x98 */  JT::String,      /* 0x99 */  JT::String,      /* 0x9a */  JT::String,      /* 0x9b */  JT::String,      
            /* 0x9c */  JT::String,      /* 0x9d */  JT::String,      /* 0x9e */  JT::String,      /* 0x9f */  JT::String,      
            /* 0xa0 */  JT::String,      /* 0xa1 */  JT::String,      /* 0xa2 */  JT::String,      /* 0xa3 */  JT::String,      
            /* 0xa4 */  JT::String,      /* 0xa5 */  JT::String,      /* 0xa6 */  JT::String,      /* 0xa7 */  JT::String,      
            /* 0xa8 */  JT::String,      /* 0xa9 */  JT::String,      /* 0xaa */  JT::String,      /* 0xab */  JT::String,      
            /* 0xac */  JT::String,      /* 0xad */  JT::String,      /* 0xae */  JT::String,      /* 0xaf */  JT::String,      
            /* 0xb0 */  JT::String,      /* 0xb1 */  JT::String,      /* 0xb2 */  JT::String,      /* 0xb3 */  JT::String,      
            /* 0xb4 */  JT::String,      /* 0xb5 */  JT::String,      /* 0xb6 */  JT::String,      /* 0xb7 */  JT::String,      
            /* 0xb8 */  JT::String,      /* 0xb9 */  JT::String,      /* 0xba */  JT::String,      /* 0xbb */  JT::String,      
            /* 0xbc */  JT::String,      /* 0xbd */  JT::String,      /* 0xbe */  JT::String,      /* 0xbf */  JT::String,      
            /* 0xc0 */  JT::Binary,      /* 0xc1 */  JT::Binary,      /* 0xc2 */  JT::Binary,      /* 0xc3 */  JT::Binary,      
            /* 0xc4 */  JT::Binary,      /* 0xc5 */  JT::Binary,      /* 0xc6 */  JT::Binary,      /* 0xc7 */  JT::Binary,      
            /* 0xc8 */  JT::BCD,         /* 0xc9 */  JT::BCD,         /* 0xca */  JT::BCD,         /* 0xcb */  JT::BCD,         
            /* 0xcc */  JT::BCD,         /* 0xcd */  JT::BCD,         /* 0xce */  JT::BCD,         /* 0xcf */  JT::BCD,         
            /* 0xd0 */  JT::BCD,         /* 0xd1 */  JT::BCD,         /* 0xd2 */  JT::BCD,         /* 0xd3 */  JT::BCD,         
            /* 0xd4 */  JT::BCD,         /* 0xd5 */  JT::BCD,         /* 0xd6 */  JT::BCD,         /* 0xd7 */  JT::BCD,         
            /* 0xd8 */  JT::None,        /* 0xd9 */  JT::None,        /* 0xda */  JT::None,        /* 0xdb */  JT::None,        
            /* 0xdc */  JT::None,        /* 0xdd */  JT::None,        /* 0xde */  JT::None,        /* 0xdf */  JT::None,        
            /* 0xe0 */  JT::None,        /* 0xe1 */  JT::None,        /* 0xe2 */  JT::None,        /* 0xe3 */  JT::None,        
            /* 0xe4 */  JT::None,        /* 0xe5 */  JT::None,        /* 0xe6 */  JT::None,        /* 0xe7 */  JT::None,        
            /* 0xe8 */  JT::None,        /* 0xe9 */  JT::None,        /* 0xea */  JT::None,        /* 0xeb */  JT::None,        
            /* 0xec */  JT::None,        /* 0xed */  JT::None,        /* 0xee */  JT::None,        /* 0xef */  JT::None,        
            /* 0xf0 */  JT::None,        /* 0xf1 */  JT::None,        /* 0xf2 */  JT::None,        /* 0xf3 */  JT::None,        
            /* 0xf4 */  JT::None,        /* 0xf5 */  JT::None,        /* 0xf6 */  JT::None,        /* 0xf7 */  JT::None,        
            /* 0xf8 */  JT::None,        /* 0xf9 */  JT::None,        /* 0xfa */  JT::None,        /* 0xfb */  JT::None,        
            /* 0xfc */  JT::None,        /* 0xfd */  JT::None,        /* 0xfe */  JT::None,        /* 0xff */  JT::None
          }; 

          return Types[head()];
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
        inline bool isType (JasonType t) const {
          return type() == t;
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
          uint8_t h = head();
          if (h >= 0x18 && h <= 0x1f) {
            // positive int
            return readInteger<int64_t>(_start + 1, h - 0x17);
          }
          else if (h >= 0x20 && h <= 0x27) { 
            // negative int
            return - readInteger<int64_t>(_start + 1, h - 0x1f);
          }
          else if (h >= 0x30 && h <= 0x3f) {
            // small int
            return getSmallInt();
          }

          throw JasonTypeError("unexpected type. expecting int");
        }

        // return the value for a UInt object
        uint64_t getUInt () const {
          uint8_t h = head();
          if (h >= 0x28 && h <= 0x2f) {
            // uint
            return readInteger<uint64_t>(_start + 1, head() - 0x27);
          }
          else if (h >= 0x30 && h <= 0x37) {
            // positive smallint
            return static_cast<uint64_t>(h - 0x30);
          }
          else if (h >= 0x18 && h <= 0x1f) {
            // positive int
            return readInteger<uint64_t>(_start + 1, h - 0x17);
          }
          
          throw JasonTypeError("unexpected type. expecting uint");
        }

        // return the value for a SmallInt object
        int64_t getSmallInt () const {
          uint8_t h = head();
          if (h >= 0x30 && h <= 0x37) {
            // positive
            return static_cast<int64_t>(h - 0x30);
          }
          else if (h >= 0x38 && h <= 0x3f) {
            // negative
            return static_cast<int64_t>(h - 0x38) - 8;
          }
          else if (h >= 0x18 && h <= 0x27) {
            // regular int
            return getInt();
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
            JasonCheckSize(length);
            return std::string(reinterpret_cast<char const*>(_start + 1), static_cast<size_t>(length));
          }
          if (h == 0x0c) {
            JasonLength length = readInteger<JasonLength>(_start + 1, 8);
            JasonCheckSize(length);
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
            JasonCheckSize(length);
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
            JasonCheckSize(length);
            out.reserve(static_cast<size_t>(length));
            out.insert(out.end(), _start + 1 + h - 0xbf, _start + 1 + h - 0xbf + length);
            return out; 
          }
          throw JasonTypeError("unexpected type. expecting binary");
        }

        // get the total byte size for the slice, including the head byte
        JasonLength byteSize () const {
          switch (type()) {
            case JasonType::None:
            case JasonType::Null:
            case JasonType::Bool: 
            case JasonType::ArangoDB_id:
            case JasonType::ID: 
            case JasonType::SmallInt: {
              return 1; 
            }

            case JasonType::Double: {
              return 1 + sizeof(double);
            }

            case JasonType::Array:
            case JasonType::Object: {
              uint8_t b = _start[1];
              if (b != 0x00) {
                // 1 byte length: already got the length
                return static_cast<JasonLength>(b);
              }
              // 8 byte length: read the following 8 bytes
              return readInteger<JasonLength>(_start + 1 + 1, 8);
            }

            case JasonType::External: {
              return 1 + sizeof(char*);
            }

            case JasonType::UTCDate:
              return static_cast<JasonLength>(head() - 0x0f);

            case JasonType::Int: {
              if (head() <= 0x1f) {
                // positive int
                return static_cast<JasonLength>(1 + (head() - 0x17));
              }
              // negative int
              return static_cast<JasonLength>(1 + (head() - 0x1f));
            }

            case JasonType::UInt: {
              return static_cast<JasonLength>(1 + (head() - 0x27));
            }

            case JasonType::String: {
              auto h = head();
              if (h == 0x0c) {
                return static_cast<JasonLength>(1 + 8 + readInteger<JasonLength>(_start + 1, 8));
              }
              return static_cast<JasonLength>(1 + (head() - 0x40));
            }

            case JasonType::Binary: {
              return static_cast<JasonLength>(1 + (head() - 0xbf) + readInteger<JasonLength>(_start + 1, head() - 0xbf));
            }

            case JasonType::BCD: {
              uint8_t base;
              if (head() <= 0xcf) {
                // positive BCD
                base = 0xc7;
              } 
              else {
                // negative BCD
                base = 0xcf;
              }
              return static_cast<JasonLength>(1 + (head() - base) + readInteger<JasonLength>(_start + 1, head() - base));
            }
          }

          assert(false);
          return 0;
        }

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

        // maximum number of attributes in an object for which a linear
        // search is performed
        static JasonLength const MaxLengthForLinearSearch = 8;

    };

    static_assert(sizeof(JasonSlice) == sizeof(void*), "JasonSlice has an unexpected size");

  }  // namespace triagens::basics
}  // namespace triagens

#endif
