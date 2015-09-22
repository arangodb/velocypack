
#include <iostream>
#include <string>

#include "Jason.h"
#include "JasonBuilder.h"
#include "JasonDumper.h"
#include "JasonParser.h"
#include "JasonSlice.h"
#include "JasonType.h"

#include "gtest/gtest.h"

using Jason            = triagens::basics::Jason;
using JasonBuffer      = triagens::basics::JasonBuffer;
using JasonBuilder     = triagens::basics::JasonBuilder;
using JasonDumper      = triagens::basics::JasonDumper;
using JasonLength      = triagens::basics::JasonLength;
using JasonPair        = triagens::basics::JasonPair;
using JasonParser      = triagens::basics::JasonParser;
using JasonSlice       = triagens::basics::JasonSlice;
using JasonType        = triagens::basics::JasonType;
  
static char Buffer[4096];

// This function is used to use the dumper to produce JSON and verify
// the result. When we have parsed previously, we usually can take the
// original input, otherwise we provide a knownGood result.

static void checkDump (JasonSlice s, std::string const& knownGood) {
  JasonBuffer buffer;
  JasonDumper dumper(s, buffer, JasonDumper::STRATEGY_FAIL);
  dumper.dump();
  std::string output(buffer.data(), buffer.size());
  EXPECT_EQ(knownGood, output);
}

// With the following function we check type determination and size
// of the produced Jason value:

static void checkBuild (JasonSlice s, JasonType t, JasonLength byteSize) {
  EXPECT_EQ(t, s.type());
  EXPECT_TRUE(s.isType(t));
  JasonType other = (t == JasonType::String) ? JasonType::Int
                                             : JasonType::String;
  EXPECT_FALSE(s.isType(other));
  EXPECT_FALSE(other == s.type());

  EXPECT_EQ(byteSize, s.byteSize());

  switch (t) {
    case JasonType::None:
      EXPECT_FALSE(s.isNull());
      EXPECT_FALSE(s.isBool());
      EXPECT_FALSE(s.isDouble());
      EXPECT_FALSE(s.isArray());
      EXPECT_FALSE(s.isObject());
      EXPECT_FALSE(s.isExternal());
      EXPECT_FALSE(s.isID());
      EXPECT_FALSE(s.isArangoDB_id());
      EXPECT_FALSE(s.isUTCDate());
      EXPECT_FALSE(s.isInt());
      EXPECT_FALSE(s.isUInt());
      EXPECT_FALSE(s.isString());
      EXPECT_FALSE(s.isBinary());
      EXPECT_FALSE(s.isNumber());
      break;
    case JasonType::Null:
      EXPECT_TRUE(s.isNull());
      EXPECT_FALSE(s.isBool());
      EXPECT_FALSE(s.isDouble());
      EXPECT_FALSE(s.isArray());
      EXPECT_FALSE(s.isObject());
      EXPECT_FALSE(s.isExternal());
      EXPECT_FALSE(s.isID());
      EXPECT_FALSE(s.isArangoDB_id());
      EXPECT_FALSE(s.isUTCDate());
      EXPECT_FALSE(s.isInt());
      EXPECT_FALSE(s.isUInt());
      EXPECT_FALSE(s.isString());
      EXPECT_FALSE(s.isBinary());
      EXPECT_FALSE(s.isNumber());
      break;
    case JasonType::Bool:
      EXPECT_FALSE(s.isNull());
      EXPECT_TRUE(s.isBool());
      EXPECT_FALSE(s.isDouble());
      EXPECT_FALSE(s.isArray());
      EXPECT_FALSE(s.isObject());
      EXPECT_FALSE(s.isExternal());
      EXPECT_FALSE(s.isID());
      EXPECT_FALSE(s.isArangoDB_id());
      EXPECT_FALSE(s.isUTCDate());
      EXPECT_FALSE(s.isInt());
      EXPECT_FALSE(s.isUInt());
      EXPECT_FALSE(s.isString());
      EXPECT_FALSE(s.isBinary());
      EXPECT_FALSE(s.isNumber());
      break;
    case JasonType::Double:
      EXPECT_FALSE(s.isNull());
      EXPECT_FALSE(s.isBool());
      EXPECT_TRUE(s.isDouble());
      EXPECT_FALSE(s.isArray());
      EXPECT_FALSE(s.isObject());
      EXPECT_FALSE(s.isExternal());
      EXPECT_FALSE(s.isID());
      EXPECT_FALSE(s.isArangoDB_id());
      EXPECT_FALSE(s.isUTCDate());
      EXPECT_FALSE(s.isInt());
      EXPECT_FALSE(s.isUInt());
      EXPECT_FALSE(s.isString());
      EXPECT_FALSE(s.isBinary());
      EXPECT_TRUE(s.isNumber());
      break;
    case JasonType::Array:
    case JasonType::ArrayLong:
      EXPECT_FALSE(s.isNull());
      EXPECT_FALSE(s.isBool());
      EXPECT_FALSE(s.isDouble());
      EXPECT_TRUE(s.isArray());
      EXPECT_FALSE(s.isObject());
      EXPECT_FALSE(s.isExternal());
      EXPECT_FALSE(s.isID());
      EXPECT_FALSE(s.isArangoDB_id());
      EXPECT_FALSE(s.isUTCDate());
      EXPECT_FALSE(s.isInt());
      EXPECT_FALSE(s.isUInt());
      EXPECT_FALSE(s.isString());
      EXPECT_FALSE(s.isBinary());
      EXPECT_FALSE(s.isNumber());
      break;
    case JasonType::Object:
    case JasonType::ObjectLong:
      EXPECT_FALSE(s.isNull());
      EXPECT_FALSE(s.isBool());
      EXPECT_FALSE(s.isDouble());
      EXPECT_FALSE(s.isArray());
      EXPECT_TRUE(s.isObject());
      EXPECT_FALSE(s.isExternal());
      EXPECT_FALSE(s.isID());
      EXPECT_FALSE(s.isArangoDB_id());
      EXPECT_FALSE(s.isUTCDate());
      EXPECT_FALSE(s.isInt());
      EXPECT_FALSE(s.isUInt());
      EXPECT_FALSE(s.isString());
      EXPECT_FALSE(s.isBinary());
      EXPECT_FALSE(s.isNumber());
      break;
    case JasonType::External:
      EXPECT_FALSE(s.isNull());
      EXPECT_FALSE(s.isBool());
      EXPECT_FALSE(s.isDouble());
      EXPECT_FALSE(s.isArray());
      EXPECT_FALSE(s.isObject());
      EXPECT_TRUE(s.isExternal());
      EXPECT_FALSE(s.isID());
      EXPECT_FALSE(s.isArangoDB_id());
      EXPECT_FALSE(s.isUTCDate());
      EXPECT_FALSE(s.isInt());
      EXPECT_FALSE(s.isUInt());
      EXPECT_FALSE(s.isString());
      EXPECT_FALSE(s.isBinary());
      EXPECT_FALSE(s.isNumber());
      break;
    case JasonType::ID:
      EXPECT_FALSE(s.isNull());
      EXPECT_FALSE(s.isBool());
      EXPECT_FALSE(s.isDouble());
      EXPECT_FALSE(s.isArray());
      EXPECT_FALSE(s.isObject());
      EXPECT_FALSE(s.isExternal());
      EXPECT_TRUE(s.isID());
      EXPECT_FALSE(s.isArangoDB_id());
      EXPECT_FALSE(s.isUTCDate());
      EXPECT_FALSE(s.isInt());
      EXPECT_FALSE(s.isUInt());
      EXPECT_FALSE(s.isString());
      EXPECT_FALSE(s.isBinary());
      EXPECT_FALSE(s.isNumber());
      break;
    case JasonType::ArangoDB_id:
      EXPECT_FALSE(s.isNull());
      EXPECT_FALSE(s.isBool());
      EXPECT_FALSE(s.isDouble());
      EXPECT_FALSE(s.isArray());
      EXPECT_FALSE(s.isObject());
      EXPECT_FALSE(s.isExternal());
      EXPECT_FALSE(s.isID());
      EXPECT_TRUE(s.isArangoDB_id());
      EXPECT_FALSE(s.isUTCDate());
      EXPECT_FALSE(s.isInt());
      EXPECT_FALSE(s.isUInt());
      EXPECT_FALSE(s.isString());
      EXPECT_FALSE(s.isBinary());
      EXPECT_FALSE(s.isNumber());
      break;
    case JasonType::UTCDate:
      EXPECT_FALSE(s.isNull());
      EXPECT_FALSE(s.isBool());
      EXPECT_FALSE(s.isDouble());
      EXPECT_FALSE(s.isArray());
      EXPECT_FALSE(s.isObject());
      EXPECT_FALSE(s.isExternal());
      EXPECT_FALSE(s.isID());
      EXPECT_FALSE(s.isArangoDB_id());
      EXPECT_TRUE(s.isUTCDate());
      EXPECT_FALSE(s.isInt());
      EXPECT_FALSE(s.isUInt());
      EXPECT_FALSE(s.isString());
      EXPECT_FALSE(s.isBinary());
      EXPECT_FALSE(s.isNumber());
      break;
    case JasonType::Int:
      EXPECT_FALSE(s.isNull());
      EXPECT_FALSE(s.isBool());
      EXPECT_FALSE(s.isDouble());
      EXPECT_FALSE(s.isArray());
      EXPECT_FALSE(s.isObject());
      EXPECT_FALSE(s.isExternal());
      EXPECT_FALSE(s.isID());
      EXPECT_FALSE(s.isArangoDB_id());
      EXPECT_FALSE(s.isUTCDate());
      EXPECT_TRUE(s.isInt());
      EXPECT_FALSE(s.isUInt());
      EXPECT_FALSE(s.isString());
      EXPECT_FALSE(s.isBinary());
      EXPECT_TRUE(s.isNumber());
      break;
    case JasonType::UInt:
      EXPECT_FALSE(s.isNull());
      EXPECT_FALSE(s.isBool());
      EXPECT_FALSE(s.isDouble());
      EXPECT_FALSE(s.isArray());
      EXPECT_FALSE(s.isObject());
      EXPECT_FALSE(s.isExternal());
      EXPECT_FALSE(s.isID());
      EXPECT_FALSE(s.isArangoDB_id());
      EXPECT_FALSE(s.isUTCDate());
      EXPECT_FALSE(s.isInt());
      EXPECT_TRUE(s.isUInt());
      EXPECT_FALSE(s.isString());
      EXPECT_FALSE(s.isBinary());
      EXPECT_TRUE(s.isNumber());
      break;
    case JasonType::String:
    case JasonType::StringLong:
      EXPECT_FALSE(s.isNull());
      EXPECT_FALSE(s.isBool());
      EXPECT_FALSE(s.isDouble());
      EXPECT_FALSE(s.isArray());
      EXPECT_FALSE(s.isObject());
      EXPECT_FALSE(s.isExternal());
      EXPECT_FALSE(s.isID());
      EXPECT_FALSE(s.isArangoDB_id());
      EXPECT_FALSE(s.isUTCDate());
      EXPECT_FALSE(s.isInt());
      EXPECT_FALSE(s.isUInt());
      EXPECT_TRUE(s.isString());
      EXPECT_FALSE(s.isBinary());
      EXPECT_FALSE(s.isNumber());
      break;
    case JasonType::Binary:
      EXPECT_FALSE(s.isNull());
      EXPECT_FALSE(s.isBool());
      EXPECT_FALSE(s.isDouble());
      EXPECT_FALSE(s.isArray());
      EXPECT_FALSE(s.isObject());
      EXPECT_FALSE(s.isExternal());
      EXPECT_FALSE(s.isID());
      EXPECT_FALSE(s.isArangoDB_id());
      EXPECT_FALSE(s.isUTCDate());
      EXPECT_FALSE(s.isInt());
      EXPECT_FALSE(s.isUInt());
      EXPECT_FALSE(s.isString());
      EXPECT_TRUE(s.isBinary());
      EXPECT_FALSE(s.isNumber());
      break;
  }
}


// Let the tests begin...

TEST(SliceTest, Null) {
  Buffer[0] = 0x0;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  EXPECT_EQ(JasonType::Null, slice.type());
  EXPECT_TRUE(slice.isNull());
  EXPECT_EQ(1ULL, slice.byteSize());
}

TEST(SliceTest, False) {
  Buffer[0] = 0x1;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  EXPECT_EQ(JasonType::Bool, slice.type());
  EXPECT_TRUE(slice.isBool());
  EXPECT_EQ(1ULL, slice.byteSize());
  EXPECT_FALSE(slice.getBool());
}

TEST(SliceTest, True) {
  Buffer[0] = 0x2;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  EXPECT_EQ(JasonType::Bool, slice.type());
  EXPECT_TRUE(slice.isBool());
  EXPECT_EQ(1ULL, slice.byteSize());
  EXPECT_TRUE(slice.getBool());
}

TEST(SliceTest, Double) {
  Buffer[0] = 0x3;

  double value = 23.5;
  memcpy(&Buffer[1], (void*) &value, sizeof(value));

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  EXPECT_EQ(JasonType::Double, slice.type());
  EXPECT_TRUE(slice.isDouble());
  EXPECT_EQ(9ULL, slice.byteSize());
  EXPECT_FLOAT_EQ(value, slice.getDouble());
}

TEST(SliceTest, DoubleNegative) {
  Buffer[0] = 0x3;

  double value = -999.91355;
  memcpy(&Buffer[1], (void*) &value, sizeof(value));

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  EXPECT_EQ(JasonType::Double, slice.type());
  EXPECT_TRUE(slice.isDouble());
  EXPECT_EQ(9ULL, slice.byteSize());
  EXPECT_FLOAT_EQ(value, slice.getDouble());
}

TEST(SliceTest, Int1) {
  Buffer[0] = 0x20;
  uint8_t value = 0x33;
  memcpy(&Buffer[1], (void*) &value, sizeof(value));

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  EXPECT_EQ(JasonType::Int, slice.type());
  EXPECT_TRUE(slice.isInt());
  EXPECT_EQ(2ULL, slice.byteSize());

  EXPECT_EQ(value, slice.getInt());
}

TEST(SliceTest, Int2) {
  Buffer[0] = 0x21;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  EXPECT_EQ(JasonType::Int, slice.type());
  EXPECT_TRUE(slice.isInt());
  EXPECT_EQ(3ULL, slice.byteSize());
  EXPECT_EQ(0x23 + 0x100 * 0x42, slice.getInt());
}

TEST(SliceTest, Int3) {
  Buffer[0] = 0x22;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  EXPECT_EQ(JasonType::Int, slice.type());
  EXPECT_TRUE(slice.isInt());
  EXPECT_EQ(4ULL, slice.byteSize());
  EXPECT_EQ(0x23 + 0x100 * 0x42 + 0x10000 * 0x66, slice.getInt());
}

TEST(SliceTest, Int4) {
  Buffer[0] = 0x23;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  EXPECT_EQ(JasonType::Int, slice.type());
  EXPECT_TRUE(slice.isInt());
  EXPECT_EQ(5ULL, slice.byteSize());
  EXPECT_EQ(static_cast<int64_t>(0x23 + 0x100ULL * 0x42ULL + 0x10000ULL * 0x66ULL + 0x1000000ULL * 0xacULL), slice.getInt());
}

TEST(SliceTest, NegInt1) {
  Buffer[0] = 0x28;
  uint8_t value = 0x33;
  memcpy(&Buffer[1], (void*) &value, sizeof(value));

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  EXPECT_EQ(JasonType::Int, slice.type());
  EXPECT_TRUE(slice.isInt());
  EXPECT_EQ(2ULL, slice.byteSize());

  EXPECT_EQ(-value, slice.getInt());
}

TEST(SliceTest, NegInt2) {
  Buffer[0] = 0x29;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  EXPECT_EQ(JasonType::Int, slice.type());
  EXPECT_TRUE(slice.isInt());
  EXPECT_EQ(3ULL, slice.byteSize());
  EXPECT_EQ(- (0x23 + 0x100 * 0x42), slice.getInt());
}

TEST(SliceTest, NegInt3) {
  Buffer[0] = 0x2a;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  EXPECT_EQ(JasonType::Int, slice.type());
  EXPECT_TRUE(slice.isInt());
  EXPECT_EQ(4ULL, slice.byteSize());
  EXPECT_EQ(static_cast<int64_t>(- (0x23 + 0x100 * 0x42 + 0x10000 * 0x66)), slice.getInt());
}

TEST(SliceTest, NegInt4) {
  Buffer[0] = 0x2b;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  EXPECT_EQ(JasonType::Int, slice.type());
  EXPECT_TRUE(slice.isInt());
  EXPECT_EQ(5ULL, slice.byteSize());
  EXPECT_EQ(static_cast<int64_t>(- (0x23 + 0x100LL * 0x42LL + 0x10000LL * 0x66LL + 0x1000000LL * 0xacLL)), slice.getInt());
}

TEST(SliceTest, UInt1) {
  Buffer[0] = 0x30;
  uint8_t value = 0x33;
  memcpy(&Buffer[1], (void*) &value, sizeof(value));

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  EXPECT_EQ(JasonType::UInt, slice.type());
  EXPECT_TRUE(slice.isUInt());
  EXPECT_EQ(2ULL, slice.byteSize());

  EXPECT_EQ(value, slice.getUInt());
}

TEST(SliceTest, UInt2) {
  Buffer[0] = 0x31;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  EXPECT_EQ(JasonType::UInt, slice.type());
  EXPECT_TRUE(slice.isUInt());
  EXPECT_EQ(3ULL, slice.byteSize());
  EXPECT_EQ(0x23ULL + 0x100ULL * 0x42ULL, slice.getUInt());
}

TEST(SliceTest, UInt3) {
  Buffer[0] = 0x32;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  EXPECT_EQ(JasonType::UInt, slice.type());
  EXPECT_TRUE(slice.isUInt());
  EXPECT_EQ(4ULL, slice.byteSize());
  EXPECT_EQ(0x23ULL + 0x100ULL * 0x42ULL + 0x10000ULL * 0x66ULL, slice.getUInt());
}

TEST(SliceTest, UInt4) {
  Buffer[0] = 0x33;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  EXPECT_EQ(JasonType::UInt, slice.type());
  EXPECT_TRUE(slice.isUInt());
  EXPECT_EQ(5ULL, slice.byteSize());
  EXPECT_EQ(0x23ULL + 0x100ULL * 0x42ULL + 0x10000ULL * 0x66ULL + 0x1000000ULL * 0xacULL, slice.getUInt());
}

TEST(SliceTest, ArrayEmpty) {
  Buffer[0] = 0x04;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x00;
  *p++ = 0x04;
  *p++ = 0x00;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  EXPECT_EQ(JasonType::Array, slice.type());
  EXPECT_TRUE(slice.isArray());
  EXPECT_EQ(4ULL, slice.byteSize());
  EXPECT_EQ(0ULL, slice.length());
}

TEST(SliceTest, StringEmpty) {
  Buffer[0] = 0x40;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  EXPECT_EQ(JasonType::String, slice.type());
  EXPECT_TRUE(slice.isString());
  EXPECT_EQ(1ULL, slice.byteSize());
  JasonLength len;
  char const* s = slice.getString(len);
  EXPECT_EQ(0ULL, len);
  EXPECT_EQ(0, strncmp(s, "", len));

  EXPECT_EQ("", slice.copyString());
}

TEST(SliceTest, String1) {
  Buffer[0] = 0x40 + strlen("foobar");

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = (uint8_t) 'f';
  *p++ = (uint8_t) 'o';
  *p++ = (uint8_t) 'o';
  *p++ = (uint8_t) 'b';
  *p++ = (uint8_t) 'a';
  *p++ = (uint8_t) 'r';

  EXPECT_EQ(JasonType::String, slice.type());
  EXPECT_TRUE(slice.isString());
  EXPECT_EQ(7ULL, slice.byteSize());
  JasonLength len;
  char const* s = slice.getString(len);
  EXPECT_EQ(6ULL, len);
  EXPECT_EQ(0, strncmp(s, "foobar", len));

  EXPECT_EQ("foobar", slice.copyString());
}

TEST(SliceTest, String2) {
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

  EXPECT_EQ(JasonType::String, slice.type());
  EXPECT_TRUE(slice.isString());
  EXPECT_EQ(9ULL, slice.byteSize());
  JasonLength len;
  char const* s = slice.getString(len);
  EXPECT_EQ(8ULL, len);
  EXPECT_EQ(0, strncmp(s, "123f\r\t\nx", len));

  EXPECT_EQ("123f\r\t\nx", slice.copyString());
}

TEST(SliceTest, StringNullBytes) {
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

  EXPECT_EQ(JasonType::String, slice.type());
  EXPECT_TRUE(slice.isString());
  EXPECT_EQ(9ULL, slice.byteSize());
  JasonLength len;
  slice.getString(len);
  EXPECT_EQ(8ULL, len);

  std::string s(slice.copyString());
  EXPECT_EQ(8ULL, s.size());
  EXPECT_EQ('\0', s[0]);
  EXPECT_EQ('1', s[1]);
  EXPECT_EQ('2', s[2]);
  EXPECT_EQ('\0', s[3]);
  EXPECT_EQ('3', s[4]);
  EXPECT_EQ('4', s[5]);
  EXPECT_EQ('\0', s[6]);
  EXPECT_EQ('x', s[7]);
}

TEST(SliceTest, StringLong1) {
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

  EXPECT_EQ(JasonType::StringLong, slice.type());
  EXPECT_TRUE(slice.isString());
  EXPECT_EQ(8ULL, slice.byteSize());
  JasonLength len;
  char const* s = slice.getString(len);
  EXPECT_EQ(6ULL, len);
  EXPECT_EQ(0, strncmp(s, "foobar", len));

  EXPECT_EQ("foobar", slice.copyString());
}

TEST(BuilderTest, Null) {
  JasonBuilder b;
  b.add(Jason());
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t const correctResult[] 
    = { 0x00 };

  EXPECT_EQ(sizeof(correctResult), len);
  EXPECT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, False) {
  JasonBuilder b;
  b.add(Jason(false));
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t const correctResult[] 
    = { 0x01 };

  EXPECT_EQ(sizeof(correctResult), len);
  EXPECT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, True) {
  JasonBuilder b;
  b.add(Jason(true));
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t const correctResult[] 
    = { 0x02 };

  EXPECT_EQ(sizeof(correctResult), len);
  EXPECT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, Double) {
  static double value = 123.456;
  JasonBuilder b;
  b.add(Jason(value));
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t correctResult[9] 
    = { 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  EXPECT_EQ(8ULL, sizeof(double));
  memcpy(correctResult + 1, &value, sizeof(value));

  EXPECT_EQ(sizeof(correctResult), len);
  EXPECT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, String) {
  JasonBuilder b;
  b.add(Jason("abcdefghijklmnopqrstuvwxyz"));
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t correctResult[] 
    = { 0x5a, 
        0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b,
        0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
        0x77, 0x78, 0x79, 0x7a };

  EXPECT_EQ(sizeof(correctResult), len);
  EXPECT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, ArrayEmpty) {
  JasonBuilder b;
  b.add(Jason(0, JasonType::Array));
  b.close();
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t correctResult[] 
    = { 0x04, 0x00, 0x04, 0x00 };

  EXPECT_EQ(sizeof(correctResult), len);
  EXPECT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, Array4) {
  double value = 2.3;
  JasonBuilder b;
  b.add(Jason(4, JasonType::Array));
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

  EXPECT_EQ(sizeof(correctResult), len);
  EXPECT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, ObjectEmpty) {
  JasonBuilder b;
  b.add(Jason(0, JasonType::Object));
  b.close();
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t correctResult[] 
    = { 0x06, 0x00, 0x04, 0x00 };

  EXPECT_EQ(sizeof(correctResult), len);
  EXPECT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, Object4) {
  double value = 2.3;
  JasonBuilder b;
  b.add(Jason(4, JasonType::Object));
  b.add("a", Jason(uint64_t(1200)));
  b.add("b", Jason(value));
  b.add("c", Jason("abc"));
  b.add("d", Jason(true));
  b.close();

  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t correctResult[] 
    = { 0x06, 0x04, 0x25, 0x00,
        0x0c, 0x00, 0x11, 0x00, 0x1c, 0x00, 0x22, 0x00,
        0x41, 0x61, 0x31, 0xb0, 0x04,   // "a": uint(1200) = 0x4b0
        0x41, 0x62, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   
                                        // "b": double(2.3)
        0x41, 0x63, 0x43, 0x61, 0x62, 0x63,  // "c": "abc"
        0x41, 0x64, 0x02 };
  memcpy(correctResult + 20, &value, 8);

  EXPECT_EQ(sizeof(correctResult), len);
  EXPECT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, External) {
  uint8_t externalStuff[] = { 0x01 };
  JasonBuilder b;
  b.add(Jason(const_cast<void const*>(static_cast<void*>(externalStuff)), 
              JasonType::External));
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t correctResult[1+sizeof(char*)] 
    = { 0x00 };
  correctResult[0] = 0x08;
  uint8_t* p = externalStuff;
  memcpy(correctResult + 1, &p, sizeof(uint8_t*));

  EXPECT_EQ(sizeof(correctResult), len);
  EXPECT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, UInt) {
  uint64_t value = 0x12345678abcdef;
  JasonBuilder b;
  b.add(Jason(value));
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t correctResult[]
    = { 0x36, 0xef, 0xcd, 0xab, 0x78, 0x56, 0x34, 0x12 };

  EXPECT_EQ(sizeof(correctResult), len);
  EXPECT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, IntPos) {
  int64_t value = 0x12345678abcdef;
  JasonBuilder b;
  b.add(Jason(value));
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t correctResult[]
    = { 0x26, 0xef, 0xcd, 0xab, 0x78, 0x56, 0x34, 0x12 };

  EXPECT_EQ(sizeof(correctResult), len);
  EXPECT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, IntNeg) {
  int64_t value = -0x12345678abcdef;
  JasonBuilder b;
  b.add(Jason(value));
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t correctResult[]
    = { 0x2e, 0xef, 0xcd, 0xab, 0x78, 0x56, 0x34, 0x12 };

  EXPECT_EQ(sizeof(correctResult), len);
  EXPECT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, StringChar) {
  char const* value = "der fuxx ging in den wald und aß pilze";
  size_t const valueLen = strlen(value);
  JasonBuilder b;
  b.add(Jason(value));

  JasonSlice slice = JasonSlice(b.start());
  EXPECT_TRUE(slice.isString());
 
  JasonLength len;
  char const* s = slice.getString(len);
  EXPECT_EQ(valueLen, len);
  EXPECT_EQ(0, strncmp(s, value, valueLen));

  std::string c = slice.copyString();
  EXPECT_EQ(valueLen, c.size());
  EXPECT_EQ(0, strncmp(value, c.c_str(), valueLen));
}

TEST(BuilderTest, StringString) {
  std::string const value("der fuxx ging in den wald und aß pilze");
  JasonBuilder b;
  b.add(Jason(value));

  JasonSlice slice = JasonSlice(b.start());
  EXPECT_TRUE(slice.isString());
 
  JasonLength len;
  char const* s = slice.getString(len);
  EXPECT_EQ(value.size(), len);
  EXPECT_EQ(0, strncmp(s, value.c_str(), value.size()));

  std::string c = slice.copyString();
  EXPECT_EQ(value.size(), c.size());
  EXPECT_EQ(value, c);
}

TEST(BuilderTest, Binary) {
  uint8_t binaryStuff[] = { 0x02, 0x03, 0x05, 0x08, 0x0d };

  JasonBuilder b;
  b.add(JasonPair(binaryStuff, sizeof(binaryStuff)));
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t correctResult[]
    = { 0xd0, 0x05, 0x02, 0x03, 0x05, 0x08, 0x0d };

  EXPECT_EQ(sizeof(correctResult), len);
  EXPECT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, ID) {
  char const* key = "\x02\x03\x05\x08\x0d";

  JasonBuilder b;
  b.add(JasonPair(key, 0x12345678, JasonType::ID));
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t const correctResult[]
    = { 0x09, 0x33, 0x78, 0x56, 0x34, 0x12,
        0x45, 0x02, 0x03, 0x05, 0x08, 0x0d };

  EXPECT_EQ(sizeof(correctResult), len);
  EXPECT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, ArangoDB_id) {
  JasonBuilder b;
  b.add(Jason(JasonType::ArangoDB_id));

  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t correctResult[] = { 0x0a };

  EXPECT_EQ(sizeof(correctResult), len);
  EXPECT_EQ(0, memcmp(result, correctResult, len));
}

TEST(ParserTest, Garbage1) {
  std::string const value("z");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(0u, parser.errorPos());
}

TEST(ParserTest, Garbage2) {
  std::string const value("foo");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(1u, parser.errorPos());
}

TEST(ParserTest, Garbage3) {
  std::string const value("truth");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(3u, parser.errorPos());
}

TEST(ParserTest, Garbage4) {
  std::string const value("tru");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(2u, parser.errorPos());
}

TEST(ParserTest, Garbage5) {
  std::string const value("truebar");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(4u, parser.errorPos());
}

TEST(ParserTest, Garbage6) {
  std::string const value("fals");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(3u, parser.errorPos());
}

TEST(ParserTest, Garbage7) {
  std::string const value("falselaber");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(5u, parser.errorPos());
}

TEST(ParserTest, Garbage8) {
  std::string const value("zauberzauber");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(0u, parser.errorPos());
}

TEST(ParserTest, Punctuation1) {
  std::string const value(",");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(0u, parser.errorPos());
}

TEST(ParserTest, Punctuation2) {
  std::string const value("/");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(0u, parser.errorPos());
}

TEST(ParserTest, Punctuation3) {
  std::string const value("@");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(0u, parser.errorPos());
}

TEST(ParserTest, Punctuation4) {
  std::string const value(":");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(0u, parser.errorPos());
}

TEST(ParserTest, Punctuation5) {
  std::string const value("!");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(0u, parser.errorPos());
}

TEST(ParserTest, Null) {
  std::string const value("null");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Null, 1ULL);

  checkDump(s, value);
}

TEST(ParserTest, False) {
  std::string const value("false");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Bool, 1ULL);
  EXPECT_FALSE(s.getBool());

  checkDump(s, value);
}

TEST(ParserTest, True) {
  std::string const value("true");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Bool, 1ULL);
  EXPECT_TRUE(s.getBool());

  checkDump(s, value);
}

TEST(ParserTest, Zero) {
  std::string const value("0");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::UInt, 2ULL);
  EXPECT_EQ(0ULL, s.getUInt());

  checkDump(s, value);
}

TEST(ParserTest, ZeroInvalid) {
  std::string const value("00");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(1u, parser.errorPos());
}

TEST(ParserTest, NumberIncomplete) {
  std::string const value("-");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(0u, parser.errorPos());
}

TEST(ParserTest, Int1) {
  std::string const value("1");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::UInt, 2ULL);
  EXPECT_EQ(1ULL, s.getUInt());

  checkDump(s, value);
}

TEST(ParserTest, IntM1) {
  std::string const value("-1");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Int, 2ULL);
  EXPECT_EQ(-1LL, s.getInt());

  checkDump(s, value);
}

TEST(ParserTest, Int2) {
  std::string const value("100000");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::UInt, 4ULL);
  EXPECT_EQ(100000ULL, s.getUInt());

  checkDump(s, value);
}

TEST(ParserTest, Int3) {
  std::string const value("-100000");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Int, 4ULL);
  EXPECT_EQ(-100000LL, s.getInt());

  checkDump(s, value);
}

TEST(ParserTest, Double1) {
  std::string const value("1.0124");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Double, 9ULL);
  EXPECT_EQ(1.0124, s.getDouble());

  checkDump(s, value);
}

TEST(ParserTest, Double2) {
  std::string const value("-1.0124");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Double, 9ULL);
  EXPECT_EQ(-1.0124, s.getDouble());

  checkDump(s, value);
}

TEST(ParserTest, DoubleScientific1) {
  std::string const value("-1.0124e42");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Double, 9ULL);
  EXPECT_EQ(-1.0124e42, s.getDouble());

  std::string const valueOut("-1.0124e+42");
  checkDump(s, valueOut);
}

TEST(ParserTest, DoubleScientific2) {
  std::string const value("-1.0124e+42");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Double, 9ULL);
  EXPECT_EQ(-1.0124e42, s.getDouble());

  checkDump(s, value);
}

TEST(ParserTest, DoubleScientific3) {
  std::string const value("3122243.0124e-42");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Double, 9ULL);
  EXPECT_EQ(3122243.0124e-42, s.getDouble());

  std::string const valueOut("3.1222430124e-36");
  checkDump(s, valueOut);
}

TEST(ParserTest, DoubleScientific4) {
  std::string const value("2335431.0124E-42");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Double, 9ULL);
  EXPECT_EQ(2335431.0124E-42, s.getDouble());

  std::string const valueOut("2.3354310124e-36");
  checkDump(s, valueOut);
}

TEST(ParserTest, Empty) {
  std::string const value("");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(0u, parser.errorPos());
}

TEST(ParserTest, WhitespaceOnly) {
  std::string const value("  ");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(1u, parser.errorPos());
}

TEST(ParserTest, UnterminatedStringLiteral) {
  std::string const value("\"der hund");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(8u, parser.errorPos());
}

TEST(ParserTest, StringLiteral) {
  std::string const value("\"der hund ging in den wald und aß den fuxx\"");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  std::string const correct = "der hund ging in den wald und aß den fuxx";
  checkBuild(s, JasonType::String, 1 + correct.size());
  char const* p = s.getString(len);
  EXPECT_EQ(correct.size(), len);
  EXPECT_EQ(0, strncmp(correct.c_str(), p, len));
  std::string out = s.copyString();
  EXPECT_EQ(correct, out);

  std::string valueOut = "\"der hund ging in den wald und aß den fuxx\"";
  checkDump(s, valueOut);
}

TEST(ParserTest, StringLiteralEmpty) {
  std::string const value("\"\"");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::String, 1ULL);
  char const* p = s.getString(len);
  EXPECT_EQ(0, strncmp("", p, len));
  EXPECT_EQ(0ULL, len);
  std::string out = s.copyString();
  std::string empty;
  EXPECT_EQ(empty, out);

  checkDump(s, value);
}

TEST(ParserTest, StringLiteralInvalidUtfValue1) {
  std::string value;
  value.push_back('"');
  value.push_back(static_cast<unsigned char>(0x80));
  value.push_back('"');

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(1u, parser.errorPos());
}

TEST(ParserTest, StringLiteralInvalidUtfValue2) {
  std::string value;
  value.push_back('"');
  value.push_back(static_cast<unsigned char>(0xff));
  value.push_back(static_cast<unsigned char>(0xff));
  value.push_back('"');

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(1u, parser.errorPos());
}

TEST(ParserTest, StringLiteralInvalidUtfValue3) {
  for (char c = 0; c < 0x20; c++) {
    std::string value;
    value.push_back('"');
    value.push_back(c);
    value.push_back('"');

    JasonParser parser;
    EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
    EXPECT_EQ(1u, parser.errorPos());
  }
}

TEST(ParserTest, StringLiteralUnfinishedUtfSequence1) {
  std::string const value("\"\\u\"");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(3u, parser.errorPos());
}

TEST(ParserTest, StringLiteralUnfinishedUtfSequence2) {
  std::string const value("\"\\u0\"");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(4u, parser.errorPos());
}

TEST(ParserTest, StringLiteralUnfinishedUtfSequence3) {
  std::string const value("\"\\u01\"");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(5u, parser.errorPos());
}

TEST(ParserTest, StringLiteralUnfinishedUtfSequence4) {
  std::string const value("\"\\u012\"");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(6u, parser.errorPos());
}

TEST(ParserTest, StringLiteralUtf8SequenceLowerCase) {
  std::string const value("\"der m\\u00d6ter\"");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::String, 11ULL);
  char const* p = s.getString(len);
  EXPECT_EQ(10ULL, len);
  std::string correct = "der m\xc3\x96ter";
  EXPECT_EQ(0, strncmp(correct.c_str(), p, len));
  std::string out = s.copyString();
  EXPECT_EQ(correct, out);

  std::string const valueOut("\"der mÖter\"");
  checkDump(s, valueOut);
}

TEST(ParserTest, StringLiteralUtf8SequenceUpperCase) {
  std::string const value("\"der m\\u00D6ter\"");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  std::string correct = "der mÖter";
  checkBuild(s, JasonType::String, 1 + correct.size());
  char const* p = s.getString(len);
  EXPECT_EQ(correct.size(), len);
  EXPECT_EQ(0, strncmp(correct.c_str(), p, len));
  std::string out = s.copyString();
  EXPECT_EQ(correct, out);

  checkDump(s, std::string("\"der mÖter\""));
}

TEST(ParserTest, StringLiteralUtf8Chars) {
  std::string const value("\"der mötör klötörte mät dän fößen\"");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  std::string correct = "der mötör klötörte mät dän fößen";
  checkBuild(s, JasonType::String, 1 + correct.size());
  char const* p = s.getString(len);
  EXPECT_EQ(correct.size(), len);
  EXPECT_EQ(0, strncmp(correct.c_str(), p, len));
  std::string out = s.copyString();
  EXPECT_EQ(correct, out);

//  std::string const valueOut("\"der mötör kö\\u00F6t\\u00F6r kl\\u00F6t\\u00F6rte m\\u00E4t d\\u00E4n f\\u00F6\\u00DFen\"");
  checkDump(s, value);
}

TEST(ParserTest, StringLiteralWithSpecials) {
  std::string const value("  \"der\\thund\\nging\\rin\\fden\\\\wald\\\"und\\b\\nden'fux\"  ");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  std::string correct = "der\thund\nging\rin\fden\\wald\"und\b\nden'fux";
  checkBuild(s, JasonType::String, 1 + correct.size());
  char const* p = s.getString(len);
  EXPECT_EQ(correct.size(), len);
  EXPECT_EQ(0, strncmp(correct.c_str(), p, len));
  std::string out = s.copyString();
  EXPECT_EQ(correct, out);

  std::string const valueOut("\"der\\thund\\nging\\rin\\fden\\\\wald\\\"und\\b\\nden'fux\"");
  checkDump(s, valueOut);
}

TEST(ParserTest, EmptyArray) {
  std::string const value("[]");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Array, 4);
  EXPECT_EQ(0ULL, s.length());

  checkDump(s, value);
}

TEST(ParserTest, WhitespacedArray) {
  std::string const value("  [    ]   ");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Array, 4);
  EXPECT_EQ(0ULL, s.length());

  std::string const valueOut = "[]";
  checkDump(s, valueOut);
}

TEST(ParserTest, Array1) {
  std::string const value("[1]");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Array, 6);
  EXPECT_EQ(1ULL, s.length());
  JasonSlice ss = s[0];
  checkBuild(ss, JasonType::UInt, 2);
  EXPECT_EQ(1ULL, ss.getUInt());

  checkDump(s, value);
}

TEST(ParserTest, Array2) {
  std::string const value("[1,2]");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Array, 10);
  EXPECT_EQ(2ULL, s.length());
  JasonSlice ss = s[0];
  checkBuild(ss, JasonType::UInt, 2);
  EXPECT_EQ(1ULL, ss.getUInt());
  ss = s[1];
  checkBuild(ss, JasonType::UInt, 2);
  EXPECT_EQ(2ULL, ss.getUInt());

  checkDump(s, value);
}

TEST(ParserTest, Array3) {
  std::string const value("[-1,2, 4.5, 3, -99.99]");
  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Array, 36);
  EXPECT_EQ(5ULL, s.length());

  JasonSlice ss = s[0];
  checkBuild(ss, JasonType::Int, 2);
  EXPECT_EQ(-1LL, ss.getInt());

  ss = s[1];
  checkBuild(ss, JasonType::UInt, 2);
  EXPECT_EQ(2ULL, ss.getUInt());

  ss = s[2];
  checkBuild(ss, JasonType::Double, 9);
  EXPECT_EQ(4.5, ss.getDouble());

  ss = s[3];
  checkBuild(ss, JasonType::UInt, 2);
  EXPECT_EQ(3ULL, ss.getUInt());

  ss = s[4];
  checkBuild(ss, JasonType::Double, 9);
  EXPECT_EQ(-99.99, ss.getDouble());

  std::string const valueOut = "[-1,2,4.5,3,-99.99]";
  checkDump(s, valueOut);
}

TEST(ParserTest, Array4) {
  std::string const value("[\"foo\", \"bar\", \"baz\", null, true, false, -42.23 ]");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Array, 40);
  EXPECT_EQ(7ULL, s.length());

  JasonSlice ss = s[0];
  checkBuild(ss, JasonType::String, 4);
  std::string correct = "foo";
  EXPECT_EQ(correct, ss.copyString());

  ss = s[1];
  checkBuild(ss, JasonType::String, 4);
  correct = "bar";
  EXPECT_EQ(correct, ss.copyString());

  ss = s[2];
  checkBuild(ss, JasonType::String, 4);
  correct = "baz";
  EXPECT_EQ(correct, ss.copyString());

  ss = s[3];
  checkBuild(ss, JasonType::Null, 1);

  ss = s[4];
  checkBuild(ss, JasonType::Bool, 1);
  EXPECT_TRUE(ss.getBool());

  ss = s[5];
  checkBuild(ss, JasonType::Bool, 1);
  EXPECT_FALSE(ss.getBool());

  ss = s[6];
  checkBuild(ss, JasonType::Double, 9);
  EXPECT_EQ(-42.23, ss.getDouble());

  std::string const valueOut = "[\"foo\",\"bar\",\"baz\",null,true,false,-42.23]";
  checkDump(s, valueOut);
}

TEST(ParserTest, NestedArray1) {
  std::string const value("[ [ ] ]");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Array, 8);
  EXPECT_EQ(1ULL, s.length());

  JasonSlice ss = s[0];
  checkBuild(ss, JasonType::Array, 4);
  EXPECT_EQ(0ULL, ss.length());

  std::string const valueOut = "[[]]";
  checkDump(s, valueOut);
}

TEST(ParserTest, NestedArray2) {
  std::string const value("[ [ ],[[]],[],[ [[ [], [ ], [ ] ], [ ] ] ], [] ]");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Array, 66);
  EXPECT_EQ(5ULL, s.length());

  JasonSlice ss = s[0];
  checkBuild(ss, JasonType::Array, 4);
  EXPECT_EQ(0ULL, ss.length());

  ss = s[1];
  checkBuild(ss, JasonType::Array, 8);
  EXPECT_EQ(1ULL, ss.length());

  JasonSlice sss = ss[0];
  checkBuild(sss, JasonType::Array, 4);
  EXPECT_EQ(0ULL, sss.length());

  ss = s[2];
  checkBuild(ss, JasonType::Array, 4);
  EXPECT_EQ(0ULL, ss.length());

  ss = s[3];
  checkBuild(ss, JasonType::Array, 34);
  EXPECT_EQ(1ULL, ss.length());

  sss = ss[0];
  checkBuild(sss, JasonType::Array, 30);
  EXPECT_EQ(2ULL, sss.length());

  JasonSlice ssss = sss[0];
  checkBuild(ssss, JasonType::Array, 20);
  EXPECT_EQ(3ULL, ssss.length());

  JasonSlice sssss = ssss[0];
  checkBuild(sssss, JasonType::Array, 4);
  EXPECT_EQ(0ULL, sssss.length());

  sssss = ssss[1];
  checkBuild(sssss, JasonType::Array, 4);
  EXPECT_EQ(0ULL, sssss.length());

  sssss = ssss[2];
  checkBuild(sssss, JasonType::Array, 4);
  EXPECT_EQ(0ULL, sssss.length());

  ssss = sss[1];
  checkBuild(ssss, JasonType::Array, 4);
  EXPECT_EQ(0ULL, ssss.length());

  ss = s[4];
  checkBuild(ss, JasonType::Array, 4);
  EXPECT_EQ(0ULL, ss.length());

  std::string const valueOut = "[[],[[]],[],[[[[],[],[]],[]]],[]]";
  checkDump(s, valueOut);
}

TEST(ParserTest, NestedArray3) {
  std::string const value("[ [ \"foo\", [ \"bar\", \"baz\", null ], true, false ], -42.23 ]");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Array, 48);
  EXPECT_EQ(2ULL, s.length());

  JasonSlice ss = s[0];
  checkBuild(ss, JasonType::Array, 33);
  EXPECT_EQ(4ULL, ss.length());

  JasonSlice sss = ss[0];
  checkBuild(sss, JasonType::String, 4);
  std::string correct = "foo";
  EXPECT_EQ(correct, sss.copyString());

  sss = ss[1];
  checkBuild(sss, JasonType::Array, 17);
  EXPECT_EQ(3ULL, sss.length());

  JasonSlice ssss = sss[0];
  checkBuild(ssss, JasonType::String, 4);
  correct = "bar";
  EXPECT_EQ(correct, ssss.copyString());

  ssss = sss[1];
  checkBuild(ssss, JasonType::String, 4);
  correct = "baz";
  EXPECT_EQ(correct, ssss.copyString());

  ssss = sss[2];
  checkBuild(ssss, JasonType::Null, 1);

  sss = ss[2];
  checkBuild(sss, JasonType::Bool, 1);
  EXPECT_TRUE(sss.getBool());

  sss = ss[3];
  checkBuild(sss, JasonType::Bool, 1);
  EXPECT_FALSE(sss.getBool());

  ss = s[1];
  checkBuild(ss, JasonType::Double, 9);
  EXPECT_EQ(-42.23, ss.getDouble());

  std::string const valueOut = "[[\"foo\",[\"bar\",\"baz\",null],true,false],-42.23]";
  checkDump(s, valueOut);
}

TEST(ParserTest, NestedArrayInvalid1) {
  std::string const value("[ [ ]");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(4u, parser.errorPos());
}

TEST(ParserTest, NestedArrayInvalid2) {
  std::string const value("[ ] ]");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(4u, parser.errorPos());
}

TEST(ParserTest, NestedArrayInvalid3) {
  std::string const value("[ [ \"foo\", [ \"bar\", \"baz\", null ] ]");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(34u, parser.errorPos());
}

TEST(ParserTest, BrokenArray1) {
  std::string const value("[");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(0u, parser.errorPos());
}

TEST(ParserTest, BrokenArray2) {
  std::string const value("[,");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(1u, parser.errorPos());
}

TEST(ParserTest, BrokenArray3) {
  std::string const value("[1,");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(2u, parser.errorPos());
}

TEST(ParserTest, EmptyObject) {
  std::string const value("{}");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Object, 4);
  EXPECT_EQ(0ULL, s.length());

  checkDump(s, value);
}

TEST(ParserTest, BrokenObject1) {
  std::string const value("{");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(0u, parser.errorPos());
}

TEST(ParserTest, BrokenObject2) {
  std::string const value("{,");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(0u, parser.errorPos());
}

TEST(ParserTest, BrokenObject3) {
  std::string const value("{1,");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(0u, parser.errorPos());
}

TEST(ParserTest, BrokenObject4) {
  std::string const value("{\"foo");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(4u, parser.errorPos());
}

TEST(ParserTest, BrokenObject5) {
  std::string const value("{\"foo\"");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(5u, parser.errorPos());
}

TEST(ParserTest, BrokenObject6) {
  std::string const value("{\"foo\":");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(6u, parser.errorPos());
}

TEST(ParserTest, BrokenObject7) {
  std::string const value("{\"foo\":\"foo");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(10u, parser.errorPos());
}

TEST(ParserTest, BrokenObject8) {
  std::string const value("{\"foo\":\"foo\", ");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(13u, parser.errorPos());
}

TEST(ParserTest, BrokenObject9) {
  std::string const value("{\"foo\":\"foo\", }");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(13u, parser.errorPos());
}

TEST(ParserTest, BrokenObject10) {
  std::string const value("{\"foo\" }");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
  EXPECT_EQ(7u, parser.errorPos());
}

TEST(ParserTest, ObjectSimple1) {
  std::string const value("{ \"foo\" : 1}");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Object, 12);
  EXPECT_EQ(1ULL, s.length());

  JasonSlice ss = s.keyAt(0);
  checkBuild(ss, JasonType::String, 4);
  std::string correct = "foo";
  EXPECT_EQ(correct, ss.copyString());
  ss = s.valueAt(0);
  checkBuild(ss, JasonType::UInt, 2);
  EXPECT_EQ(1ULL, ss.getUInt());

  std::string valueOut = "{\"foo\":1}";
  checkDump(s, valueOut);
}

TEST(ParserTest, ObjectSimple2) {
  std::string const value("{ \"foo\" : \"bar\", \"baz\":true}");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Object, 21);
  EXPECT_EQ(2ULL, s.length());

  JasonSlice ss = s.keyAt(0);
  checkBuild(ss, JasonType::String, 4);
  std::string correct = "baz";
  EXPECT_EQ(correct, ss.copyString());
  ss = s.valueAt(0);
  checkBuild(ss, JasonType::Bool, 1);
  EXPECT_TRUE(ss.getBool());

  ss = s.keyAt(1);
  checkBuild(ss, JasonType::String, 4);
  correct = "foo";
  EXPECT_EQ(correct, ss.copyString());
  ss = s.valueAt(1);
  checkBuild(ss, JasonType::String, 4);
  correct = "bar";
  EXPECT_EQ(correct, ss.copyString());

  std::string valueOut = "{\"baz\":true,\"foo\":\"bar\"}";
  checkDump(s, valueOut);
}

TEST(ParserTest, ObjectDenseNotation) {
  std::string const value("{\"a\":\"b\",\"c\":\"d\"}");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Object, 16);
  EXPECT_EQ(2ULL, s.length());

  JasonSlice ss = s.keyAt(0);
  checkBuild(ss, JasonType::String, 2);
  std::string correct = "a";
  EXPECT_EQ(correct, ss.copyString());
  ss = s.valueAt(0);
  checkBuild(ss, JasonType::String, 2);
  correct = "b";
  EXPECT_EQ(correct, ss.copyString());

  ss = s.keyAt(1);
  checkBuild(ss, JasonType::String, 2);
  correct = "c";
  EXPECT_EQ(correct, ss.copyString());
  ss = s.valueAt(1);
  checkBuild(ss, JasonType::String, 2);
  correct = "d";
  EXPECT_EQ(correct, ss.copyString());

  checkDump(s, value);
}

TEST(ParserTest, ObjectReservedKeys) {
  std::string const value("{ \"null\" : \"true\", \"false\":\"bar\", \"true\":\"foo\"}");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Object, 39);
  EXPECT_EQ(3ULL, s.length());

  JasonSlice ss = s.keyAt(0);
  checkBuild(ss, JasonType::String, 6);
  std::string correct = "false";
  EXPECT_EQ(correct, ss.copyString());
  ss = s.valueAt(0);
  checkBuild(ss, JasonType::String, 4);
  correct = "bar";
  EXPECT_EQ(correct, ss.copyString());

  ss = s.keyAt(1);
  checkBuild(ss, JasonType::String, 5);
  correct = "null";
  EXPECT_EQ(correct, ss.copyString());
  ss = s.valueAt(1);
  checkBuild(ss, JasonType::String, 5);
  correct = "true";
  EXPECT_EQ(correct, ss.copyString());

  ss = s.keyAt(2);
  checkBuild(ss, JasonType::String, 5);
  correct = "true";
  EXPECT_EQ(correct, ss.copyString());
  ss = s.valueAt(2);
  checkBuild(ss, JasonType::String, 4);
  correct = "foo";
  EXPECT_EQ(correct, ss.copyString());

  std::string const valueOut = "{\"false\":\"bar\",\"null\":\"true\",\"true\":\"foo\"}";
  checkDump(s, valueOut);
}

TEST(ParserTest, ObjectMixed) {
  std::string const value("{\"foo\":null,\"bar\":true,\"baz\":13.53,\"qux\":[1],\"quz\":{}}");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Object, 55);
  EXPECT_EQ(5ULL, s.length());

  JasonSlice ss = s.keyAt(0);
  checkBuild(ss, JasonType::String, 4);
  std::string correct = "bar";
  EXPECT_EQ(correct, ss.copyString());
  ss = s.valueAt(0);
  checkBuild(ss, JasonType::Bool, 1);
  EXPECT_TRUE(ss.getBool());

  ss = s.keyAt(1);
  checkBuild(ss, JasonType::String, 4);
  correct = "baz";
  EXPECT_EQ(correct, ss.copyString());
  ss = s.valueAt(1);
  checkBuild(ss, JasonType::Double, 9);
  EXPECT_EQ(13.53, ss.getDouble());

  ss = s.keyAt(2);
  checkBuild(ss, JasonType::String, 4);
  correct = "foo";
  EXPECT_EQ(correct, ss.copyString());
  ss = s.valueAt(2);
  checkBuild(ss, JasonType::Null, 1);

  ss = s.keyAt(3);
  checkBuild(ss, JasonType::String, 4);
  correct = "qux";
  EXPECT_EQ(correct, ss.copyString());
  ss = s.valueAt(3);
  checkBuild(ss, JasonType::Array, 6);

  JasonSlice sss = ss[0];
  checkBuild(sss, JasonType::UInt, 2);
  EXPECT_EQ(1ULL, sss.getUInt());

  ss = s.keyAt(4);
  checkBuild(ss, JasonType::String, 4);
  correct = "quz";
  EXPECT_EQ(correct, ss.copyString());
  ss = s.valueAt(4);
  checkBuild(ss, JasonType::Object, 4);
  EXPECT_EQ(0ULL, ss.length());

  std::string const valueOut("{\"bar\":true,\"baz\":13.53,\"foo\":null,\"qux\":[1],\"quz\":{}}");
  checkDump(s, valueOut);
}

TEST(ParserTest, ObjectInvalidQuotes) {
  std::string const value("{'foo':'bar' }");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
}

TEST(ParserTest, ObjectMissingQuotes) {
  std::string const value("{foo:\"bar\" }");

  JasonParser parser;
  EXPECT_THROW(parser.parse(value), JasonParser::JasonParserError);
}

TEST(LookupTest, LookupShortObject) {
  std::string const value("{\"foo\":null,\"bar\":true,\"baz\":13.53,\"qux\":[1],\"quz\":{}}");

  JasonParser parser;
  parser.parse(value);
  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());

  JasonSlice v;
  v = s.get("foo"); 
  EXPECT_TRUE(v.isNull());
 
  v = s.get("bar");  
  EXPECT_TRUE(v.isBool());
  EXPECT_EQ(true, v.getBool());

  v = s.get("baz");  
  EXPECT_TRUE(v.isDouble());
  EXPECT_FLOAT_EQ(13.53, v.getDouble());

  v = s.get("qux");  
  EXPECT_TRUE(v.isArray());
  EXPECT_TRUE(v.isType(JasonType::Array));
  EXPECT_EQ(1ULL, v.length());

  v = s.get("quz");  
  EXPECT_TRUE(v.isObject());
  EXPECT_TRUE(v.isType(JasonType::Object));
  EXPECT_EQ(0ULL, v.length());

  // non-present attributes
  v = s.get("nada");  
  EXPECT_TRUE(v.isNone());

  v = s.get(std::string("foo\0", 4));  
  EXPECT_TRUE(v.isNone());

  v = s.get("Foo");  
  EXPECT_TRUE(v.isNone());

  v = s.get("food");  
  EXPECT_TRUE(v.isNone());

  v = s.get("");  
  EXPECT_TRUE(v.isNone());
}

TEST(LookupTest, LookupSubattributes) {
  std::string const value("{\"foo\":{\"bar\":1,\"bark\":[],\"baz\":{\"qux\":{\"qurz\":null}}}}");

  JasonParser parser;
  parser.parse(value);
  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());

  JasonSlice v;
  v = s.get(std::vector<std::string>({ "foo" })); 
  EXPECT_TRUE(v.isObject());
 
  v = s.get(std::vector<std::string>({ "foo", "bar" })); 
  EXPECT_TRUE(v.isNumber());
  EXPECT_EQ(1ULL, v.getUInt());

  v = s.get(std::vector<std::string>({ "boo" })); 
  EXPECT_TRUE(v.isNone());

  v = s.get(std::vector<std::string>({ "boo", "far" })); 
  EXPECT_TRUE(v.isNone());

  v = s.get(std::vector<std::string>({ "foo", "bark" })); 
  EXPECT_TRUE(v.isArray());

  v = s.get(std::vector<std::string>({ "foo", "bark", "baz" })); 
  EXPECT_TRUE(v.isNone());

  v = s.get(std::vector<std::string>({ "foo", "baz" })); 
  EXPECT_TRUE(v.isObject());

  v = s.get(std::vector<std::string>({ "foo", "baz", "qux" })); 
  EXPECT_TRUE(v.isObject());

  v = s.get(std::vector<std::string>({ "foo", "baz", "qux", "qurz" })); 
  EXPECT_TRUE(v.isNull());

  v = s.get(std::vector<std::string>({ "foo", "baz", "qux", "qurk" })); 
  EXPECT_TRUE(v.isNone());

  v = s.get(std::vector<std::string>({ "foo", "baz", "qux", "qurz", "p0rk" })); 
  EXPECT_TRUE(v.isNone());
}

TEST(LookupTest, LookupLongObject) {
  std::string value("{");
  for (size_t i = 4; i < 1024; ++i) {
    if (i > 4) {
      value.append(","); 
    }
    value.append("\"test"); 
    value.append(std::to_string(i));
    value.append("\":"); 
    value.append(std::to_string(i));
  }
  value.append("}"); 

  JasonParser parser;
  parser.parse(value);
  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());

  JasonSlice v;
  v = s.get("test4"); 
  EXPECT_TRUE(v.isNumber());
  EXPECT_EQ(4ULL, v.getUInt());

  v = s.get("test10"); 
  EXPECT_TRUE(v.isNumber());
  EXPECT_EQ(10ULL, v.getUInt());

  v = s.get("test42"); 
  EXPECT_TRUE(v.isNumber());
  EXPECT_EQ(42ULL, v.getUInt());

  v = s.get("test100"); 
  EXPECT_TRUE(v.isNumber());
  EXPECT_EQ(100ULL, v.getUInt());
  
  v = s.get("test932"); 
  EXPECT_TRUE(v.isNumber());
  EXPECT_EQ(932ULL, v.getUInt());

  v = s.get("test1000"); 
  EXPECT_TRUE(v.isNumber());
  EXPECT_EQ(1000ULL, v.getUInt());

  v = s.get("test1023"); 
  EXPECT_TRUE(v.isNumber());
  EXPECT_EQ(1023ULL, v.getUInt());

  // none existing
  v = s.get("test0"); 
  EXPECT_TRUE(v.isNone());

  v = s.get("test1"); 
  EXPECT_TRUE(v.isNone());

  v = s.get("test1024"); 
  EXPECT_TRUE(v.isNone());
}

TEST(LookupTest, LookupLinear) {
  std::string value("{");
  for (size_t i = 0; i < 4; ++i) {
    if (i > 0) {
      value.append(","); 
    }
    value.append("\"test"); 
    value.append(std::to_string(i));
    value.append("\":"); 
    value.append(std::to_string(i));
  }
  value.append("}"); 

  JasonParser parser;
  parser.parse(value);
  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());

  JasonSlice v;
  v = s.get("test0"); 
  EXPECT_TRUE(v.isNumber());
  EXPECT_EQ(0ULL, v.getUInt());
  
  v = s.get("test1"); 
  EXPECT_TRUE(v.isNumber());
  EXPECT_EQ(1ULL, v.getUInt());

  v = s.get("test2"); 
  EXPECT_TRUE(v.isNumber());
  EXPECT_EQ(2ULL, v.getUInt());
  
  v = s.get("test3"); 
  EXPECT_TRUE(v.isNumber());
  EXPECT_EQ(3ULL, v.getUInt());
}

TEST(LookupTest, LookupBinary) {
  std::string value("{");
  for (size_t i = 0; i < 128; ++i) {
    if (i > 0) {
      value.append(","); 
    }
    value.append("\"test"); 
    value.append(std::to_string(i));
    value.append("\":"); 
    value.append(std::to_string(i));
  }
  value.append("}"); 

  JasonParser parser;
  parser.parse(value);
  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());

  for (size_t i = 0; i < 128; ++i) {
    std::string key = "test";
    key.append(std::to_string(i));
    JasonSlice v = s.get(key);
  
    EXPECT_TRUE(v.isNumber());
    EXPECT_EQ(i, v.getUInt());
  } 
}

TEST(LookupTest, LookupBinarySamePrefix) {
  std::string value("{");
  for (size_t i = 0; i < 128; ++i) {
    if (i > 0) {
      value.append(","); 
    }
    value.append("\"test"); 
    for (size_t j = 0; j < i; ++j) {
      value.append("x");
    }
    value.append("\":"); 
    value.append(std::to_string(i));
  }
  value.append("}"); 

  JasonParser parser;
  parser.parse(value);
  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());

  for (size_t i = 0; i < 128; ++i) {
    std::string key = "test";
    for (size_t j = 0; j < i; ++j) {
      key.append("x");
    }
    JasonSlice v = s.get(key);
  
    EXPECT_TRUE(v.isNumber());
    EXPECT_EQ(i, v.getUInt());
  } 
}

TEST(LookupTest, LookupBinaryLongObject) {
  std::string value("{");
  for (size_t i = 0; i < 1127; ++i) {
    if (i > 0) {
      value.append(","); 
    }
    value.append("\"test"); 
    value.append(std::to_string(i));
    value.append("\":"); 
    value.append(std::to_string(i));
  }
  value.append("}"); 

  JasonParser parser;
  parser.parse(value);
  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());

  for (size_t i = 0; i < 1127; ++i) {
    std::string key = "test";
    key.append(std::to_string(i));
    JasonSlice v = s.get(key);
  
    EXPECT_TRUE(v.isNumber());
    EXPECT_EQ(i, v.getUInt());
  } 
}


int main (int argc, char* argv[]) {
  JasonSlice::Initialize();

  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
