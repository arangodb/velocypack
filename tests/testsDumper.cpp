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

TEST(OutStreamTest, StringifyComplexObject) {
  std::string const value("{\"foo\":\"bar\",\"baz\":[1,2,3,[4]],\"bark\":[{\"troet\\nmann\":1,\"mötör\":[2,3.4,-42.5,true,false,null,\"some\\nstring\"]}]}");

  Parser parser;
  parser.options.sortAttributeNames = false;
  parser.parse(value);

  Builder builder = parser.steal();
  Slice s(builder.start());

  std::ostringstream result;
  result << s;

  ASSERT_EQ("[Slice object (0x0f), byteSize: 107]", result.str());
  
  std::string prettyResult = StringPrettyDumper::Dump(s);
  ASSERT_EQ(std::string("{\n  \"foo\" : \"bar\",\n  \"baz\" : [\n    1,\n    2,\n    3,\n    [\n      4\n    ]\n  ],\n  \"bark\" : [\n    {\n      \"troet\\nmann\" : 1,\n      \"mötör\" : [\n        2,\n        3.4,\n        -42.5,\n        true,\n        false,\n        null,\n        \"some\\nstring\"\n      ]\n    }\n  ]\n}"), prettyResult);
}

TEST(PrettyDumperTest, SimpleObject) {
  std::string const value("{\"foo\":\"bar\"}");

  Parser parser;
  parser.parse(value);

  Builder builder = parser.steal();
  Slice s(builder.start());
  
  std::ostringstream result;
  result << s;

  ASSERT_EQ("[Slice object (0x0b), byteSize: 11]", result.str());

  std::string prettyResult = StringPrettyDumper::Dump(s);
  ASSERT_EQ(std::string("{\n  \"foo\" : \"bar\"\n}"), prettyResult);
}

TEST(PrettyDumperTest, ComplexObject) {
  std::string const value("{\"foo\":\"bar\",\"baz\":[1,2,3,[4]],\"bark\":[{\"troet\\nmann\":1,\"mötör\":[2,3.4,-42.5,true,false,null,\"some\\nstring\"]}]}");

  Parser parser;
  parser.options.sortAttributeNames = false;
  parser.parse(value);

  Builder builder = parser.steal();
  Slice s(builder.start());

  std::string result = StringPrettyDumper::Dump(s);
  ASSERT_EQ(std::string("{\n  \"foo\" : \"bar\",\n  \"baz\" : [\n    1,\n    2,\n    3,\n    [\n      4\n    ]\n  ],\n  \"bark\" : [\n    {\n      \"troet\\nmann\" : 1,\n      \"mötör\" : [\n        2,\n        3.4,\n        -42.5,\n        true,\n        false,\n        null,\n        \"some\\nstring\"\n      ]\n    }\n  ]\n}"), result);
}

TEST(BufferDumperTest, Null) {
  LocalBuffer[0] = 0x18;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  CharBuffer buffer;
  BufferDumper dumper(buffer);
  dumper.dump(slice);
  std::string output(buffer.data(), buffer.size());
  ASSERT_EQ(std::string("null"), output);
}

TEST(StringDumperTest, Null) {
  LocalBuffer[0] = 0x18;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  std::string buffer;
  StringDumper dumper(buffer);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), buffer);
}

TEST(StringDumperTest, Numbers) {
  int64_t pp = 2;
  for (int p = 1; p <= 62; p++) {
    int64_t i;

    auto check = [&] () -> void {
      Builder b;
      b.add(Value(i));
      Slice s(b.start());
      CharBuffer buffer;
      BufferDumper dumper(buffer);
      dumper.dump(s);
      std::string output(buffer.data(), buffer.size());
      ASSERT_EQ(std::to_string(i), output);
    };

    i = pp; check();
    i = pp + 1; check();
    i = pp - 1; check();
    i = -pp; check();
    i = -pp + 1; check();
    i = -pp - 1; check();

    pp *= 2;
  }
}

TEST(BufferDumperTest, False) {
  LocalBuffer[0] = 0x19;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  CharBuffer buffer;
  BufferDumper dumper(buffer);
  dumper.dump(slice);
  std::string output(buffer.data(), buffer.size());
  ASSERT_EQ(std::string("false"), output);
}

TEST(StringDumperTest, False) {
  LocalBuffer[0] = 0x19;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  std::string buffer;
  StringDumper dumper(buffer);
  dumper.dump(slice);
  ASSERT_EQ(std::string("false"), buffer);
}

TEST(BufferDumperTest, True) {
  LocalBuffer[0] = 0x1a;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  CharBuffer buffer;
  BufferDumper dumper(buffer);
  dumper.dump(slice);
  std::string output(buffer.data(), buffer.size());
  ASSERT_EQ(std::string("true"), output);
}

TEST(StringDumperTest, True) {
  LocalBuffer[0] = 0x1a;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  std::string buffer;
  StringDumper dumper(buffer);
  dumper.dump(slice);
  ASSERT_EQ(std::string("true"), buffer);
}

TEST(StringDumperTest, CustomWithoutHandler) {
  LocalBuffer[0] = 0xf0;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  std::string buffer;
  StringDumper dumper(buffer);
  ASSERT_VELOCYPACK_EXCEPTION(dumper.dump(slice), Exception::NoJsonEquivalent);
}

TEST(StringDumperTest, CustomWithCallback) {
  Builder b;
  b.add(Value(ValueType::Object));
  uint8_t* p = b.add("_id", ValuePair(1ULL, ValueType::Custom));
  *p = 0xf0;
  b.close();

  bool sawCustom = false;
  std::string buffer;
  StringDumper dumper(buffer);
  dumper.setCallback([&] (std::string*, Slice const* slice, Slice const*) -> bool {
    if (slice->type() == ValueType::Custom) {
      sawCustom = true;
      return true;
    }
    return false;
  });
  dumper.dump(b.slice());
  ASSERT_TRUE(sawCustom);
}

TEST(StringDumperTest, CustomWithCallbackWithContent) {
  Builder b;
  b.add(Value(ValueType::Object));
  uint8_t* p = b.add("_id", ValuePair(1ULL, ValueType::Custom));
  *p = 0xf0;
  b.add("_key", Value("this is a key"));
  b.close();

  std::string buffer;
  StringDumper dumper(buffer);

  dumper.setCallback([] (std::string* buffer, Slice const* slice, Slice const* parent) -> bool {
    if (slice->type() == ValueType::Custom) {
      EXPECT_TRUE(parent->isObject());
      auto key = parent->get("_key");
      EXPECT_EQ(ValueType::String, key.type());
      buffer->append("\"foobar/");
      buffer->append(key.copyString());
      buffer->push_back('"');
      return true;
    }
    return false;
  });
  dumper.dump(b.slice());

  ASSERT_EQ(std::string("{\"_id\":\"foobar/this is a key\",\"_key\":\"this is a key\"}"), buffer);
}

TEST(StringDumperTest, AppendCharTest) {
  std::string buffer;
  StringDumper dumper(buffer);
  dumper.appendString(std::string("this is a simple string"));

  ASSERT_EQ(std::string("\"this is a simple string\""), buffer);
}

TEST(StringDumperTest, AppendStringTest) {
  std::string buffer;
  StringDumper dumper(buffer);
  dumper.appendString("this is a simple string");

  ASSERT_EQ(std::string("\"this is a simple string\""), buffer);
}

TEST(StringDumperTest, AppendCharTestSpecialChars) {
  std::string buffer;
  StringDumper dumper(buffer);
  dumper.options.escapeForwardSlashes = true;
  dumper.appendString(std::string("this is a string with special chars / \" \\ ' foo\n\r\t baz"));

  ASSERT_EQ(std::string("\"this is a string with special chars \\/ \\\" \\\\ ' foo\\n\\r\\t baz\""), buffer);

  dumper.reset();
  dumper.options.escapeForwardSlashes = false;
  dumper.appendString(std::string("this is a string with special chars / \" \\ ' foo\n\r\t baz"));

  ASSERT_EQ(std::string("\"this is a string with special chars / \\\" \\\\ ' foo\\n\\r\\t baz\""), buffer);
}

TEST(StringDumperTest, AppendStringTestSpecialChars) {
  std::string buffer;
  StringDumper dumper(buffer);
  dumper.options.escapeForwardSlashes = true;
  dumper.appendString("this is a string with special chars / \" \\ ' foo\n\r\t baz");

  ASSERT_EQ(std::string("\"this is a string with special chars \\/ \\\" \\\\ ' foo\\n\\r\\t baz\""), buffer);

  dumper.reset();
  dumper.options.escapeForwardSlashes = false;
  dumper.appendString("this is a string with special chars / \" \\ ' foo\n\r\t baz");

  ASSERT_EQ(std::string("\"this is a string with special chars / \\\" \\\\ ' foo\\n\\r\\t baz\""), buffer);
}

TEST(StringDumperTest, AppendStringSlice) {
  std::string buffer;
  StringDumper dumper(buffer);

  std::string const s = "this is a string with special chars / \" \\ ' foo\n\r\t baz";
  Builder b;
  b.add(Value(s));
  Slice slice(b.start());
  dumper.options.escapeForwardSlashes = true;
  dumper.append(slice);

  ASSERT_EQ(std::string("\"this is a string with special chars \\/ \\\" \\\\ ' foo\\n\\r\\t baz\""), buffer);

  dumper.reset();
  dumper.options.escapeForwardSlashes = false;
  dumper.append(slice);
  ASSERT_EQ(std::string("\"this is a string with special chars / \\\" \\\\ ' foo\\n\\r\\t baz\""), buffer);
}

TEST(StringDumperTest, AppendStringSliceRef) {
  std::string buffer;
  StringDumper dumper(buffer);

  std::string const s = "this is a string with special chars / \" \\ ' foo\n\r\t baz";
  Builder b;
  b.add(Value(s));
  Slice slice(b.start());
  dumper.options.escapeForwardSlashes = true;
  dumper.append(&slice);

  ASSERT_EQ(std::string("\"this is a string with special chars \\/ \\\" \\\\ ' foo\\n\\r\\t baz\""), buffer);
  
  dumper.reset();
  dumper.options.escapeForwardSlashes = false;
  dumper.append(&slice);
  ASSERT_EQ(std::string("\"this is a string with special chars / \\\" \\\\ ' foo\\n\\r\\t baz\""), buffer);
}

TEST(StringDumperTest, AppendToOstream) {
  std::string const value("{\"foo\":\"the quick brown fox\"}");

  Parser parser;
  parser.options.sortAttributeNames = false;
  parser.parse(value);

  Builder builder = parser.steal();
  Slice slice(builder.start());

  std::string buffer;
  StringDumper dumper(buffer);
  dumper.dump(slice);

  std::ostringstream out;
  out << dumper;
  
  ASSERT_EQ(std::string("{\"foo\":\"the quick brown fox\"}"), out.str());
}

TEST(StringDumperTest, UnsupportedTypeDoubleMinusInf) {
  double v = -3.33e307;
  v *= -v;
  Builder b;
  b.add(Value(v));

  Slice slice = b.slice();

  std::string buffer;
  StringDumper dumper(buffer);
  ASSERT_VELOCYPACK_EXCEPTION(dumper.dump(slice), Exception::NoJsonEquivalent);
}

TEST(StringDumperTest, ConvertTypeDoubleMinusInf) {
  double v = -3.33e307;
  v *= -v;
  Builder b;
  b.add(Value(v));

  Slice slice = b.slice();

  std::string buffer;
  StringDumper dumper(buffer, StringDumper::StrategyNullifyUnsupportedType);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), buffer);
}

TEST(StringDumperTest, UnsupportedTypeDoublePlusInf) {
  double v = 3.33e307;
  v *= v;
  Builder b;
  b.add(Value(v));

  Slice slice = b.slice();

  std::string buffer;
  StringDumper dumper(buffer);
  ASSERT_VELOCYPACK_EXCEPTION(dumper.dump(slice), Exception::NoJsonEquivalent);
}

TEST(StringDumperTest, ConvertTypeDoublePlusInf) {
  double v = 3.33e307;
  v *= v;
  Builder b;
  b.add(Value(v));

  Slice slice = b.slice();

  std::string buffer;
  StringDumper dumper(buffer, StringDumper::StrategyNullifyUnsupportedType);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), buffer);
}

TEST(StringDumperTest, UnsupportedTypeDoubleNan) {
  double v = std::nan("1");
  ASSERT_TRUE(std::isnan(v));
  Builder b;
  b.add(Value(v));

  Slice slice = b.slice();

  std::string buffer;
  StringDumper dumper(buffer);
  ASSERT_VELOCYPACK_EXCEPTION(dumper.dump(slice), Exception::NoJsonEquivalent);
}

TEST(StringDumperTest, ConvertTypeDoubleNan) {
  double v = std::nan("1");
  ASSERT_TRUE(std::isnan(v));
  Builder b;
  b.add(Value(v));

  Slice slice = b.slice();

  std::string buffer;
  StringDumper dumper(buffer, StringDumper::StrategyNullifyUnsupportedType);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), buffer);
}

TEST(StringDumperTest, UnsupportedTypeBinary) {
  Builder b;
  b.add(Value(std::string("der fuchs"), ValueType::Binary));

  Slice slice = b.slice();

  std::string buffer;
  StringDumper dumper(buffer);
  ASSERT_VELOCYPACK_EXCEPTION(dumper.dump(slice), Exception::NoJsonEquivalent);
}

TEST(StringDumperTest, ConvertTypeBinary) {
  Builder b;
  b.add(Value(std::string("der fuchs"), ValueType::Binary));

  Slice slice = b.slice();

  std::string buffer;
  StringDumper dumper(buffer, StringDumper::StrategyNullifyUnsupportedType);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), buffer);
}

TEST(StringDumperTest, UnsupportedTypeUTCDate) {
  int64_t v = 0;
  Builder b;
  b.add(Value(v, ValueType::UTCDate));

  Slice slice = b.slice();

  std::string buffer;
  StringDumper dumper(buffer);
  ASSERT_VELOCYPACK_EXCEPTION(dumper.dump(slice), Exception::NoJsonEquivalent);
}

TEST(StringDumperTest, ConvertTypeUTCDate) {
  int64_t v = 0;
  Builder b;
  b.add(Value(v, ValueType::UTCDate));

  Slice slice = b.slice();

  std::string buffer;
  StringDumper dumper(buffer, StringDumper::StrategyNullifyUnsupportedType);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), buffer);
}

int main (int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

