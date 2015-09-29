
#include "JasonSlice.h"
#include "JasonType.h"

using JasonLength = triagens::basics::JasonLength;
using JasonSlice  = triagens::basics::JasonSlice;
using JasonType   = triagens::basics::JasonType;

static_assert(sizeof(JasonSlice) == sizeof(void*), "JasonSlice has an unexpected size");

static uint8_t const NoneValue = 0x00;

// a lookup table for Jason types
std::array<JasonType, 256> JasonSlice::TypeTable;
        
// a built-in "not found" value slice
uint8_t const* JasonSlice::NotFoundSliceData = &NoneValue;
        
// maximum number of attributes in an object for which a linear
// search is performed
JasonLength const JasonSlice::MaxLengthForLinearSearch = 8;

// get the total byte size for the object, including the head byte
JasonLength JasonSlice::byteSize () const {
  switch (type()) {
    case JasonType::None:
    case JasonType::Null:
    case JasonType::Bool: 
    case JasonType::ArangoDB_id:
    case JasonType::SmallInt: {
      return 1;
    }

    case JasonType::Double: {
      return 1 + sizeof(double);
    }

    case JasonType::Array:
    case JasonType::ArrayLong:
    case JasonType::Object:
    case JasonType::ObjectLong: {
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

    case JasonType::ID: {
      return 1; // TODO
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
      return static_cast<JasonLength>(1 + (head() - 0x40));
    }

    case JasonType::StringLong: {
      return static_cast<JasonLength>(1 + 8 + readInteger<JasonLength>(_start + 1, 8));
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

////////////////////////////////////////////////////////////////////////////////
/// @brief initialize the Jason handling
////////////////////////////////////////////////////////////////////////////////

void JasonSlice::Initialize () {
  // initialize lookup table to point to no specific type 
  for (int i = 0x00; i <= 0xff; ++i) {
    TypeTable[i] = JasonType::None;
  }

  TypeTable[0x01] = JasonType::Null;        // null value
  TypeTable[0x02] = JasonType::Bool;        // false
  TypeTable[0x03] = JasonType::Bool;        // true
  TypeTable[0x04] = JasonType::Double;      // IEEE754 double
  TypeTable[0x05] = JasonType::Array;       // short array
  TypeTable[0x06] = JasonType::ArrayLong;   // long array
  TypeTable[0x07] = JasonType::Object;      // short object
  TypeTable[0x08] = JasonType::ObjectLong;  // long object
  TypeTable[0x09] = JasonType::External;    // external
  TypeTable[0x0a] = JasonType::ID;          // id type 
  TypeTable[0x0b] = JasonType::ArangoDB_id; // ArangoDB _id
  TypeTable[0x0c] = JasonType::StringLong;  // long UTF-8 string
 
  for (int i = 0x10; i <= 0x17; ++i) { 
    TypeTable[i] = JasonType::UTCDate;      // UTC date
  }
  for (int i = 0x18; i <= 0x27; ++i) { 
    TypeTable[i] = JasonType::Int;          // positive int, negative int
  }
  for (int i = 0x28; i <= 0x2f; ++i) { 
    TypeTable[i] = JasonType::UInt;         // uint
  }
  for (int i = 0x30; i <= 0x3f; ++i) { 
    TypeTable[i] = JasonType::SmallInt;     // small integers -8..7
  }
  for (int i = 0x40; i <= 0xbf; ++i) { 
    TypeTable[i] = JasonType::String;       // short UTF-8 string
  }
  for (int i = 0xc0; i <= 0xc7; ++i) { 
    TypeTable[i] = JasonType::Binary;       // binary
  }
  for (int i = 0xc8; i <= 0xd7; ++i) { 
    TypeTable[i] = JasonType::BCD;          // positive and negative packed BCD-encoded integers
  }

  // reserved
  assert(TypeTable[0x0d] == JasonType::None); 
  assert(TypeTable[0x0e] == JasonType::None); 
  assert(TypeTable[0x0f] == JasonType::None); 
  for (int i = 0xd8; i <= 0xff; ++i) { 
    assert(TypeTable[i] == JasonType::None); 
  }
}

