
#include "Jason.h"
#include "JasonBuilder.h"
#include "JasonParser.h"
#include "JasonSlice.h"
#include "JasonType.h"

#include "gtest/gtest.h"

using Jason        = triagens::basics::Jason;
using JasonPair    = triagens::basics::JasonPair;
using JasonBuilder = triagens::basics::JasonBuilder;
using JasonLength  = triagens::basics::JasonLength;
using JasonSlice   = triagens::basics::JasonSlice;
using JasonType    = triagens::basics::JasonType;
  
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
  uint8_t key[] = { 0x02, 0x03, 0x05, 0x08, 0x0d };

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

int main (int argc, char* argv[]) {
  JasonSlice::Initialize();

  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
