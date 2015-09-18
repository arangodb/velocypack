
#include <iostream>

#include "Jason.h"
#include "JasonBuilder.h"
#include "JasonParser.h"
#include "JasonSlice.h"
#include "JasonType.h"

#include "gtest/gtest.h"

using Jason            = triagens::basics::Jason;
using JasonBuilder     = triagens::basics::JasonBuilder;
using JasonLength      = triagens::basics::JasonLength;
using JasonPair        = triagens::basics::JasonPair;
using JasonParser      = triagens::basics::JasonParser;
using JasonSlice       = triagens::basics::JasonSlice;
using JasonType        = triagens::basics::JasonType;
  
static char Buffer[4096];

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
  b.set(Jason());
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t const correctResult[] 
    = { 0x00 };

  EXPECT_EQ(sizeof(correctResult), len);
  EXPECT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, False) {
  JasonBuilder b;
  b.set(Jason(false));
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t const correctResult[] 
    = { 0x01 };

  EXPECT_EQ(sizeof(correctResult), len);
  EXPECT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, True) {
  JasonBuilder b;
  b.set(Jason(true));
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
  b.set(Jason(value));
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
  b.set(Jason("abcdefghijklmnopqrstuvwxyz"));
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
  b.set(Jason(0, JasonType::Array));
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

  EXPECT_EQ(sizeof(correctResult), len);
  EXPECT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, ObjectEmpty) {
  JasonBuilder b;
  b.set(Jason(0, JasonType::Object));
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
  b.set(Jason(4, JasonType::Object));
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
  b.set(Jason(const_cast<void const*>(static_cast<void*>(externalStuff)), 
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
  b.set(Jason(value));
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
  b.set(Jason(value));
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
  b.set(Jason(value));
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
  b.set(Jason(value));

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
  b.set(Jason(value));

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
  b.set(JasonPair(binaryStuff, sizeof(binaryStuff)));
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
  b.set(JasonPair(key, 0x12345678, JasonType::ID));
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
  b.set(Jason(JasonType::ArangoDB_id));

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
}

TEST(ParserTest, False) {
  std::string const value("false");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);
}

TEST(ParserTest, True) {
  std::string const value("true");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);
}

TEST(ParserTest, Zero) {
  std::string const value("0");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);
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
}

TEST(ParserTest, Int2) {
  std::string const value("100000");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);
}

TEST(ParserTest, Int3) {
  std::string const value("-100000");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);
}

TEST(ParserTest, Double1) {
  std::string const value("1.0124");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);
}

TEST(ParserTest, Double2) {
  std::string const value("-1.0124");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);
}

TEST(ParserTest, DoubleScientific1) {
  std::string const value("-1.0124e42");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);
}

TEST(ParserTest, DoubleScientific2) {
  std::string const value("-1.0124e+42");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);
}

TEST(ParserTest, DoubleScientific3) {
  std::string const value("3122243.0124e-42");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);
}

TEST(ParserTest, DoubleScientific4) {
  std::string const value("2335431.0124E-42");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);
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
}

TEST(ParserTest, StringLiteralEmpty) {
  std::string const value("\"\"");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);
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
}

TEST(ParserTest, StringLiteralUtf8SequenceUpperCase) {
  std::string const value("\"der m\\u00D6ter\"");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);
}

TEST(ParserTest, StringLiteralUtf8Chars) {
  std::string const value("\"der mötör klötörte mät dän fößen\"");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);
}

TEST(ParserTest, StringLiteralWithSpecials) {
  std::string const value("  \"der\\thund\\nging\\rin\\fden\\\\wald\\\"und\\b\\nden'fux\"  ");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);
}

TEST(ParserTest, EmptyArray) {
  std::string const value("[]");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);
}

TEST(ParserTest, WhitespacedArray) {
  std::string const value("  [    ]   ");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);
}

TEST(ParserTest, Array1) {
  std::string const value("[1]");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);
}

TEST(ParserTest, Array2) {
  std::string const value("[1,2]");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);
}

TEST(ParserTest, Array3) {
  std::string const value("[-1,2, 4.5, 3, -99.99]");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);
}

TEST(ParserTest, Array4) {
  std::string const value("[\"foo\", \"bar\", \"baz\", null, true, false, -42.23 ]");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);
}

TEST(ParserTest, NestedArray1) {
  std::string const value("[ [ ] ]");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);
}

TEST(ParserTest, NestedArray2) {
  std::string const value("[ [ ],[[]],[],[ [[ [], [ ], [ ] ], [ ] ] ], [] ]");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);
}

TEST(ParserTest, NestedArray3) {
  std::string const value("[ [ \"foo\", [ \"bar\", \"baz\", null ], true, false ], -42.23 ]");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);
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
}

TEST(ParserTest, ObjectSimple2) {
  std::string const value("{ \"foo\" : \"bar\", \"baz\":true}");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);
}

TEST(ParserTest, ObjectDenseNotation) {
  std::string const value("{\"a\":\"b\",\"c\":\"d\"}");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);
}

TEST(ParserTest, ObjectReservedKeys) {
  std::string const value("{ \"null\" : \"true\", \"false\":\"bar\", \"true\":\"foo\"}");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);
}

TEST(ParserTest, ObjectMixed) {
  std::string const value("{\"foo\":null,\"bar\":true,\"baz\":13.53,\"qux\":[1],\"quz\":{}}");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  EXPECT_EQ(1ULL, len);
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


int main (int argc, char* argv[]) {
  JasonSlice::Initialize();

  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
