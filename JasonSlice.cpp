
#include "JasonSlice.h"
#include "JasonType.h"

using JasonLength = triagens::basics::JasonLength;
using JasonSlice  = triagens::basics::JasonSlice;
using JasonType   = triagens::basics::JasonType;

// a lookup table for Jason types
std::array<JasonType, 256> JasonSlice::TypeTable;

// get the total byte size for the object, including the head byte
JasonLength JasonSlice::byteSize () const {
  switch (type()) {
    case JasonType::None:
    case JasonType::Null:
    case JasonType::Bool:
      return 1;

    case JasonType::Double:
      return 1 + sizeof(double);

    case JasonType::Array:
      // TODO
      return readInteger<JasonLength>(_start + 2, 2);
       
    case JasonType::ArrayLong:
      // TODO
      return readInteger<JasonLength>(_start + 7, 8);

    case JasonType::Object:
      // TODO
      return readInteger<JasonLength>(_start + 2, 2);

    case JasonType::ObjectLong:
      return 0; // TODO

    case JasonType::External:
      return 1 + sizeof(char*);

    case JasonType::ID:
      return 1; // TODO

    case JasonType::ArangoDB_id:
      return 1; 

    case JasonType::UTCDate:
      return 1 + readInteger<JasonLength>(head() - 0x0f);

    case JasonType::Int: {
      uint8_t h = head();
      if (h <= 0x27) {
        // positive int
        return 1 + (h - 0x1f);
      }
      // negative int
      return 1 + (h - 0x27);
    }

    case JasonType::UInt: {
      return 1 + (head() - 0x2f);
    }

    case JasonType::String:
    case JasonType::StringLong: {
      uint8_t h = head();
      if (h <= 0xbf) {
        // short string
        return 1 + (h - 0x40);
      }
      // long string
      return 1 + (h - 0xbf) + readInteger<JasonLength>(h - 0xbf);
    }

    case JasonType::Binary: {
      uint8_t h = head();
      return 1 + (h - 0xcf) + readInteger<JasonLength>(h - 0xcf);
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

  TypeTable[0x00] = JasonType::Null;
  TypeTable[0x01] = JasonType::Bool;
  TypeTable[0x02] = JasonType::Bool;
  TypeTable[0x03] = JasonType::Double;
  TypeTable[0x04] = JasonType::Array;
  TypeTable[0x05] = JasonType::ArrayLong;
  TypeTable[0x06] = JasonType::Object;
  TypeTable[0x07] = JasonType::ObjectLong;
  TypeTable[0x08] = JasonType::External;
  TypeTable[0x09] = JasonType::ID;
  TypeTable[0x0a] = JasonType::ArangoDB_id;
 
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
}

