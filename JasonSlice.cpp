
#include "JasonSlice.h"
#include "JasonType.h"

using JasonSlice = triagens::basics::JasonSlice;
using JasonType = triagens::basics::JasonType;

////////////////////////////////////////////////////////////////////////////////
/// @brief a lookup table for Jason types
////////////////////////////////////////////////////////////////////////////////

std::array<JasonType, 256> JasonSlice::TypeTable;

////////////////////////////////////////////////////////////////////////////////
/// @brief cached powers
////////////////////////////////////////////////////////////////////////////////

std::array<uint64_t, 8> JasonSlice::Powers;

////////////////////////////////////////////////////////////////////////////////
/// @brief get the byte size of the payload
////////////////////////////////////////////////////////////////////////////////

uint64_t JasonSlice::byteSize () const {
  switch (type()) {
    case JasonType::None:
    case JasonType::Null:
    case JasonType::Bool:
      return 0;

    case JasonType::Double:
      return 8;

    case JasonType::Array:
      return static_cast<uint64_t>(readLength(_start + 2, 2));
       
    case JasonType::ArrayLong:
      return 0; // TODO

    case JasonType::Object:
      return static_cast<uint64_t>(readLength(_start + 2, 2));

    case JasonType::ObjectLong:
      return 0; // TODO

    case JasonType::External:
      return 0; // TODO

    case JasonType::ID:
      return 0; // TODO

    case JasonType::ArangoDB_id:
      return 0; // TODO

    case JasonType::UTCDate:
      return readLength(*_start - 0xf);

    case JasonType::Int:
      if (*_start <= 0x27) {
        // positive int
        return readLength(*_start - 0x1f);
      }
      // negative int
      return readLength(*_start - 0x27);

    case JasonType::UInt:
      return readLength(*_start - 0x2f);

    case JasonType::String:
      if (*_start <= 0xbf) {
        // short string
        return (*_start - 0x40);
      }
      // long string
      return readLength(*_start - 0xbf);

    case JasonType::Binary: 
      return readLength(*_start - 0xcf);
  }

  assert(false);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief initialize the Jason handling
////////////////////////////////////////////////////////////////////////////////

void JasonSlice::Initialize () {
  // initialize lookup table to point to no specific type 
  for (int i = 0x0; i <= 0xff; ++i) {
    TypeTable[i] = JasonType::None;
  }

  TypeTable[0x0] = JasonType::Null;
  TypeTable[0x1] = JasonType::Bool;
  TypeTable[0x2] = JasonType::Bool;
  TypeTable[0x3] = JasonType::Double;
  TypeTable[0x4] = JasonType::Array;
  TypeTable[0x5] = JasonType::ArrayLong;
  TypeTable[0x6] = JasonType::Object;
  TypeTable[0x7] = JasonType::ObjectLong;
  TypeTable[0x8] = JasonType::External;
  TypeTable[0x9] = JasonType::ID;
  TypeTable[0xa] = JasonType::ArangoDB_id;
 
  for (int i = 0x10; i <= 0x17; ++i) { 
    TypeTable[i] = JasonType::UTCDate;
  }
  for (int i = 0x20; i <= 0x2f; ++i) { 
    TypeTable[i] = JasonType::Int;
  }
  for (int i = 0x30; i <= 0x37; ++i) { 
    TypeTable[i] = JasonType::UInt;
  }
  for (int i = 0x40; i <= 0xc7; ++i) { 
    TypeTable[i] = JasonType::String;
  }
  for (int i = 0xd0; i <= 0xd7; ++i) { 
    TypeTable[i] = JasonType::Binary;
  }

  uint64_t value = 0;
  for (int i = 0; i < 8; ++i) {
    Powers[i] = value;
    value = value << 8;
  } 
}

