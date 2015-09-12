
#include <iostream>

#include "Jason.h"
#include "JasonBuilder.h"
#include "JasonParser.h"
#include "JasonSlice.h"
#include "JasonType.h"

using Jason        = triagens::basics::Jason;
using JasonBuilder = triagens::basics::JasonBuilder;
using JasonSlice   = triagens::basics::JasonSlice;
using JasonType    = triagens::basics::JasonType;
  
static char Buffer[4096];

static void TestNull () {
  Buffer[0] = 0x0;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::Null);
  assert(slice.isNull());
  assert(slice.byteSize() == 0);
}

static void TestFalse () {
  Buffer[0] = 0x1;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::Bool);
  assert(slice.isBool());
  assert(slice.byteSize() == 0);
  assert(slice.getBool() == false);
}

static void TestTrue () {
  Buffer[0] = 0x2;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::Bool);
  assert(slice.isBool());
  assert(slice.byteSize() == 0);
  assert(slice.getBool() == true);
}

static void TestDouble () {
  Buffer[0] = 0x3;
  double value = 23.5;
  memcpy(&Buffer[1], (void*) &value, sizeof(value));

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::Double);
  assert(slice.isDouble());
  assert(slice.byteSize() == 8);
  assert(slice.getDouble() == value);
}

static void TestInt1 () {
  Buffer[0] = 0x20;
  uint8_t value = 0x33;
  memcpy(&Buffer[1], (void*) &value, sizeof(value));

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::Int);
  assert(slice.isInt());
  assert(slice.byteSize() == 1);

  assert(slice.getInt() == value);
}

static void TestInt2 () {
  Buffer[0] = 0x21;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::Int);
  assert(slice.isInt());
  assert(slice.byteSize() == 2);
  assert(slice.getInt() == 0x23 + 0x100 * 0x42);
}

static void TestInt3 () {
  Buffer[0] = 0x22;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::Int);
  assert(slice.isInt());
  assert(slice.byteSize() == 3);
  assert(slice.getInt() == 0x23 + 0x100 * 0x42 + 0x10000 * 0x66);
}

static void TestInt4 () {
  Buffer[0] = 0x23;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::Int);
  assert(slice.isInt());
  assert(slice.byteSize() == 4);
  assert(slice.getInt() == 0x23 + 0x100ULL * 0x42ULL + 0x10000ULL * 0x66ULL + 0x1000000ULL * 0xacULL);
}

static void TestNegInt1 () {
  Buffer[0] = 0x28;
  uint8_t value = 0x33;
  memcpy(&Buffer[1], (void*) &value, sizeof(value));

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::Int);
  assert(slice.isInt());
  assert(slice.byteSize() == 1);

  assert(slice.getInt() == - value);
}

static void TestNegInt2 () {
  Buffer[0] = 0x29;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::Int);
  assert(slice.isInt());
  assert(slice.byteSize() == 2);
  assert(slice.getInt() == - (0x23 + 0x100 * 0x42));
}

static void TestNegInt3 () {
  Buffer[0] = 0x2a;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::Int);
  assert(slice.isInt());
  assert(slice.byteSize() == 3);
  assert(slice.getInt() == - (0x23 + 0x100 * 0x42 + 0x10000 * 0x66));
}

static void TestNegInt4 () {
  Buffer[0] = 0x2b;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::Int);
  assert(slice.isInt());
  assert(slice.byteSize() == 4);
  assert(slice.getInt() == - (0x23 + 0x100LL * 0x42LL + 0x10000LL * 0x66LL + 0x1000000LL * 0xacLL));
}

static void TestUInt1 () {
  Buffer[0] = 0x30;
  uint8_t value = 0x33;
  memcpy(&Buffer[1], (void*) &value, sizeof(value));

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::UInt);
  assert(slice.isUInt());
  assert(slice.byteSize() == 1);

  assert(slice.getUInt() == value);
}

static void TestUInt2 () {
  Buffer[0] = 0x31;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::UInt);
  assert(slice.isUInt());
  assert(slice.byteSize() == 2);
  assert(slice.getUInt() == (0x23 + 0x100 * 0x42));
}

static void TestUInt3 () {
  Buffer[0] = 0x32;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::UInt);
  assert(slice.isUInt());
  assert(slice.byteSize() == 3);
  assert(slice.getUInt() == (0x23 + 0x100 * 0x42 + 0x10000 * 0x66));
}

static void TestUInt4 () {
  Buffer[0] = 0x33;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::UInt);
  assert(slice.isUInt());
  assert(slice.byteSize() == 4);
  assert(slice.getUInt() == (0x23 + 0x100ULL * 0x42ULL + 0x10000ULL * 0x66ULL + 0x1000000ULL * 0xacULL));
}

static void TestArrayEmpty () {
  Buffer[0] = 0x04;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x00;
  *p++ = 0x03;
  *p++ = 0x00;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::Array);
  assert(slice.isArray());
  assert(slice.byteSize() == 3);
  assert(slice.length() == 0);
}

static void TestBuilderArrayEmpty () {
  JasonBuilder b;
  b.set(Jason(0, JasonType::Array));
  b.close();
}

static void TestBuilderArray3 () {
  JasonBuilder b;
  b.set(Jason(4, JasonType::Array));
  b.add(Jason(uint64_t(1200)));
  b.add(Jason(2.3));
  b.add(Jason("abc"));
  b.add(Jason(true));
  b.close();
}

int main (int argc, char* argv[]) {
  JasonSlice::Initialize();

  TestNull();
  TestFalse();
  TestTrue();
  TestDouble();
  TestInt1();
  TestInt2();
  TestInt3();
  TestInt4();
  TestNegInt1();
  TestNegInt2();
  TestNegInt3();
  TestNegInt4();
  TestUInt1();
  TestUInt2();
  TestUInt3();
  TestUInt4();
  TestArrayEmpty();
  TestBuilderArrayEmpty();
  TestBuilderArray3();

  std::cout << "ye olde tests passeth.\n";
  return 0;
}
