
#include "JasonSlice.h"
#include "JasonType.h"

using JasonSlice = triagens::basics::JasonSlice;
using JasonType = triagens::basics::JasonType;

////////////////////////////////////////////////////////////////////////////////
/// @brief a lookup table for Jason types
////////////////////////////////////////////////////////////////////////////////

std::array<JasonType, 256> JasonSlice::TypeTable;

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
  TypeTable[0x5] = JasonType::Array;
  TypeTable[0x6] = JasonType::Object;
  TypeTable[0x7] = JasonType::Object;
  TypeTable[0x8] = JasonType::External;
  TypeTable[0x9] = JasonType::ID;
  TypeTable[0xa] = JasonType::ArangoDB_id;
 
  for (int i = 0x10; i <= 0x1f; ++i) { 
    TypeTable[i] = JasonType::UTCDate;
  }
  for (int i = 0x20; i <= 0x2f; ++i) { 
    TypeTable[i] = JasonType::Int;
  }
  for (int i = 0x30; i <= 0x3f; ++i) { 
    TypeTable[i] = JasonType::UInt;
  }
  for (int i = 0x40; i <= 0xcf; ++i) { 
    TypeTable[i] = JasonType::String;
  }
  for (int i = 0xd0; i <= 0xdf; ++i) { 
    TypeTable[i] = JasonType::Binary;
  }
}

