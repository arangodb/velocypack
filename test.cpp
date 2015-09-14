
#include <iostream>

#include "Jason.h"
#include "JasonBuilder.h"
#include "JasonParser.h"
#include "JasonSlice.h"
#include "JasonType.h"

using Jason        = triagens::basics::Jason;
using JasonBuilder = triagens::basics::JasonBuilder;
using JasonLength  = triagens::basics::JasonLength;
using JasonSlice   = triagens::basics::JasonSlice;
using JasonType    = triagens::basics::JasonType;
  
static char Buffer[4096];

static void TestNull () {
  Buffer[0] = 0x0;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::Null);
  assert(slice.isNull());
  assert(slice.byteSize() == 1);
}

static void TestFalse () {
  Buffer[0] = 0x1;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::Bool);
  assert(slice.isBool());
  assert(slice.byteSize() == 1);
  assert(slice.getBool() == false);
}

static void TestTrue () {
  Buffer[0] = 0x2;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::Bool);
  assert(slice.isBool());
  assert(slice.byteSize() == 1);
  assert(slice.getBool() == true);
}

static void TestDouble () {
  Buffer[0] = 0x3;
  double value = 23.5;
  memcpy(&Buffer[1], (void*) &value, sizeof(value));

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::Double);
  assert(slice.isDouble());
  assert(slice.byteSize() == 9);
  assert(slice.getDouble() == value);
}

static void TestInt1 () {
  Buffer[0] = 0x20;
  uint8_t value = 0x33;
  memcpy(&Buffer[1], (void*) &value, sizeof(value));

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::Int);
  assert(slice.isInt());
  assert(slice.byteSize() == 2);

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
  assert(slice.byteSize() == 3);
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
  assert(slice.byteSize() == 4);
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
  assert(slice.byteSize() == 5);
  assert(slice.getInt() == 0x23 + 0x100ULL * 0x42ULL + 0x10000ULL * 0x66ULL + 0x1000000ULL * 0xacULL);
}

static void TestNegInt1 () {
  Buffer[0] = 0x28;
  uint8_t value = 0x33;
  memcpy(&Buffer[1], (void*) &value, sizeof(value));

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::Int);
  assert(slice.isInt());
  assert(slice.byteSize() == 2);

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
  assert(slice.byteSize() == 3);
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
  assert(slice.byteSize() == 4);
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
  assert(slice.byteSize() == 5);
  assert(slice.getInt() == - (0x23 + 0x100LL * 0x42LL + 0x10000LL * 0x66LL + 0x1000000LL * 0xacLL));
}

static void TestUInt1 () {
  Buffer[0] = 0x30;
  uint8_t value = 0x33;
  memcpy(&Buffer[1], (void*) &value, sizeof(value));

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::UInt);
  assert(slice.isUInt());
  assert(slice.byteSize() == 2);

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
  assert(slice.byteSize() == 3);
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
  assert(slice.byteSize() == 4);
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
  assert(slice.byteSize() == 5);
  assert(slice.getUInt() == (0x23 + 0x100ULL * 0x42ULL + 0x10000ULL * 0x66ULL + 0x1000000ULL * 0xacULL));
}

static void TestArrayEmpty () {
  Buffer[0] = 0x04;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x00;
  *p++ = 0x04;
  *p++ = 0x00;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::Array);
  assert(slice.isArray());
  assert(slice.byteSize() == 4);
  assert(slice.length() == 0);
}

static void TestStringEmpty () {
  Buffer[0] = 0x40;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::String);
  assert(slice.isString());
  assert(slice.byteSize() == 1);
  JasonLength len;
  char const* s = slice.getString(len);
  assert(len == 0);
  assert(strncmp(s, "", len) == 0);

  assert(slice.copyString() == "");
}

static void TestString1 () {
  Buffer[0] = 0x40 + strlen("foobar");

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = (uint8_t) 'f';
  *p++ = (uint8_t) 'o';
  *p++ = (uint8_t) 'o';
  *p++ = (uint8_t) 'b';
  *p++ = (uint8_t) 'a';
  *p++ = (uint8_t) 'r';

  assert(slice.type() == JasonType::String);
  assert(slice.isString());
  assert(slice.byteSize() == 7);
  JasonLength len;
  char const* s = slice.getString(len);
  assert(len == 6);
  assert(strncmp(s, "foobar", len) == 0);

  assert(slice.copyString() == "foobar");
}

static void TestString2 () {
  Buffer[0] = 0x48;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = (uint8_t) '1';
  *p++ = (uint8_t) '2';
  *p++ = (uint8_t) '3';
  *p++ = (uint8_t) 'f';
  *p++ = (uint8_t) '\r';
  *p++ = (uint8_t) '\t';
  *p++ = (uint8_t) '\n';
  *p++ = (uint8_t) 'x';

  assert(slice.type() == JasonType::String);
  assert(slice.isString());
  assert(slice.byteSize() == 9);
  JasonLength len;
  char const* s = slice.getString(len);
  assert(len == 8);
  assert(strncmp(s, "123f\r\t\nx", len) == 0);

  assert(slice.copyString() == "123f\r\t\nx");
}

static void TestStringNull () {
  Buffer[0] = 0x48;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = (uint8_t) '\0';
  *p++ = (uint8_t) '1';
  *p++ = (uint8_t) '2';
  *p++ = (uint8_t) '\0';
  *p++ = (uint8_t) '3';
  *p++ = (uint8_t) '4';
  *p++ = (uint8_t) '\0';
  *p++ = (uint8_t) 'x';

  assert(slice.type() == JasonType::String);
  assert(slice.isString());
  assert(slice.byteSize() == 9);
  JasonLength len;
  slice.getString(len);
  assert(len == 8);

  std::string s(slice.copyString());
  assert(s.size() == 8);
  assert(s[0] == '\0');
  assert(s[1] == '1');
  assert(s[2] == '2');
  assert(s[3] == '\0');
  assert(s[4] == '3');
  assert(s[5] == '4');
  assert(s[6] == '\0');
  assert(s[7] == 'x');
}

static void TestStringLong1 () {
  Buffer[0] = 0xc0;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = (uint8_t) 6;
  *p++ = (uint8_t) 'f';
  *p++ = (uint8_t) 'o';
  *p++ = (uint8_t) 'o';
  *p++ = (uint8_t) 'b';
  *p++ = (uint8_t) 'a';
  *p++ = (uint8_t) 'r';

  assert(slice.type() == JasonType::String);
  assert(slice.isString());
  assert(slice.byteSize() == 8);
  JasonLength len;
  char const* s = slice.getString(len);
  assert(len == 6);
  assert(strncmp(s, "foobar", len) == 0);

  assert(slice.copyString() == "foobar");
}

static void TestBuilderNull () {
  JasonBuilder b;
  b.set(Jason());
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t const correctResult[] 
    = { 0x00 };

  assert(len == sizeof(correctResult));
  assert(memcmp(result, correctResult, len) == 0);
}

static void TestBuilderFalse () {
  JasonBuilder b;
  b.set(Jason(false));
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t const correctResult[] 
    = { 0x01 };

  assert(len == sizeof(correctResult));
  assert(memcmp(result, correctResult, len) == 0);
}

static void TestBuilderTrue () {
  JasonBuilder b;
  b.set(Jason(true));
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t const correctResult[] 
    = { 0x02 };

  assert(len == sizeof(correctResult));
  assert(memcmp(result, correctResult, len) == 0);
}

static void TestBuilderDouble () {
  static double value = 123.456;
  JasonBuilder b;
  b.set(Jason(value));
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t correctResult[9] 
    = { 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  assert(sizeof(double) == 8);
  memcpy(correctResult + 1, &value, sizeof(value));

  assert(len == sizeof(correctResult));
  assert(memcmp(result, correctResult, len) == 0);
}

static void TestBuilderString () {
  JasonBuilder b;
  b.set(Jason("abcdefghijklmnopqrstuvwxyz"));
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t correctResult[] 
    = { 0x5a, 
        0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b,
        0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
        0x77, 0x78, 0x79, 0x7a };

  assert(len == sizeof(correctResult));
  assert(memcmp(result, correctResult, len) == 0);
}

static void TestBuilderArrayEmpty () {
  JasonBuilder b;
  b.set(Jason(0, JasonType::Array));
  b.close();
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t correctResult[] 
    = { 0x04, 0x00, 0x04, 0x00 };

  assert(len == sizeof(correctResult));
  assert(memcmp(result, correctResult, len) == 0);
}

static void TestBuilderArray4 () {
  double value = 2.3;
  JasonBuilder b;
  b.set(Jason(4, JasonType::Array));
  b.add(Jason(uint64_t(1200)));
  b.add(Jason(value));
  b.add(Jason("abc"));
  b.add(Jason(true));
  b.close();

  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t correctResult[] 
    = { 0x04, 0x04, 0x1b, 0x00,
        0x0d, 0x00, 0x16, 0x00, 0x1a, 0x00,
        0x31, 0xb0, 0x04,   // uint(1200) = 0x4b0
        0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // double(2.3)
        0x43, 0x61, 0x62, 0x63,
        0x02 };
  memcpy(correctResult + 14, &value, 8);

  assert(len == sizeof(correctResult));
  assert(memcmp(result, correctResult, len) == 0);
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
  TestStringEmpty();
  TestString1();
  TestString2();
  TestStringNull();
  TestStringLong1();
  TestArrayEmpty();
  TestBuilderNull();
  TestBuilderFalse();
  TestBuilderTrue();
  TestBuilderDouble();
  TestBuilderString();
  TestBuilderArrayEmpty();
  TestBuilderArray4();

  std::cout << "ye olde tests passeth.\n";
  return 0;
}
