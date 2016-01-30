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

TEST(SliceTest, SliceStart) {
  std::string const value("null");

  Parser parser;
  parser.parse(value);
  std::shared_ptr<Builder> builder = parser.steal();
  Slice s(builder->start());

  ASSERT_EQ(0x18UL, s.head());
  ASSERT_EQ(0x18UL, *s.start());
  ASSERT_EQ('\x18', *s.startAs<char>());
  ASSERT_EQ('\x18', *s.startAs<unsigned char>());
  ASSERT_EQ(0x18UL, *s.startAs<uint8_t>());

  ASSERT_EQ(s.start(), s.begin());
  ASSERT_EQ(s.start() + 1, s.end());
}

TEST(SliceTest, ToJsonNull) {
  std::string const value("null");

  Parser parser;
  parser.parse(value);
  std::shared_ptr<Builder> builder = parser.steal();
  Slice s(builder->start());
  ASSERT_EQ("null", s.toJson());
}

TEST(SliceTest, ToJsonFalse) {
  std::string const value("false");

  Parser parser;
  parser.parse(value);
  std::shared_ptr<Builder> builder = parser.steal();
  Slice s(builder->start());
  ASSERT_EQ("false", s.toJson());
}

TEST(SliceTest, ToJsonTrue) {
  std::string const value("true");

  Parser parser;
  parser.parse(value);
  std::shared_ptr<Builder> builder = parser.steal();
  Slice s(builder->start());
  ASSERT_EQ("true", s.toJson());
}

TEST(SliceTest, ToJsonNumber) {
  std::string const value("-12345");

  Parser parser;
  parser.parse(value);
  std::shared_ptr<Builder> builder = parser.steal();
  Slice s(builder->start());
  ASSERT_EQ("-12345", s.toJson());
}

TEST(SliceTest, ToJsonString) {
  std::string const value("\"foobarbaz\"");

  Parser parser;
  parser.parse(value);
  std::shared_ptr<Builder> builder = parser.steal();
  Slice s(builder->start());
  ASSERT_EQ("\"foobarbaz\"", s.toJson());
}

TEST(SliceTest, ToJsonArray) {
  std::string const value("[1,2,3,4,5]");

  Parser parser;
  parser.parse(value);
  std::shared_ptr<Builder> builder = parser.steal();
  Slice s(builder->start());
  ASSERT_EQ("[1,2,3,4,5]", s.toJson());
}

TEST(SliceTest, ToJsonArrayCompact) {
  std::string const value("[1,2,3,4,5]");

  Options options;
  options.buildUnindexedArrays = true;

  Parser parser(&options);
  parser.parse(value);
  std::shared_ptr<Builder> builder = parser.steal();
  Slice s(builder->start());
  ASSERT_EQ(0x13, s.head());

  ASSERT_EQ("[1,2,3,4,5]", s.toJson());
}

TEST(SliceTest, ToJsonObject) {
  std::string const value("{\"a\":1,\"b\":2,\"c\":3,\"d\":4,\"e\":5}");

  Options options;
  options.buildUnindexedObjects = true;

  Parser parser(&options);
  parser.parse(value);
  std::shared_ptr<Builder> builder = parser.steal();
  Slice s(builder->start());
  ASSERT_EQ(0x14, s.head());

  ASSERT_EQ("{\"a\":1,\"b\":2,\"c\":3,\"d\":4,\"e\":5}", s.toJson());
}

TEST(SliceTest, ToJsonObjectCompact) {
  std::string const value("{\"a\":1,\"b\":2,\"c\":3,\"d\":4,\"e\":5}");

  Parser parser;
  parser.parse(value);
  std::shared_ptr<Builder> builder = parser.steal();
  Slice s(builder->start());
  ASSERT_EQ("{\"a\":1,\"b\":2,\"c\":3,\"d\":4,\"e\":5}", s.toJson());
}

TEST(SliceTest, InvalidGetters) {
  std::string const value("[null,true,1,\"foo\",[],{}]");

  Parser parser;
  parser.parse(value);
  std::shared_ptr<Builder> builder = parser.steal();
  Slice s(builder->start());

  ValueLength len;

  ASSERT_VELOCYPACK_EXCEPTION(s.at(0).getBool(), Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(0).getInt(), Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(0).getUInt(), Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(0).getSmallInt(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(0).getDouble(), Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(0).copyString(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(0).copyBinary(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(0).getString(len),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(0).getBinary(len),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(0).getExternal(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(0).getUTCDate(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(0).length(), Exception::InvalidValueType);

  ASSERT_VELOCYPACK_EXCEPTION(s.at(1).getInt(), Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(1).getUInt(), Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(1).getSmallInt(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(1).getDouble(), Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(1).copyString(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(1).copyBinary(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(1).getString(len),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(1).getBinary(len),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(1).getExternal(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(1).getUTCDate(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(1).length(), Exception::InvalidValueType);

  ASSERT_VELOCYPACK_EXCEPTION(s.at(2).getBool(), Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(2).copyString(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(2).copyBinary(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(2).getString(len),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(2).getBinary(len),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(2).getExternal(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(2).getUTCDate(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(2).length(), Exception::InvalidValueType);

  ASSERT_VELOCYPACK_EXCEPTION(s.at(3).getBool(), Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(3).getInt(), Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(3).getUInt(), Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(3).getSmallInt(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(3).getDouble(), Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(3).getExternal(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(3).getUTCDate(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(3).getBinary(len),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(3).copyBinary(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(3).length(), Exception::InvalidValueType);

  ASSERT_VELOCYPACK_EXCEPTION(s.at(4).getBool(), Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(4).getInt(), Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(4).getUInt(), Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(4).getSmallInt(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(4).getDouble(), Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(4).getExternal(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(4).getUTCDate(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(4).copyBinary(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(4).copyString(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(2).getString(len),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(2).getBinary(len),
                              Exception::InvalidValueType);

  ASSERT_VELOCYPACK_EXCEPTION(s.at(5).getBool(), Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(5).getInt(), Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(5).getUInt(), Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(5).getSmallInt(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(5).getDouble(), Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(5).getExternal(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(5).getUTCDate(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(5).getString(len),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(5).getBinary(len),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(5).copyBinary(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(5).copyString(),
                              Exception::InvalidValueType);
}

TEST(SliceTest, LengthNull) {
  std::string const value("null");

  Parser parser;
  parser.parse(value);
  std::shared_ptr<Builder> builder = parser.steal();
  Slice s(builder->start());

  ASSERT_VELOCYPACK_EXCEPTION(s.length(), Exception::InvalidValueType);
}

TEST(SliceTest, LengthTrue) {
  std::string const value("true");

  Parser parser;
  parser.parse(value);
  std::shared_ptr<Builder> builder = parser.steal();
  Slice s(builder->start());

  ASSERT_VELOCYPACK_EXCEPTION(s.length(), Exception::InvalidValueType);
}

TEST(SliceTest, LengthArrayEmpty) {
  std::string const value("[]");

  Parser parser;
  parser.parse(value);
  std::shared_ptr<Builder> builder = parser.steal();
  Slice s(builder->start());

  ASSERT_EQ(0UL, s.length());
}

TEST(SliceTest, LengthArray) {
  std::string const value("[1,2,3,4,5,6,7,8,\"foo\",\"bar\"]");

  Parser parser;
  parser.parse(value);
  std::shared_ptr<Builder> builder = parser.steal();
  Slice s(builder->start());

  ASSERT_EQ(10UL, s.length());
}

TEST(SliceTest, LengthArrayCompact) {
  std::string const value("[1,2,3,4,5,6,7,8,\"foo\",\"bar\"]");

  Options options;
  options.buildUnindexedArrays = true;

  Parser parser(&options);
  parser.parse(value);
  std::shared_ptr<Builder> builder = parser.steal();
  Slice s(builder->start());

  ASSERT_EQ(0x13, s.head());
  ASSERT_EQ(10UL, s.length());
}

TEST(SliceTest, LengthObjectEmpty) {
  std::string const value("{}");

  Parser parser;
  parser.parse(value);
  std::shared_ptr<Builder> builder = parser.steal();
  Slice s(builder->start());

  ASSERT_EQ(0UL, s.length());
}

TEST(SliceTest, LengthObject) {
  std::string const value(
      "{\"a\":1,\"b\":2,\"c\":3,\"d\":4,\"e\":5,\"f\":6,\"g\":7,\"h\":8,\"i\":"
      "\"foo\",\"j\":\"bar\"}");

  Parser parser;
  parser.parse(value);
  std::shared_ptr<Builder> builder = parser.steal();
  Slice s(builder->start());

  ASSERT_EQ(10UL, s.length());
}

TEST(SliceTest, LengthObjectCompact) {
  std::string const value(
      "{\"a\":1,\"b\":2,\"c\":3,\"d\":4,\"e\":5,\"f\":6,\"g\":7,\"h\":8,\"i\":"
      "\"foo\",\"j\":\"bar\"}");

  Options options;
  options.buildUnindexedObjects = true;

  Parser parser(&options);
  parser.parse(value);
  std::shared_ptr<Builder> builder = parser.steal();
  Slice s(builder->start());

  ASSERT_EQ(0x14, s.head());
  ASSERT_EQ(10UL, s.length());
}

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
  ASSERT_TRUE(slice.isFalse());
  ASSERT_FALSE(slice.isTrue());
  ASSERT_EQ(1ULL, slice.byteSize());
  ASSERT_FALSE(slice.getBool());
}

TEST(SliceTest, True) {
  LocalBuffer[0] = 0x1a;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Bool, slice.type());
  ASSERT_TRUE(slice.isBool());
  ASSERT_FALSE(slice.isFalse());
  ASSERT_TRUE(slice.isTrue());
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
  int64_t expected[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -6, -5, -4, -3, -2, -1};

  for (int i = 0; i < 16; ++i) {
    LocalBuffer[0] = 0x30 + i;

    Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));
    ASSERT_EQ(ValueType::SmallInt, slice.type());
    ASSERT_TRUE(slice.isSmallInt());
    ASSERT_EQ(1ULL, slice.byteSize());

    ASSERT_EQ(expected[i], slice.getSmallInt());
    ASSERT_EQ(expected[i], slice.getInt());
  }
}

TEST(SliceTest, Int1) {
  LocalBuffer[0] = 0x20;
  uint8_t value = 0x33;
  memcpy(&LocalBuffer[1], (void*)&value, sizeof(value));

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(2ULL, slice.byteSize());

  ASSERT_EQ(value, slice.getInt());
  ASSERT_EQ(value, slice.getSmallInt());
}

TEST(SliceTest, Int2) {
  LocalBuffer[0] = 0x21;
  uint8_t* p = (uint8_t*)&LocalBuffer[1];
  *p++ = 0x23;
  *p++ = 0x42;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(3ULL, slice.byteSize());
  ASSERT_EQ(0x4223LL, slice.getInt());
  ASSERT_EQ(0x4223LL, slice.getSmallInt());
}

TEST(SliceTest, Int3) {
  LocalBuffer[0] = 0x22;
  uint8_t* p = (uint8_t*)&LocalBuffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(4ULL, slice.byteSize());
  ASSERT_EQ(0x664223LL, slice.getInt());
  ASSERT_EQ(0x664223LL, slice.getSmallInt());
}

TEST(SliceTest, Int4) {
  LocalBuffer[0] = 0x23;
  uint8_t* p = (uint8_t*)&LocalBuffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0x7c;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(5ULL, slice.byteSize());
  ASSERT_EQ(0x7c664223LL, slice.getInt());
  ASSERT_EQ(0x7c664223LL, slice.getSmallInt());
}

TEST(SliceTest, Int5) {
  LocalBuffer[0] = 0x24;
  uint8_t* p = (uint8_t*)&LocalBuffer[1];
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
  ASSERT_EQ(0x6fac664223LL, slice.getSmallInt());
}

TEST(SliceTest, Int6) {
  LocalBuffer[0] = 0x25;
  uint8_t* p = (uint8_t*)&LocalBuffer[1];
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
  ASSERT_EQ(0x3fffac664223LL, slice.getSmallInt());
}

TEST(SliceTest, Int7) {
  LocalBuffer[0] = 0x26;
  uint8_t* p = (uint8_t*)&LocalBuffer[1];
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
  ASSERT_EQ(0x5a3fffac664223LL, slice.getSmallInt());
}

TEST(SliceTest, Int8) {
  LocalBuffer[0] = 0x27;
  uint8_t* p = (uint8_t*)&LocalBuffer[1];
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
  ASSERT_EQ(0x6ffa3fffac664223LL, slice.getSmallInt());
}

TEST(SliceTest, IntMax) {
  Builder b;
  b.add(Value(INT64_MAX));

  Slice slice(b.slice());

  ASSERT_EQ(ValueType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(9ULL, slice.byteSize());
  ASSERT_EQ(INT64_MAX, slice.getInt());
}

TEST(SliceTest, NegInt1) {
  LocalBuffer[0] = 0x20;
  uint8_t value = 0xa3;
  memcpy(&LocalBuffer[1], (void*)&value, sizeof(value));

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(2ULL, slice.byteSize());

  ASSERT_EQ(static_cast<int64_t>(0xffffffffffffffa3ULL), slice.getInt());
}

TEST(SliceTest, NegInt2) {
  LocalBuffer[0] = 0x21;
  uint8_t* p = (uint8_t*)&LocalBuffer[1];
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
  uint8_t* p = (uint8_t*)&LocalBuffer[1];
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
  uint8_t* p = (uint8_t*)&LocalBuffer[1];
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
  uint8_t* p = (uint8_t*)&LocalBuffer[1];
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
  uint8_t* p = (uint8_t*)&LocalBuffer[1];
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
  uint8_t* p = (uint8_t*)&LocalBuffer[1];
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
  uint8_t* p = (uint8_t*)&LocalBuffer[1];
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

TEST(SliceTest, IntMin) {
  Builder b;
  b.add(Value(INT64_MIN));

  Slice slice(b.slice());

  ASSERT_EQ(ValueType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(9ULL, slice.byteSize());
  ASSERT_EQ(INT64_MIN, slice.getInt());
  ASSERT_VELOCYPACK_EXCEPTION(slice.getUInt(), Exception::NumberOutOfRange);
}

TEST(SliceTest, UInt1) {
  LocalBuffer[0] = 0x28;
  uint8_t value = 0x33;
  memcpy(&LocalBuffer[1], (void*)&value, sizeof(value));

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::UInt, slice.type());
  ASSERT_TRUE(slice.isUInt());
  ASSERT_EQ(2ULL, slice.byteSize());
  ASSERT_EQ(value, slice.getUInt());
}

TEST(SliceTest, UInt2) {
  LocalBuffer[0] = 0x29;
  uint8_t* p = (uint8_t*)&LocalBuffer[1];
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
  uint8_t* p = (uint8_t*)&LocalBuffer[1];
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
  uint8_t* p = (uint8_t*)&LocalBuffer[1];
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
  uint8_t* p = (uint8_t*)&LocalBuffer[1];
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
  uint8_t* p = (uint8_t*)&LocalBuffer[1];
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
  uint8_t* p = (uint8_t*)&LocalBuffer[1];
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
  uint8_t* p = (uint8_t*)&LocalBuffer[1];
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

TEST(SliceTest, UIntMax) {
  Builder b;
  b.add(Value(UINT64_MAX));

  Slice slice(b.slice());

  ASSERT_EQ(ValueType::UInt, slice.type());
  ASSERT_TRUE(slice.isUInt());
  ASSERT_EQ(9ULL, slice.byteSize());
  ASSERT_EQ(UINT64_MAX, slice.getUInt());
  ASSERT_VELOCYPACK_EXCEPTION(slice.getInt(), Exception::NumberOutOfRange);
}

TEST(SliceTest, ArrayEmpty) {
  LocalBuffer[0] = 0x01;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  ASSERT_EQ(ValueType::Array, slice.type());
  ASSERT_TRUE(slice.isArray());
  ASSERT_EQ(1ULL, slice.byteSize());
  ASSERT_EQ(0ULL, slice.length());
}

TEST(SliceTest, StringNoString) {
  Slice slice;

  ASSERT_FALSE(slice.isString());
  ValueLength length;
  ASSERT_VELOCYPACK_EXCEPTION(slice.getString(length),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(slice.getStringLength(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(slice.copyString(), Exception::InvalidValueType);
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

  ASSERT_EQ(0U, slice.getStringLength());
  ASSERT_EQ("", slice.copyString());
}

TEST(SliceTest, String1) {
  LocalBuffer[0] = 0x40 + static_cast<char>(strlen("foobar"));

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));
  uint8_t* p = (uint8_t*)&LocalBuffer[1];
  *p++ = (uint8_t)'f';
  *p++ = (uint8_t)'o';
  *p++ = (uint8_t)'o';
  *p++ = (uint8_t)'b';
  *p++ = (uint8_t)'a';
  *p++ = (uint8_t)'r';

  ASSERT_EQ(ValueType::String, slice.type());
  ASSERT_TRUE(slice.isString());
  ASSERT_EQ(7ULL, slice.byteSize());
  ValueLength len;
  char const* s = slice.getString(len);
  ASSERT_EQ(6ULL, len);
  ASSERT_EQ(0, strncmp(s, "foobar", len));

  ASSERT_EQ(strlen("foobar"), slice.getStringLength());
  ASSERT_EQ("foobar", slice.copyString());
}

TEST(SliceTest, String2) {
  LocalBuffer[0] = 0x48;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));
  uint8_t* p = (uint8_t*)&LocalBuffer[1];
  *p++ = (uint8_t)'1';
  *p++ = (uint8_t)'2';
  *p++ = (uint8_t)'3';
  *p++ = (uint8_t)'f';
  *p++ = (uint8_t)'\r';
  *p++ = (uint8_t)'\t';
  *p++ = (uint8_t)'\n';
  *p++ = (uint8_t)'x';

  ASSERT_EQ(ValueType::String, slice.type());
  ASSERT_TRUE(slice.isString());
  ASSERT_EQ(9ULL, slice.byteSize());
  ValueLength len;
  char const* s = slice.getString(len);
  ASSERT_EQ(8ULL, len);
  ASSERT_EQ(0, strncmp(s, "123f\r\t\nx", len));
  ASSERT_EQ(8U, slice.getStringLength());

  ASSERT_EQ("123f\r\t\nx", slice.copyString());
}

TEST(SliceTest, StringNullBytes) {
  LocalBuffer[0] = 0x48;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));
  uint8_t* p = (uint8_t*)&LocalBuffer[1];
  *p++ = (uint8_t)'\0';
  *p++ = (uint8_t)'1';
  *p++ = (uint8_t)'2';
  *p++ = (uint8_t)'\0';
  *p++ = (uint8_t)'3';
  *p++ = (uint8_t)'4';
  *p++ = (uint8_t)'\0';
  *p++ = (uint8_t)'x';

  ASSERT_EQ(ValueType::String, slice.type());
  ASSERT_TRUE(slice.isString());
  ASSERT_EQ(9ULL, slice.byteSize());
  ValueLength len;
  slice.getString(len);
  ASSERT_EQ(8ULL, len);
  ASSERT_EQ(8U, slice.getStringLength());

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

TEST(SliceTest, StringLong) {
  LocalBuffer[0] = 0xbf;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));
  uint8_t* p = (uint8_t*)&LocalBuffer[1];
  // length
  *p++ = (uint8_t)6;
  *p++ = (uint8_t)0;
  *p++ = (uint8_t)0;
  *p++ = (uint8_t)0;
  *p++ = (uint8_t)0;
  *p++ = (uint8_t)0;
  *p++ = (uint8_t)0;
  *p++ = (uint8_t)0;

  *p++ = (uint8_t)'f';
  *p++ = (uint8_t)'o';
  *p++ = (uint8_t)'o';
  *p++ = (uint8_t)'b';
  *p++ = (uint8_t)'a';
  *p++ = (uint8_t)'r';

  ASSERT_EQ(ValueType::String, slice.type());
  ASSERT_TRUE(slice.isString());
  ASSERT_EQ(15ULL, slice.byteSize());
  ValueLength len;
  char const* s = slice.getString(len);
  ASSERT_EQ(6ULL, len);
  ASSERT_EQ(0, strncmp(s, "foobar", len));
  ASSERT_EQ(6U, slice.getStringLength());

  ASSERT_EQ("foobar", slice.copyString());
}

TEST(SliceTest, BinaryEmpty) {
  uint8_t buf[] = {0xc0, 0x00};
  Slice slice(&buf[0]);

  ASSERT_TRUE(slice.isBinary());
  ValueLength len;
  slice.getBinary(len);
  ASSERT_EQ(0ULL, len);
  ASSERT_EQ(0U, slice.getBinaryLength());
  auto result = slice.copyBinary();
  ASSERT_EQ(0UL, result.size());
}

TEST(SliceTest, BinarySomeValue) {
  uint8_t buf[] = {0xc0, 0x05, 0xfe, 0xfd, 0xfc, 0xfb, 0xfa};
  Slice slice(&buf[0]);

  ASSERT_TRUE(slice.isBinary());
  ValueLength len;
  uint8_t const* s = slice.getBinary(len);
  ASSERT_EQ(5ULL, len);
  ASSERT_EQ(0, memcmp(s, &buf[2], len));
  ASSERT_EQ(5U, slice.getBinaryLength());

  auto result = slice.copyBinary();
  ASSERT_EQ(5UL, result.size());
  ASSERT_EQ(0xfe, result[0]);
  ASSERT_EQ(0xfd, result[1]);
  ASSERT_EQ(0xfc, result[2]);
  ASSERT_EQ(0xfb, result[3]);
  ASSERT_EQ(0xfa, result[4]);
}

TEST(SliceTest, BinaryWithNullBytes) {
  uint8_t buf[] = {0xc0, 0x05, 0x01, 0x02, 0x00, 0x03, 0x00};
  Slice slice(&buf[0]);

  ASSERT_TRUE(slice.isBinary());
  ValueLength len;
  uint8_t const* s = slice.getBinary(len);
  ASSERT_EQ(5ULL, len);
  ASSERT_EQ(0, memcmp(s, &buf[2], len));
  ASSERT_EQ(5U, slice.getBinaryLength());

  auto result = slice.copyBinary();
  ASSERT_EQ(5UL, result.size());
  ASSERT_EQ(0x01, result[0]);
  ASSERT_EQ(0x02, result[1]);
  ASSERT_EQ(0x00, result[2]);
  ASSERT_EQ(0x03, result[3]);
  ASSERT_EQ(0x00, result[4]);
}

TEST(SliceTest, BinaryNonBinary) {
  Slice slice;

  ValueLength len;
  ASSERT_VELOCYPACK_EXCEPTION(slice.getBinary(len),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(slice.getBinaryLength(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(slice.copyBinary(), Exception::InvalidValueType);
}

TEST(SliceTest, ArrayCases1) {
  uint8_t buf[] = {0x02, 0x05, 0x31, 0x32, 0x33};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases2) {
  uint8_t buf[] = {0x02, 0x06, 0x00, 0x31, 0x32, 0x33};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases3) {
  uint8_t buf[] = {0x02, 0x08, 0x00, 0x00, 0x00, 0x31, 0x32, 0x33};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases4) {
  uint8_t buf[] = {0x02, 0x0c, 0x00, 0x00, 0x00, 0x00,
                   0x00, 0x00, 0x00, 0x31, 0x32, 0x33};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases5) {
  uint8_t buf[] = {0x03, 0x06, 0x00, 0x31, 0x32, 0x33};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases6) {
  uint8_t buf[] = {0x03, 0x08, 0x00, 0x00, 0x00, 0x31, 0x32, 0x33};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases7) {
  uint8_t buf[] = {0x03, 0x0c, 0x00, 0x00, 0x00, 0x00,
                   0x00, 0x00, 0x00, 0x31, 0x32, 0x33};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases8) {
  uint8_t buf[] = {0x04, 0x08, 0x00, 0x00, 0x00, 0x31, 0x32, 0x33};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases9) {
  uint8_t buf[] = {0x04, 0x0c, 0x00, 0x00, 0x00, 0x00,
                   0x00, 0x00, 0x00, 0x31, 0x32, 0x33};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases10) {
  uint8_t buf[] = {0x05, 0x0c, 0x00, 0x00, 0x00, 0x00,
                   0x00, 0x00, 0x00, 0x31, 0x32, 0x33};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases11) {
  uint8_t buf[] = {0x06, 0x09, 0x03, 0x31, 0x32, 0x33, 0x03, 0x04, 0x05};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases12) {
  uint8_t buf[] = {0x06, 0x0b, 0x03, 0x00, 0x00, 0x31,
                   0x32, 0x33, 0x05, 0x06, 0x07};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases13) {
  uint8_t buf[] = {0x06, 0x0f, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
                   0x00, 0x31, 0x32, 0x33, 0x09, 0x0a, 0x0b};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases14) {
  uint8_t buf[] = {0x07, 0x0e, 0x00, 0x03, 0x00, 0x31, 0x32,
                   0x33, 0x05, 0x00, 0x06, 0x00, 0x07, 0x00};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases15) {
  uint8_t buf[] = {0x07, 0x12, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
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
  uint8_t buf[] = {0x08, 0x18, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
                   0x00, 0x31, 0x32, 0x33, 0x09, 0x00, 0x00, 0x00,
                   0x0a, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCases17) {
  uint8_t buf[] = {0x09, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                   0x31, 0x32, 0x33, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00,
                   0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                   0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                   0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ArrayCasesCompact) {
  uint8_t buf[] = {0x13, 0x08, 0x30, 0x31, 0x32, 0x33, 0x34, 0x05};

  Slice s(buf);
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(5ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s[0];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(0LL, ss.getInt());
  ss = s[1];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
  ss = s[4];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(4LL, ss.getInt());
}

TEST(SliceTest, ObjectCases1) {
  uint8_t buf[] = {0x0b, 0x00, 0x03, 0x41, 0x61, 0x31, 0x41, 0x62,
                   0x32, 0x41, 0x63, 0x33, 0x03, 0x06, 0x09};
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
  uint8_t buf[] = {0x0b, 0x00, 0x03, 0x00, 0x00, 0x41, 0x61, 0x31, 0x41,
                   0x62, 0x32, 0x41, 0x63, 0x33, 0x05, 0x08, 0x0b};
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
  uint8_t buf[] = {0x0b, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
                   0x00, 0x00, 0x41, 0x61, 0x31, 0x41, 0x62,
                   0x32, 0x41, 0x63, 0x33, 0x09, 0x0c, 0x0f};
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
  uint8_t buf[] = {0x0f, 0x00, 0x03, 0x41, 0x61, 0x31, 0x41, 0x62,
                   0x32, 0x41, 0x63, 0x33, 0x03, 0x06, 0x09};
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
  uint8_t buf[] = {0x0f, 0x00, 0x03, 0x00, 0x00, 0x41, 0x61, 0x31, 0x41,
                   0x62, 0x32, 0x41, 0x63, 0x33, 0x05, 0x08, 0x0b};
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
  uint8_t buf[] = {0x0f, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
                   0x00, 0x00, 0x41, 0x61, 0x31, 0x41, 0x62,
                   0x32, 0x41, 0x63, 0x33, 0x09, 0x0c, 0x0f};
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
  uint8_t buf[] = {0x0c, 0x00, 0x00, 0x03, 0x00, 0x41, 0x61, 0x31, 0x41, 0x62,
                   0x32, 0x41, 0x63, 0x33, 0x05, 0x00, 0x08, 0x00, 0x0b, 0x00};
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
  uint8_t buf[] = {0x0c, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
                   0x00, 0x41, 0x61, 0x31, 0x41, 0x62, 0x32, 0x41,
                   0x63, 0x33, 0x09, 0x00, 0x0c, 0x00, 0x0f, 0x00};
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
  uint8_t buf[] = {0x10, 0x00, 0x00, 0x03, 0x00, 0x41, 0x61, 0x31, 0x41, 0x62,
                   0x32, 0x41, 0x63, 0x33, 0x05, 0x00, 0x08, 0x00, 0x0b, 0x00};
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
  uint8_t buf[] = {0x10, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
                   0x00, 0x41, 0x61, 0x31, 0x41, 0x62, 0x32, 0x41,
                   0x63, 0x33, 0x09, 0x00, 0x0c, 0x00, 0x0f, 0x00};
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
  uint8_t buf[] = {0x0d, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x41,
                   0x61, 0x31, 0x41, 0x62, 0x32, 0x41, 0x63, 0x33, 0x09, 0x00,
                   0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00};
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
  uint8_t buf[] = {0x11, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x41,
                   0x61, 0x31, 0x41, 0x62, 0x32, 0x41, 0x63, 0x33, 0x09, 0x00,
                   0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00};
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
  uint8_t buf[] = {0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x41,
                   0x61, 0x31, 0x41, 0x62, 0x32, 0x41, 0x63, 0x33, 0x09, 0x00,
                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00,
                   0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,
                   0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
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
  uint8_t buf[] = {0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x41,
                   0x61, 0x31, 0x41, 0x62, 0x32, 0x41, 0x63, 0x33, 0x09, 0x00,
                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00,
                   0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,
                   0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  buf[1] = sizeof(buf);  // set the bytelength
  Slice s(buf);
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(3ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s["a"];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
}

TEST(SliceTest, ObjectCompact) {
  uint8_t const buf[] = {0x14, 0x0f, 0x41, 0x61, 0x30, 0x41, 0x62, 0x31,
                         0x41, 0x63, 0x32, 0x41, 0x64, 0x33, 0x04};
  Slice s(buf);
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(4ULL, s.length());
  ASSERT_EQ(sizeof(buf), s.byteSize());
  Slice ss = s["a"];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(0LL, ss.getInt());
  ss = s["b"];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(1LL, ss.getInt());
  ss = s["d"];
  ASSERT_TRUE(ss.isSmallInt());
  ASSERT_EQ(3LL, ss.getInt());
}

TEST(SliceTest, ToStringNull) {
  std::string const value("null");

  std::shared_ptr<Builder> b = Parser::fromJson(value);
  Slice s(b->start());

  ASSERT_EQ("null", s.toString());
}

TEST(SliceTest, ToStringArray) {
  std::string const value("[1,2,3,4,5]");

  std::shared_ptr<Builder> b = Parser::fromJson(value);
  Slice s(b->start());

  ASSERT_EQ("[\n  1,\n  2,\n  3,\n  4,\n  5\n]", s.toString());
}

TEST(SliceTest, ToStringArrayCompact) {
  Options options;
  options.buildUnindexedArrays = true;

  std::string const value("[1,2,3,4,5]");

  std::shared_ptr<Builder> b = Parser::fromJson(value, &options);
  Slice s(b->start());

  ASSERT_EQ(0x13, s.head());
  ASSERT_EQ("[\n  1,\n  2,\n  3,\n  4,\n  5\n]", s.toString());
}

TEST(SliceTest, ToStringArrayEmpty) {
  std::string const value("[]");

  std::shared_ptr<Builder> b = Parser::fromJson(value);
  Slice s(b->start());

  ASSERT_EQ("[\n]", s.toString());
}

TEST(SliceTest, ToStringObjectEmpty) {
  std::string const value("{ }");

  std::shared_ptr<Builder> b = Parser::fromJson(value);
  Slice s(b->start());

  ASSERT_EQ("{\n}", s.toString());
}

TEST(SliceTest, EqualToUniqueValues) {
  std::string const value("[1,2,3,4,null,true,\"foo\",\"bar\"]");

  Parser parser;
  parser.parse(value);

  std::unordered_set<Slice> values;
  for (auto it : ArrayIterator(Slice(parser.start()))) {
    values.emplace(it);
  }

  ASSERT_EQ(8UL, values.size());
}

TEST(SliceTest, EqualToDuplicateValuesNumbers) {
  std::string const value("[1,2,3,4,1,2,3,4,5,9,1]");

  Parser parser;
  parser.parse(value);

  std::unordered_set<Slice> values;
  for (auto it : ArrayIterator(Slice(parser.start()))) {
    values.emplace(it);
  }

  ASSERT_EQ(6UL, values.size());  // 1,2,3,4,5,9
}

TEST(SliceTest, EqualToBiggerNumbers) {
  std::string const value("[1024,1025,1031,1024,1029,1025]");

  Parser parser;
  parser.parse(value);

  std::unordered_set<Slice> values;
  for (auto it : ArrayIterator(Slice(parser.start()))) {
    values.emplace(it);
  }

  ASSERT_EQ(4UL, values.size());  // 1024, 1025, 1029, 1031
}

TEST(SliceTest, EqualToDuplicateValuesStrings) {
  std::string const value(
      "[\"foo\",\"bar\",\"baz\",\"bart\",\"foo\",\"bark\",\"qux\",\"foo\"]");

  Parser parser;
  parser.parse(value);

  std::unordered_set<Slice> values;
  for (auto it : ArrayIterator(Slice(parser.start()))) {
    values.emplace(it);
  }

  ASSERT_EQ(6UL, values.size());  // "foo", "bar", "baz", "bart", "bark", "qux"
}

TEST(SliceTest, EqualToNull) {
  std::shared_ptr<Builder> b1 = Parser::fromJson("null");
  Slice s1 = b1->slice();
  std::shared_ptr<Builder> b2 = Parser::fromJson("null");
  Slice s2 = b2->slice();

  ASSERT_TRUE(std::equal_to<Slice>()(s1, s2));
}

TEST(SliceTest, EqualToInt) {
  std::shared_ptr<Builder> b1 = Parser::fromJson("-128885355");
  Slice s1 = b1->slice();
  std::shared_ptr<Builder> b2 = Parser::fromJson("-128885355");
  Slice s2 = b2->slice();

  ASSERT_TRUE(std::equal_to<Slice>()(s1, s2));
}

TEST(SliceTest, EqualToUInt) {
  std::shared_ptr<Builder> b1 = Parser::fromJson("128885355");
  Slice s1 = b1->slice();
  std::shared_ptr<Builder> b2 = Parser::fromJson("128885355");
  Slice s2 = b2->slice();

  ASSERT_TRUE(std::equal_to<Slice>()(s1, s2));
}

TEST(SliceTest, EqualToDouble) {
  std::shared_ptr<Builder> b1 = Parser::fromJson("-128885355.353");
  Slice s1 = b1->slice();
  std::shared_ptr<Builder> b2 = Parser::fromJson("-128885355.353");
  Slice s2 = b2->slice();

  ASSERT_TRUE(std::equal_to<Slice>()(s1, s2));
}

TEST(SliceTest, EqualToString) {
  std::shared_ptr<Builder> b1 = Parser::fromJson("\"this is a test string\"");
  Slice s1 = b1->slice();
  std::shared_ptr<Builder> b2 = Parser::fromJson("\"this is a test string\"");
  Slice s2 = b2->slice();

  ASSERT_TRUE(std::equal_to<Slice>()(s1, s2));
}

TEST(SliceTest, EqualToDirectInvocation) {
  std::string const value("[1024,1025,1026,1027,1028]");

  Parser parser;
  parser.parse(value);

  int comparisons = 0;
  std::equal_to<Slice> comparer;
  ArrayIterator it(Slice(parser.start()));
  while (it.valid()) {
    ArrayIterator it2(Slice(parser.start()));
    while (it2.valid()) {
      if (it.index() != it2.index()) {
        ASSERT_FALSE(comparer(it.value(), it2.value()));
        ++comparisons;
      }
      it2.next();
    }
    it.next();
  }
  ASSERT_EQ(20, comparisons);
}

TEST(SliceTest, EqualToDirectInvocationSmallInts) {
  std::string const value("[1,2,3,4,5]");

  Parser parser;
  parser.parse(value);

  int comparisons = 0;
  std::equal_to<Slice> comparer;
  ArrayIterator it(Slice(parser.start()));
  while (it.valid()) {
    ArrayIterator it2(Slice(parser.start()));
    while (it2.valid()) {
      if (it.index() != it2.index()) {
        ASSERT_FALSE(comparer(it.value(), it2.value()));
        ++comparisons;
      }
      it2.next();
    }
    it.next();
  }
  ASSERT_EQ(20, comparisons);
}

TEST(SliceTest, EqualToDirectInvocationLongStrings) {
  std::shared_ptr<Builder> b1 = Parser::fromJson(
      "\"thisisalongstring.dddddddddddddddddddddddddddds......................."
      "....................................................."
      "longerthan127chars\"");
  std::shared_ptr<Builder> b2 = Parser::fromJson(
      "\"thisisalongstring.dddddddddddddddddddddddddddds.................eek!.."
      "........................................................."
      "longerthan127chars\"");

  std::equal_to<Slice> comparer;
  ASSERT_TRUE(comparer(b1->slice(), b1->slice()));
  ASSERT_TRUE(comparer(b2->slice(), b2->slice()));
  ASSERT_FALSE(comparer(b1->slice(), b2->slice()));
  ASSERT_FALSE(comparer(b2->slice(), b1->slice()));
}

TEST(SliceTest, HashNull) {
  std::shared_ptr<Builder> b = Parser::fromJson("null");
  Slice s = b->slice();

  ASSERT_EQ(15292542490648858194ULL, s.hash());
}

TEST(SliceTest, HashDouble) {
  std::shared_ptr<Builder> b = Parser::fromJson("-345354.35532352");
  Slice s = b->slice();

  ASSERT_EQ(8711156443018077288ULL, s.hash());
}

TEST(SliceTest, HashString) {
  std::shared_ptr<Builder> b = Parser::fromJson("\"this is a test string\"");
  Slice s = b->slice();

  ASSERT_EQ(16298643255475496611ULL, s.hash());
}

TEST(SliceTest, HashStringEmpty) {
  std::shared_ptr<Builder> b = Parser::fromJson("\"\"");
  Slice s = b->slice();

  ASSERT_EQ(5324680019219065241ULL, s.hash());
}

TEST(SliceTest, HashStringShort) {
  std::shared_ptr<Builder> b = Parser::fromJson("\"123456\"");
  Slice s = b->slice();

  ASSERT_EQ(13345050106135537218ULL, s.hash());
}

TEST(SliceTest, HashArray) {
  std::shared_ptr<Builder> b = Parser::fromJson("[1,2,3,4,5,6,7,8,9,10]");
  Slice s = b->slice();

  ASSERT_EQ(1515761289406454211ULL, s.hash());
}

TEST(SliceTest, HashObject) {
  std::shared_ptr<Builder> b = Parser::fromJson(
      "{\"one\":1,\"two\":2,\"three\":3,\"four\":4,\"five\":5,\"six\":6,"
      "\"seven\":7}");
  Slice s = b->slice();

  ASSERT_EQ(6865527808070733846ULL, s.hash());
}

TEST(SliceTest, GetNumericValueIntNoLoss) {
  Builder b;
  b.add(Value(ValueType::Array));
  b.add(Value(1));
  b.add(Value(-1));
  b.add(Value(10));
  b.add(Value(-10));
  b.add(Value(INT64_MAX));
  b.add(Value(-3453.32));
  b.add(Value(2343323453.3232235));
  b.close();

  Slice s = Slice(b.start());

  ASSERT_EQ(1, s.at(0).getNumber<int64_t>());
  ASSERT_EQ(-1, s.at(1).getNumber<int64_t>());
  ASSERT_EQ(10, s.at(2).getNumber<int64_t>());
  ASSERT_EQ(-10, s.at(3).getNumber<int64_t>());
  ASSERT_EQ(INT64_MAX, s.at(4).getNumber<int64_t>());
  ASSERT_EQ(-3453, s.at(5).getNumber<int64_t>());
  ASSERT_EQ(2343323453, s.at(6).getNumber<int64_t>());

  ASSERT_EQ(1, s.at(0).getNumber<int16_t>());
  ASSERT_EQ(-1, s.at(1).getNumber<int16_t>());
  ASSERT_EQ(10, s.at(2).getNumber<int16_t>());
  ASSERT_EQ(-10, s.at(3).getNumber<int16_t>());
  ASSERT_VELOCYPACK_EXCEPTION(s.at(4).getNumber<int16_t>(),
                              Exception::NumberOutOfRange);
  ASSERT_EQ(-3453, s.at(5).getNumber<int16_t>());
  ASSERT_VELOCYPACK_EXCEPTION(s.at(6).getNumber<int16_t>(),
                              Exception::NumberOutOfRange);
}

TEST(SliceTest, GetNumericValueUIntNoLoss) {
  Builder b;
  b.add(Value(ValueType::Array));
  b.add(Value(1));
  b.add(Value(-1));
  b.add(Value(10));
  b.add(Value(-10));
  b.add(Value(INT64_MAX));
  b.add(Value(-3453.32));
  b.add(Value(2343323453.3232235));
  b.close();

  Slice s = Slice(b.start());

  ASSERT_EQ(1ULL, s.at(0).getNumber<uint64_t>());
  ASSERT_VELOCYPACK_EXCEPTION(s.at(1).getNumber<uint64_t>(),
                              Exception::NumberOutOfRange);
  ASSERT_EQ(10ULL, s.at(2).getNumber<uint64_t>());
  ASSERT_VELOCYPACK_EXCEPTION(s.at(3).getNumber<uint64_t>(),
                              Exception::NumberOutOfRange);
  ASSERT_EQ(static_cast<uint64_t>(INT64_MAX), s.at(4).getNumber<uint64_t>());
  ASSERT_VELOCYPACK_EXCEPTION(s.at(5).getNumber<uint64_t>(),
                              Exception::NumberOutOfRange);
  ASSERT_EQ(2343323453ULL, s.at(6).getNumber<uint64_t>());

  ASSERT_EQ(1ULL, s.at(0).getNumber<uint16_t>());
  ASSERT_VELOCYPACK_EXCEPTION(s.at(1).getNumber<uint16_t>(),
                              Exception::NumberOutOfRange);
  ASSERT_EQ(10ULL, s.at(2).getNumber<uint16_t>());
  ASSERT_VELOCYPACK_EXCEPTION(s.at(3).getNumber<uint16_t>(),
                              Exception::NumberOutOfRange);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(4).getNumber<uint16_t>(),
                              Exception::NumberOutOfRange);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(5).getNumber<uint16_t>(),
                              Exception::NumberOutOfRange);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(6).getNumber<uint16_t>(),
                              Exception::NumberOutOfRange);
}

TEST(SliceTest, GetNumericValueDoubleNoLoss) {
  Builder b;
  b.add(Value(ValueType::Array));
  b.add(Value(1));
  b.add(Value(-1));
  b.add(Value(10));
  b.add(Value(-10));
  b.add(Value(INT64_MAX));
  b.add(Value(-3453.32));
  b.add(Value(2343323453.3232235));
  b.add(Value(static_cast<uint64_t>(10000)));
  b.close();

  Slice s = Slice(b.start());

  ASSERT_DOUBLE_EQ(1., s.at(0).getNumber<double>());
  ASSERT_DOUBLE_EQ(-1., s.at(1).getNumber<double>());
  ASSERT_DOUBLE_EQ(10., s.at(2).getNumber<double>());
  ASSERT_DOUBLE_EQ(-10., s.at(3).getNumber<double>());
  ASSERT_DOUBLE_EQ(static_cast<double>(INT64_MAX), s.at(4).getNumber<double>());
  ASSERT_DOUBLE_EQ(-3453.32, s.at(5).getNumber<double>());
  ASSERT_DOUBLE_EQ(2343323453.3232235, s.at(6).getNumber<double>());
  ASSERT_DOUBLE_EQ(10000., s.at(7).getNumber<double>());
}

TEST(SliceTest, GetNumericValueWrongSource) {
  Builder b;
  b.add(Value(ValueType::Array));
  b.add(Value(ValueType::Null));
  b.add(Value(true));
  b.add(Value("foo"));
  b.add(Value("bar"));
  b.add(Value(ValueType::Array));
  b.close();
  b.add(Value(ValueType::Object));
  b.close();
  b.close();

  Slice s = Slice(b.start());

  ASSERT_VELOCYPACK_EXCEPTION(s.at(0).getNumber<int64_t>(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(0).getNumber<uint64_t>(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(0).getNumber<double>(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(1).getNumber<int64_t>(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(1).getNumber<uint64_t>(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(1).getNumber<double>(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(2).getNumber<int64_t>(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(2).getNumber<uint64_t>(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(2).getNumber<double>(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(3).getNumber<int64_t>(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(3).getNumber<uint64_t>(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(3).getNumber<double>(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(4).getNumber<int64_t>(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(4).getNumber<uint64_t>(),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(4).getNumber<double>(),
                              Exception::InvalidValueType);
}

TEST(SliceTest, Translations) {
  std::unique_ptr<AttributeTranslator> translator(new AttributeTranslator);

  translator->add("foo", 1);
  translator->add("bar", 2);
  translator->add("baz", 3);
  translator->add("bark", 4);
  translator->add("mötör", 5);
  translator->add("quetzalcoatl", 6);
  translator->seal();

  Options options;
  Builder b(&options);
  options.sortAttributeNames = false;
  options.attributeTranslator = translator.get();

  b.add(Value(ValueType::Object));
  b.add("foo", Value(true));
  b.add("bar", Value(false));
  b.add("baz", Value(1));
  b.add("bart", Value(2));
  b.add("bark", Value(42));
  b.add("mötör", Value(19));
  b.add("mötörhead", Value(20));
  b.add("quetzal", Value(21));
  b.close();

  Slice s = Slice(b.start(), &options);

  ASSERT_EQ(8UL, s.length());
  ASSERT_TRUE(s.hasKey("foo"));
  ASSERT_TRUE(s.get("foo").getBoolean());
  ASSERT_TRUE(s.hasKey("bar"));
  ASSERT_FALSE(s.get("bar").getBoolean());
  ASSERT_TRUE(s.hasKey("baz"));
  ASSERT_EQ(1UL, s.get("baz").getUInt());
  ASSERT_TRUE(s.hasKey("bart"));
  ASSERT_EQ(2UL, s.get("bart").getUInt());
  ASSERT_TRUE(s.hasKey("bark"));
  ASSERT_EQ(42UL, s.get("bark").getUInt());
  ASSERT_TRUE(s.hasKey("mötör"));
  ASSERT_EQ(19UL, s.get("mötör").getUInt());
  ASSERT_TRUE(s.hasKey("mötörhead"));
  ASSERT_EQ(20UL, s.get("mötörhead").getUInt());
  ASSERT_TRUE(s.hasKey("quetzal"));
  ASSERT_EQ(21UL, s.get("quetzal").getUInt());

  ASSERT_EQ("foo", s.keyAt(0).copyString());
  ASSERT_EQ("bar", s.keyAt(1).copyString());
  ASSERT_EQ("baz", s.keyAt(2).copyString());
  ASSERT_EQ("bart", s.keyAt(3).copyString());
  ASSERT_EQ("bark", s.keyAt(4).copyString());
  ASSERT_EQ("mötör", s.keyAt(5).copyString());
  ASSERT_EQ("mötörhead", s.keyAt(6).copyString());
  ASSERT_EQ("quetzal", s.keyAt(7).copyString());
}

TEST(SliceTest, TranslationsSingleMemberObject) {
  std::unique_ptr<AttributeTranslator> translator(new AttributeTranslator);

  translator->add("foo", 1);
  translator->seal();

  Options options;
  Builder b(&options);
  options.attributeTranslator = translator.get();

  b.add(Value(ValueType::Object));
  b.add("foo", Value(true));
  b.close();

  Slice s = Slice(b.start(), &options);

  ASSERT_EQ(1UL, s.length());
  ASSERT_TRUE(s.hasKey("foo"));
  ASSERT_TRUE(s.get("foo").getBoolean());

  ASSERT_FALSE(s.hasKey("bar"));
  ASSERT_TRUE(s.get("bar").isNone());
}

TEST(SliceTest, TranslationsSubObjects) {
  std::unique_ptr<AttributeTranslator> translator(new AttributeTranslator);

  translator->add("foo", 1);
  translator->add("bar", 2);
  translator->add("baz", 3);
  translator->add("bark", 4);
  translator->seal();

  Options options;
  options.sortAttributeNames = false;
  options.attributeTranslator = translator.get();

  Builder b(&options);
  b.add(Value(ValueType::Object));
  b.add("foo", Value(ValueType::Object));
  b.add("bar", Value(false));
  b.add("baz", Value(1));
  b.add("bark", Value(ValueType::Object));
  b.add("foo", Value(2));
  b.add("bar", Value(3));
  b.close();
  b.close();
  b.add("bar", Value(4));
  b.add("bark", Value(ValueType::Object));
  b.add("foo", Value(5));
  b.add("bar", Value(6));
  b.close();
  b.close();

  Slice s = Slice(b.start(), &options);

  ASSERT_EQ(3UL, s.length());
  ASSERT_TRUE(s.hasKey("foo"));
  ASSERT_TRUE(s.get("foo").isObject());
  ASSERT_FALSE(s.get(std::vector<std::string>({"foo", "bar"})).getBoolean());
  ASSERT_EQ(1UL, s.get(std::vector<std::string>({"foo", "baz"})).getUInt());
  ASSERT_TRUE(s.get(std::vector<std::string>({"foo", "bark"})).isObject());
  ASSERT_EQ(2UL,
            s.get(std::vector<std::string>({"foo", "bark", "foo"})).getUInt());
  ASSERT_EQ(3UL,
            s.get(std::vector<std::string>({"foo", "bark", "bar"})).getUInt());
  ASSERT_TRUE(s.hasKey("bar"));
  ASSERT_EQ(4UL, s.get("bar").getUInt());
  ASSERT_TRUE(s.hasKey("bark"));
  ASSERT_TRUE(s.get("bark").isObject());
  ASSERT_EQ(5UL, s.get(std::vector<std::string>({"bark", "foo"})).getUInt());
  ASSERT_EQ(6UL, s.get(std::vector<std::string>({"bark", "bar"})).getUInt());
  ASSERT_EQ("foo", s.keyAt(0).copyString());
  ASSERT_EQ("bar", s.valueAt(0).keyAt(0).copyString());
  ASSERT_EQ("baz", s.valueAt(0).keyAt(1).copyString());
  ASSERT_EQ("bark", s.valueAt(0).keyAt(2).copyString());
  ASSERT_EQ("bar", s.keyAt(1).copyString());
  ASSERT_EQ("bark", s.keyAt(2).copyString());
  ASSERT_EQ("foo", s.valueAt(2).keyAt(0).copyString());
  ASSERT_EQ("bar", s.valueAt(2).keyAt(1).copyString());
}

TEST(SliceTest, TranslatedObjectWithoutTranslator) {
  std::unique_ptr<AttributeTranslator> translator(new AttributeTranslator);

  translator->add("foo", 1);
  translator->add("bar", 2);
  translator->add("baz", 3);
  translator->seal();

  Options options;
  Builder b(&options);
  options.sortAttributeNames = false;
  options.attributeTranslator = translator.get();

  b.add(Value(ValueType::Object));
  b.add("mötör", Value(1));
  b.add("quetzal", Value(2));
  b.add("foo", Value(3));
  b.add("bar", Value(4));
  b.add("baz", Value(5));
  b.add("bart", Value(6));
  b.close();

  Slice s = Slice(b.start());

  ASSERT_EQ(6UL, s.length());
  ASSERT_EQ("mötör", s.keyAt(0).copyString());
  ASSERT_EQ("quetzal", s.keyAt(1).copyString());
  ASSERT_VELOCYPACK_EXCEPTION(s.keyAt(2).copyString(),
                              Exception::NeedAttributeTranslator);
  ASSERT_VELOCYPACK_EXCEPTION(s.keyAt(3).copyString(),
                              Exception::NeedAttributeTranslator);
  ASSERT_VELOCYPACK_EXCEPTION(s.keyAt(4).copyString(),
                              Exception::NeedAttributeTranslator);
  ASSERT_EQ("bart", s.keyAt(5).copyString());
}

TEST(SliceTest, TranslatedWithCompactNotation) {
  std::unique_ptr<AttributeTranslator> translator(new AttributeTranslator);

  translator->add("foo", 1);
  translator->add("bar", 2);
  translator->add("baz", 3);
  translator->add("bart", 4);
  translator->add("bark", 5);
  translator->seal();

  Options options;
  Builder b(&options);
  options.sortAttributeNames = false;
  options.buildUnindexedObjects = true;
  options.attributeTranslator = translator.get();

  b.add(Value(ValueType::Object));
  b.add("foo", Value(1));
  b.add("bar", Value(2));
  b.add("baz", Value(3));
  b.add("bark", Value(4));
  b.add("bart", Value(5));
  b.close();

  Slice s = Slice(b.start(), &options);
  ASSERT_EQ(0x14, s.head());

  ASSERT_EQ(5UL, s.length());
  ASSERT_EQ("foo", s.keyAt(0).copyString());
  ASSERT_EQ("bar", s.keyAt(1).copyString());
  ASSERT_EQ("baz", s.keyAt(2).copyString());
  ASSERT_EQ("bark", s.keyAt(3).copyString());
  ASSERT_EQ("bart", s.keyAt(4).copyString());
}

TEST(SliceTest, TranslatedInvalidKey) {
  std::unique_ptr<AttributeTranslator> translator(new AttributeTranslator);

  translator->add("foo", 1);
  translator->seal();

  Options options;
  options.sortAttributeNames = false;
  options.attributeTranslator = translator.get();

  // a compact object with a single member (key: 4, value: false)
  uint8_t const data[] = {0x14, 0x05, 0x34, 0x19, 0x01};

  Slice s = Slice(data, &options);

  ASSERT_EQ(1UL, s.length());
  ASSERT_VELOCYPACK_EXCEPTION(s.keyAt(0).copyString(), Exception::KeyNotFound);
  ASSERT_VELOCYPACK_EXCEPTION(Collection::keys(s), Exception::KeyNotFound);
}

TEST(SliceTest, SliceScope) {
  SliceScope scope;

  Slice a;
  Slice b;
  {
    a = Slice::fromJson(scope, "\"foobarbazsomevalue\"");
    {
      b = Slice::fromJson(scope,
                          "\"some longer string that hopefully requires a "
                          "dynamic memory allocation and that hopefully "
                          "survives even if the Slice object itself goes out "
                          "of scope - if it does not survive, this test will "
                          "reveal it. ready? let's check it!\"");
    }
    // overwrite stack
    Slice c(Slice::fromJson(
        scope, "\"012345678901234567890123456789012345678901234567\""));
    ASSERT_TRUE(c.isString());
  }

  ASSERT_TRUE(a.isString());
  ASSERT_EQ("foobarbazsomevalue", a.copyString());

  ASSERT_TRUE(b.isString());
  ASSERT_EQ(
      "some longer string that hopefully requires a dynamic memory allocation "
      "and that hopefully survives even if the Slice object itself goes out of "
      "scope - if it does not survive, this test will reveal it. ready? let's "
      "check it!",
      b.copyString());
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
