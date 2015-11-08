////////////////////////////////////////////////////////////////////////////////
/// @brief Library to build up VPack documents.
///
/// DISCLAIMER
///
/// Copyright 2015 ArangoDB GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Max Neunhoeffer
/// @author Jan Steemann
/// @author Copyright 2015, ArangoDB GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include <ostream>
#include <string>

#include "tests-common.h"
  
static unsigned char LocalBuffer[4096];

TEST(SliceTest, Null) {
  LocalBuffer[0] = 0x18;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Null, slice.type());
  ASSERT_TRUE(slice.isNull());
  ASSERT_EQ(1ULL, slice.byteSize());
}

TEST(SliceTest, False) {
  LocalBuffer[0] = 0x19;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Bool, slice.type());
  ASSERT_TRUE(slice.isBool());
  ASSERT_EQ(1ULL, slice.byteSize());
  ASSERT_FALSE(slice.getBool());
}

TEST(SliceTest, True) {
  LocalBuffer[0] = 0x1a;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Bool, slice.type());
  ASSERT_TRUE(slice.isBool());
  ASSERT_EQ(1ULL, slice.byteSize());
  ASSERT_TRUE(slice.getBool());
}

TEST(SliceTest, MinKey) {
  LocalBuffer[0] = 0x1e;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::MinKey, slice.type());
  ASSERT_TRUE(slice.isMinKey());
  ASSERT_EQ(1ULL, slice.byteSize());
}

TEST(SliceTest, MaxKey) {
  LocalBuffer[0] = 0x1f;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::MaxKey, slice.type());
  ASSERT_TRUE(slice.isMaxKey());
  ASSERT_EQ(1ULL, slice.byteSize());
}

TEST(SliceTest, Double) {
  LocalBuffer[0] = 0x1b;

  double value = 23.5;
  dumpDouble(value, reinterpret_cast<uint8_t*>(LocalBuffer) + 1);

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Double, slice.type());
  ASSERT_TRUE(slice.isDouble());
  ASSERT_EQ(9ULL, slice.byteSize());
  ASSERT_DOUBLE_EQ(value, slice.getDouble());
}

TEST(SliceTest, DoubleNegative) {
  LocalBuffer[0] = 0x1b;

  double value = -999.91355;
  dumpDouble(value, reinterpret_cast<uint8_t*>(LocalBuffer) + 1);

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Double, slice.type());
  ASSERT_TRUE(slice.isDouble());
  ASSERT_EQ(9ULL, slice.byteSize());
  ASSERT_DOUBLE_EQ(value, slice.getDouble());
}

TEST(SliceTest, SmallInt) {
  int64_t expected[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -6, -5, -4, -3, -2, -1 };

  for (int i = 0; i < 16; ++i) {
    LocalBuffer[0] = 0x30 + i;

    Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));
    ASSERT_EQ(ValueType::SmallInt, slice.type());
    ASSERT_TRUE(slice.isSmallInt());
    ASSERT_EQ(1ULL, slice.byteSize());

    ASSERT_EQ(expected[i], slice.getSmallInt());
  } 
}

TEST(SliceTest, Int1) {
  LocalBuffer[0] = 0x20;
  uint8_t value = 0x33;
  memcpy(&LocalBuffer[1], (void*) &value, sizeof(value));

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(2ULL, slice.byteSize());

  ASSERT_EQ(value, slice.getInt());
}

TEST(SliceTest, Int2) {
  LocalBuffer[0] = 0x21;
  uint8_t* p = (uint8_t*) &LocalBuffer[1];
  *p++ = 0x23;
  *p++ = 0x42;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(3ULL, slice.byteSize());
  ASSERT_EQ(0x4223LL, slice.getInt());
}

TEST(SliceTest, Int3) {
  LocalBuffer[0] = 0x22;
  uint8_t* p = (uint8_t*) &LocalBuffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(4ULL, slice.byteSize());
  ASSERT_EQ(0x664223LL, slice.getInt());
}

TEST(SliceTest, Int4) {
  LocalBuffer[0] = 0x23;
  uint8_t* p = (uint8_t*) &LocalBuffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0x7c;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(5ULL, slice.byteSize());
  ASSERT_EQ(0x7c664223LL, slice.getInt());
}

TEST(SliceTest, Int5) {
  LocalBuffer[0] = 0x24;
  uint8_t* p = (uint8_t*) &LocalBuffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;
  *p++ = 0x6f;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(6ULL, slice.byteSize());
  ASSERT_EQ(0x6fac664223LL, slice.getInt());
}

TEST(SliceTest, Int6) {
  LocalBuffer[0] = 0x25;
  uint8_t* p = (uint8_t*) &LocalBuffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;
  *p++ = 0xff;
  *p++ = 0x3f;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(7ULL, slice.byteSize());
  ASSERT_EQ(0x3fffac664223LL, slice.getInt());
}

TEST(SliceTest, Int7) {
  LocalBuffer[0] = 0x26;
  uint8_t* p = (uint8_t*) &LocalBuffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;
  *p++ = 0xff;
  *p++ = 0x3f;
  *p++ = 0x5a;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(8ULL, slice.byteSize());
  ASSERT_EQ(0x5a3fffac664223LL, slice.getInt());
}

TEST(SliceTest, Int8) {
  LocalBuffer[0] = 0x27;
  uint8_t* p = (uint8_t*) &LocalBuffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;
  *p++ = 0xff;
  *p++ = 0x3f;
  *p++ = 0xfa;
  *p++ = 0x6f;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(9ULL, slice.byteSize());
  ASSERT_EQ(0x6ffa3fffac664223LL, slice.getInt());
}

TEST(SliceTest, NegInt1) {
  LocalBuffer[0] = 0x20;
  uint8_t value = 0xa3;
  memcpy(&LocalBuffer[1], (void*) &value, sizeof(value));

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(2ULL, slice.byteSize());

  ASSERT_EQ(static_cast<int64_t>(0xffffffffffffffa3ULL), slice.getInt());
}

TEST(SliceTest, NegInt2) {
  LocalBuffer[0] = 0x21;
  uint8_t* p = (uint8_t*) &LocalBuffer[1];
  *p++ = 0x23;
  *p++ = 0xe2;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(3ULL, slice.byteSize());
  ASSERT_EQ(static_cast<int64_t>(0xffffffffffffe223ULL), slice.getInt());
}

TEST(SliceTest, NegInt3) {
  LocalBuffer[0] = 0x22;
  uint8_t* p = (uint8_t*) &LocalBuffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0xd6;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(4ULL, slice.byteSize());
  ASSERT_EQ(static_cast<int64_t>(0xffffffffffd64223ULL), slice.getInt());
}

TEST(SliceTest, NegInt4) {
  LocalBuffer[0] = 0x23;
  uint8_t* p = (uint8_t*) &LocalBuffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(5ULL, slice.byteSize());
  ASSERT_EQ(static_cast<int64_t>(0xffffffffac664223ULL), slice.getInt());
}

TEST(SliceTest, NegInt5) {
  LocalBuffer[0] = 0x24;
  uint8_t* p = (uint8_t*) &LocalBuffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;
  *p++ = 0xff;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(6ULL, slice.byteSize());
  ASSERT_EQ(static_cast<int64_t>(0xffffffffac664223ULL), slice.getInt());
}

TEST(SliceTest, NegInt6) {
  LocalBuffer[0] = 0x25;
  uint8_t* p = (uint8_t*) &LocalBuffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;
  *p++ = 0xff;
  *p++ = 0xef;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(7ULL, slice.byteSize());
  ASSERT_EQ(static_cast<int64_t>(0xffffefffac664223ULL), slice.getInt());
}

TEST(SliceTest, NegInt7) {
  LocalBuffer[0] = 0x26;
  uint8_t* p = (uint8_t*) &LocalBuffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;
  *p++ = 0xff;
  *p++ = 0xef;
  *p++ = 0xfa;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(8ULL, slice.byteSize());
  ASSERT_EQ(static_cast<int64_t>(0xfffaefffac664223ULL), slice.getInt());
}

TEST(SliceTest, NegInt8) {
  LocalBuffer[0] = 0x27;
  uint8_t* p = (uint8_t*) &LocalBuffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;
  *p++ = 0xff;
  *p++ = 0xef;
  *p++ = 0xfa;
  *p++ = 0x8e;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(9ULL, slice.byteSize());
  ASSERT_EQ(static_cast<int64_t>(0x8efaefffac664223ULL), slice.getInt());
}

TEST(SliceTest, UInt1) {
  LocalBuffer[0] = 0x28;
  uint8_t value = 0x33;
  memcpy(&LocalBuffer[1], (void*) &value, sizeof(value));

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::UInt, slice.type());
  ASSERT_TRUE(slice.isUInt());
  ASSERT_EQ(2ULL, slice.byteSize());
  ASSERT_EQ(value, slice.getUInt());
}

TEST(SliceTest, UInt2) {
  LocalBuffer[0] = 0x29;
  uint8_t* p = (uint8_t*) &LocalBuffer[1];
  *p++ = 0x23;
  *p++ = 0x42;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::UInt, slice.type());
  ASSERT_TRUE(slice.isUInt());
  ASSERT_EQ(3ULL, slice.byteSize());
  ASSERT_EQ(0x4223ULL, slice.getUInt());
}

TEST(SliceTest, UInt3) {
  LocalBuffer[0] = 0x2a;
  uint8_t* p = (uint8_t*) &LocalBuffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::UInt, slice.type());
  ASSERT_TRUE(slice.isUInt());
  ASSERT_EQ(4ULL, slice.byteSize());
  ASSERT_EQ(0x664223ULL, slice.getUInt());
}

TEST(SliceTest, UInt4) {
  LocalBuffer[0] = 0x2b;
  uint8_t* p = (uint8_t*) &LocalBuffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::UInt, slice.type());
  ASSERT_TRUE(slice.isUInt());
  ASSERT_EQ(5ULL, slice.byteSize());
  ASSERT_EQ(0xac664223ULL, slice.getUInt());
}

TEST(SliceTest, UInt5) {
  LocalBuffer[0] = 0x2c;
  uint8_t* p = (uint8_t*) &LocalBuffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;
  *p++ = 0xff;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::UInt, slice.type());
  ASSERT_TRUE(slice.isUInt());
  ASSERT_EQ(6ULL, slice.byteSize());
  ASSERT_EQ(0xffac664223ULL, slice.getUInt());
}

TEST(SliceTest, UInt6) {
  LocalBuffer[0] = 0x2d;
  uint8_t* p = (uint8_t*) &LocalBuffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;
  *p++ = 0xff;
  *p++ = 0xee;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::UInt, slice.type());
  ASSERT_TRUE(slice.isUInt());
  ASSERT_EQ(7ULL, slice.byteSize());
  ASSERT_EQ(0xeeffac664223ULL, slice.getUInt());
}

TEST(SliceTest, UInt7) {
  LocalBuffer[0] = 0x2e;
  uint8_t* p = (uint8_t*) &LocalBuffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;
  *p++ = 0xff;
  *p++ = 0xee;
  *p++ = 0x59;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::UInt, slice.type());
  ASSERT_TRUE(slice.isUInt());
  ASSERT_EQ(8ULL, slice.byteSize());
  ASSERT_EQ(0x59eeffac664223ULL, slice.getUInt());
}

TEST(SliceTest, UInt8) {
  LocalBuffer[0] = 0x2f;
  uint8_t* p = (uint8_t*) &LocalBuffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;
  *p++ = 0xff;
  *p++ = 0xee;
  *p++ = 0x59;
  *p++ = 0xab;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::UInt, slice.type());
  ASSERT_TRUE(slice.isUInt());
  ASSERT_EQ(9ULL, slice.byteSize());
  ASSERT_EQ(0xab59eeffac664223ULL, slice.getUInt());
}

TEST(SliceTest, ArrayEmpty) {
  LocalBuffer[0] = 0x01;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Array, slice.type());
  ASSERT_TRUE(slice.isArray());
  ASSERT_EQ(1ULL, slice.byteSize());
  ASSERT_EQ(0ULL, slice.length());
}

TEST(SliceTest, StringEmpty) {
  LocalBuffer[0] = 0x40;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::String, slice.type());
  ASSERT_TRUE(slice.isString());
  ASSERT_EQ(1ULL, slice.byteSize());
  ValueLength len;
  char const* s = slice.getString(len);
  ASSERT_EQ(0ULL, len);
  ASSERT_EQ(0, strncmp(s, "", len));

  ASSERT_EQ("", slice.copyString());
}

TEST(SliceTest, String1) {
  LocalBuffer[0] = 0x40 + static_cast<char>(strlen("foobar"));

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));
  uint8_t* p = (uint8_t*) &LocalBuffer[1];
  *p++ = (uint8_t) 'f';
  *p++ = (uint8_t) 'o';
  *p++ = (uint8_t) 'o';
  *p++ = (uint8_t) 'b';
  *p++ = (uint8_t) 'a';
  *p++ = (uint8_t) 'r';

  ASSERT_EQ(ValueType::String, slice.type());
  ASSERT_TRUE(slice.isString());
  ASSERT_EQ(7ULL, slice.byteSize());
  ValueLength len;
  char const* s = slice.getString(len);
  ASSERT_EQ(6ULL, len);
  ASSERT_EQ(0, strncmp(s, "foobar", len));

  ASSERT_EQ("foobar", slice.copyString());
}

TEST(SliceTest, String2) {
  LocalBuffer[0] = 0x48;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));
  uint8_t* p = (uint8_t*) &LocalBuffer[1];
  *p++ = (uint8_t) '1';
  *p++ = (uint8_t) '2';
  *p++ = (uint8_t) '3';
  *p++ = (uint8_t) 'f';
  *p++ = (uint8_t) '\r';
  *p++ = (uint8_t) '\t';
  *p++ = (uint8_t) '\n';
  *p++ = (uint8_t) 'x';

  ASSERT_EQ(ValueType::String, slice.type());
  ASSERT_TRUE(slice.isString());
  ASSERT_EQ(9ULL, slice.byteSize());
  ValueLength len;
  char const* s = slice.getString(len);
  ASSERT_EQ(8ULL, len);
  ASSERT_EQ(0, strncmp(s, "123f\r\t\nx", len));

  ASSERT_EQ("123f\r\t\nx", slice.copyString());
}

TEST(SliceTest, StringNullBytes) {
  LocalBuffer[0] = 0x48;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));
  uint8_t* p = (uint8_t*) &LocalBuffer[1];
  *p++ = (uint8_t) '\0';
  *p++ = (uint8_t) '1';
  *p++ = (uint8_t) '2';
  *p++ = (uint8_t) '\0';
  *p++ = (uint8_t) '3';
  *p++ = (uint8_t) '4';
  *p++ = (uint8_t) '\0';
  *p++ = (uint8_t) 'x';

  ASSERT_EQ(ValueType::String, slice.type());
  ASSERT_TRUE(slice.isString());
  ASSERT_EQ(9ULL, slice.byteSize());
  ValueLength len;
  slice.getString(len);
  ASSERT_EQ(8ULL, len);

  std::string s(slice.copyString());
  ASSERT_EQ(8ULL, s.size());
  ASSERT_EQ('\0', s[0]);
  ASSERT_EQ('1', s[1]);
  ASSERT_EQ('2', s[2]);
  ASSERT_EQ('\0', s[3]);
  ASSERT_EQ('3', s[4]);
  ASSERT_EQ('4', s[5]);
  ASSERT_EQ('\0', s[6]);
  ASSERT_EQ('x', s[7]);
}

TEST(SliceTest, StringLong1) {
  LocalBuffer[0] = 0xbf;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));
  uint8_t* p = (uint8_t*) &LocalBuffer[1];
  // length
  *p++ = (uint8_t) 6;
  *p++ = (uint8_t) 0;
  *p++ = (uint8_t) 0;
  *p++ = (uint8_t) 0;
  *p++ = (uint8_t) 0;
  *p++ = (uint8_t) 0;
  *p++ = (uint8_t) 0;
  *p++ = (uint8_t) 0;

  *p++ = (uint8_t) 'f';
  *p++ = (uint8_t) 'o';
  *p++ = (uint8_t) 'o';
  *p++ = (uint8_t) 'b';
  *p++ = (uint8_t) 'a';
  *p++ = (uint8_t) 'r';

  ASSERT_EQ(ValueType::String, slice.type());
  ASSERT_TRUE(slice.isString());
  ASSERT_EQ(15ULL, slice.byteSize());
  ValueLength len;
  char const* s = slice.getString(len);
  ASSERT_EQ(6ULL, len);
  ASSERT_EQ(0, strncmp(s, "foobar", len));

  ASSERT_EQ("foobar", slice.copyString());
}

TEST(SliceTest, IterateArrayValues) {
  std::string const value("[1,2,3,4,null,true,\"foo\",\"bar\"]");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  size_t state = 0;
  Collection::forEach(s, [&state] (Slice const& value, ValueLength) -> bool {
    switch (state++) {
      case 0:
        EXPECT_TRUE(value.isNumber());
        EXPECT_EQ(1ULL, value.getUInt());
        break;
      case 1:
        EXPECT_TRUE(value.isNumber());
        EXPECT_EQ(2ULL, value.getUInt());
        break;
      case 2:
        EXPECT_TRUE(value.isNumber());
        EXPECT_EQ(3ULL, value.getUInt());
        break;
      case 3:
        EXPECT_TRUE(value.isNumber());
        EXPECT_EQ(4ULL, value.getUInt());
        break;
      case 4:
        EXPECT_TRUE(value.isNull());
        break;
      case 5:
        EXPECT_TRUE(value.isBoolean());
        EXPECT_TRUE(value.getBoolean());
        break;
      case 6:
        EXPECT_TRUE(value.isString());
        EXPECT_EQ("foo", value.copyString());
        break;
      case 7:
        EXPECT_TRUE(value.isString());
        EXPECT_EQ("bar", value.copyString());
        break;
    }
    return true;
  });
  ASSERT_EQ(8U, state);
}

TEST(SliceTest, ArrayCases1) {
  uint8_t buf[] = { 0x02, 0x05, 0x31, 0x32, 0x33};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases2) {
  uint8_t buf[] = { 0x02, 0x06, 0x00, 0x31, 0x32, 0x33};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases3) {
  uint8_t buf[] = { 0x02, 0x08, 0x00, 0x00, 0x00, 0x31, 0x32, 0x33};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases4) {
  uint8_t buf[] = { 0x02, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x31, 0x32, 0x33};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases5) {
  uint8_t buf[] = { 0x03, 0x06, 0x00, 0x31, 0x32, 0x33};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases6) {
  uint8_t buf[] = { 0x03, 0x08, 0x00, 0x00, 0x00, 0x31, 0x32, 0x33};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases7) {
  uint8_t buf[] = { 0x03, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x31, 0x32, 0x33};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases8) {
  uint8_t buf[] = { 0x04, 0x08, 0x00, 0x00, 0x00, 0x31, 0x32, 0x33};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases9) {
  uint8_t buf[] = { 0x04, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x31, 0x32, 0x33};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases10) {
  uint8_t buf[] = { 0x05, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x31, 0x32, 0x33};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases11) {
  uint8_t buf[] = { 0x06, 0x09, 0x03, 0x31, 0x32, 0x33, 0x03, 0x04, 0x05};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases12) {
  uint8_t buf[] = { 0x06, 0x0b, 0x03, 0x00, 0x00, 0x31, 0x32, 0x33, 0x05, 0x06, 0x07};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases13) {
  uint8_t buf[] = { 0x06, 0x0f, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x31, 0x32, 0x33, 0x09, 0x0a, 0x0b};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases14) {
  uint8_t buf[] = { 0x07, 0x0e, 0x00, 0x03, 0x00, 0x31, 0x32, 0x33, 
                    0x05, 0x00, 0x06, 0x00, 0x07, 0x00};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases15) {
  uint8_t buf[] = { 0x07, 0x12, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x31, 0x32, 0x33, 0x09, 0x00, 0x0a, 0x00, 0x0b, 0x00};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases16) {
  uint8_t buf[] = { 0x08, 0x18, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 
                    0x31, 0x32, 0x33, 0x09, 0x00, 0x00, 0x00, 0x0a, 0x00, 
                    0x00, 0x00, 0x0b, 0x00, 0x00, 0x00};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases17) {
  uint8_t buf[] = { 0x09, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x31, 0x32, 0x33, 0x09, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ObjectCases1) {
  uint8_t buf[] = { 0x0b, 0x00, 0x03, 0x41, 0x61, 0x31, 0x41, 0x62, 
                    0x32, 0x41, 0x63, 0x33, 0x03, 0x06, 0x09 };
  buf[1] = sizeof(buf);  // set the bytelength
  Slice s(buf);
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s["a"];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ObjectCases2) {
  uint8_t buf[] = { 0x0b, 0x00, 0x03, 0x00, 0x00, 0x41, 0x61, 0x31,
                    0x41, 0x62, 0x32, 0x41, 0x63, 0x33, 0x05, 0x08, 
                    0x0b };
  buf[1] = sizeof(buf);  // set the bytelength
  Slice s(buf);
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s["a"];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ObjectCases3) {
  uint8_t buf[] = { 0x0b, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x41, 0x61, 0x31, 0x41, 0x62, 0x32, 0x41, 
                    0x63, 0x33, 0x09, 0x0c, 0x0f };
  buf[1] = sizeof(buf);  // set the bytelength
  Slice s(buf);
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s["a"];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ObjectCases4) {
  uint8_t buf[] = { 0x0f, 0x00, 0x03, 0x41, 0x61, 0x31, 0x41, 0x62, 
                    0x32, 0x41, 0x63, 0x33, 0x03, 0x06, 0x09 };
  buf[1] = sizeof(buf);  // set the bytelength
  Slice s(buf);
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s["a"];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ObjectCases5) {
  uint8_t buf[] = { 0x0f, 0x00, 0x03, 0x00, 0x00, 0x41, 0x61, 0x31,
                    0x41, 0x62, 0x32, 0x41, 0x63, 0x33, 0x05, 0x08, 
                    0x0b };
  buf[1] = sizeof(buf);  // set the bytelength
  Slice s(buf);
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s["a"];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ObjectCases6) {
  uint8_t buf[] = { 0x0f, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x41, 0x61, 0x31, 0x41, 0x62, 0x32, 0x41, 
                    0x63, 0x33, 0x09, 0x0c, 0x0f };
  buf[1] = sizeof(buf);  // set the bytelength
  Slice s(buf);
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s["a"];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ObjectCases7) {
  uint8_t buf[] = { 0x0c, 0x00, 0x00, 0x03, 0x00, 0x41, 0x61, 0x31,
                    0x41, 0x62, 0x32, 0x41, 0x63, 0x33, 0x05, 0x00,
                    0x08, 0x00, 0x0b, 0x00 };
  buf[1] = sizeof(buf);  // set the bytelength
  Slice s(buf);
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s["a"];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ObjectCases8) {
  uint8_t buf[] = { 0x0c, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x41, 0x61, 0x31, 0x41, 0x62, 0x32, 0x41,
                    0x63, 0x33, 0x09, 0x00, 0x0c, 0x00, 0x0f, 0x00 };
  buf[1] = sizeof(buf);  // set the bytelength
  Slice s(buf);
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s["a"];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ObjectCases9) {
  uint8_t buf[] = { 0x10, 0x00, 0x00, 0x03, 0x00, 0x41, 0x61, 0x31,
                    0x41, 0x62, 0x32, 0x41, 0x63, 0x33, 0x05, 0x00,
                    0x08, 0x00, 0x0b, 0x00 };
  buf[1] = sizeof(buf);  // set the bytelength
  Slice s(buf);
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s["a"];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ObjectCases10) {
  uint8_t buf[] = { 0x10, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x41, 0x61, 0x31, 0x41, 0x62, 0x32, 0x41,
                    0x63, 0x33, 0x09, 0x00, 0x0c, 0x00, 0x0f, 0x00 };
  buf[1] = sizeof(buf);  // set the bytelength
  Slice s(buf);
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s["a"];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ObjectCases11) {
  uint8_t buf[] = { 0x0d, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
                    0x00, 0x41, 0x61, 0x31, 0x41, 0x62, 0x32, 0x41,
                    0x63, 0x33, 0x09, 0x00, 0x00, 0x00, 0x0c, 0x00,
                    0x00, 0x00, 0x0f, 0x00, 0x00, 0x00 };
  buf[1] = sizeof(buf);  // set the bytelength
  Slice s(buf);
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s["a"];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ObjectCases12) {
  uint8_t buf[] = { 0x11, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
                    0x00, 0x41, 0x61, 0x31, 0x41, 0x62, 0x32, 0x41,
                    0x63, 0x33, 0x09, 0x00, 0x00, 0x00, 0x0c, 0x00,
                    0x00, 0x00, 0x0f, 0x00, 0x00, 0x00 };
  buf[1] = sizeof(buf);  // set the bytelength
  Slice s(buf);
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s["a"];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ObjectCases13) {
  uint8_t buf[] = { 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x41, 0x61, 0x31, 0x41, 0x62, 0x32, 0x41,
                    0x63, 0x33, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00};
  buf[1] = sizeof(buf);  // set the bytelength
  Slice s(buf);
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s["a"];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ObjectCases14) {
  uint8_t buf[] = { 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x41, 0x61, 0x31, 0x41, 0x62, 0x32, 0x41,
                    0x63, 0x33, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00};
  buf[1] = sizeof(buf);  // set the bytelength
  Slice s(buf);
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s["a"];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

int main (int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

