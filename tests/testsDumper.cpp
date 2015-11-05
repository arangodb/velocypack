////////////////////////////////////////////////////////////////////////////////
/// @brief Library to build up Jason documents.
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

static unsigned char Buffer[4096];

TEST(OutStreamTest, StringifyComplexObject) {
  std::string const value("{\"foo\":\"bar\",\"baz\":[1,2,3,[4]],\"bark\":[{\"troet\\nmann\":1,\"mötör\":[2,3.4,-42.5,true,false,null,\"some\\nstring\"]}]}");

  JasonParser parser;
  parser.options.sortAttributeNames = false;
  parser.parse(value);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());

  std::ostringstream result;
  result << s;

  ASSERT_EQ("[JasonSlice object (0x0f), byteSize: 107]", result.str());
  
  std::string prettyResult = JasonStringPrettyDumper::Dump(s);
  ASSERT_EQ(std::string("{\n  \"foo\" : \"bar\",\n  \"baz\" : [\n    1,\n    2,\n    3,\n    [\n      4\n    ]\n  ],\n  \"bark\" : [\n    {\n      \"troet\\nmann\" : 1,\n      \"mötör\" : [\n        2,\n        3.4,\n        -42.5,\n        true,\n        false,\n        null,\n        \"some\\nstring\"\n      ]\n    }\n  ]\n}"), prettyResult);
}

TEST(PrettyDumperTest, SimpleObject) {
  std::string const value("{\"foo\":\"bar\"}");

  JasonParser parser;
  parser.parse(value);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  
  std::ostringstream result;
  result << s;

  ASSERT_EQ("[JasonSlice object (0x0b), byteSize: 11]", result.str());

  std::string prettyResult = JasonStringPrettyDumper::Dump(s);
  ASSERT_EQ(std::string("{\n  \"foo\" : \"bar\"\n}"), prettyResult);
}

TEST(PrettyDumperTest, ComplexObject) {
  std::string const value("{\"foo\":\"bar\",\"baz\":[1,2,3,[4]],\"bark\":[{\"troet\\nmann\":1,\"mötör\":[2,3.4,-42.5,true,false,null,\"some\\nstring\"]}]}");

  JasonParser parser;
  parser.options.sortAttributeNames = false;
  parser.parse(value);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());

  std::string result = JasonStringPrettyDumper::Dump(s);
  ASSERT_EQ(std::string("{\n  \"foo\" : \"bar\",\n  \"baz\" : [\n    1,\n    2,\n    3,\n    [\n      4\n    ]\n  ],\n  \"bark\" : [\n    {\n      \"troet\\nmann\" : 1,\n      \"mötör\" : [\n        2,\n        3.4,\n        -42.5,\n        true,\n        false,\n        null,\n        \"some\\nstring\"\n      ]\n    }\n  ]\n}"), result);
}

TEST(BufferDumperTest, Null) {
  Buffer[0] = 0x18;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  JasonCharBuffer buffer;
  JasonBufferDumper dumper(buffer, JasonBufferDumper::StrategyFail);
  dumper.dump(slice);
  std::string output(buffer.data(), buffer.size());
  ASSERT_EQ(std::string("null"), output);
}

TEST(StringDumperTest, Null) {
  Buffer[0] = 0x18;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  std::string buffer;
  JasonStringDumper dumper(buffer, JasonStringDumper::StrategyFail);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), buffer);
}

TEST(StringDumperTest, Numbers) {
  int64_t pp = 2;
  for (int p = 1; p <= 62; p++) {
    int64_t i;

    auto check = [&] () -> void {
      JasonBuilder b;
      b.add(Jason(i));
      JasonSlice s(b.start());
      JasonCharBuffer buffer;
      JasonBufferDumper dumper(buffer, JasonBufferDumper::StrategyFail);
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
  Buffer[0] = 0x19;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  JasonCharBuffer buffer;
  JasonBufferDumper dumper(buffer, JasonBufferDumper::StrategyFail);
  dumper.dump(slice);
  std::string output(buffer.data(), buffer.size());
  ASSERT_EQ(std::string("false"), output);
}

TEST(StringDumperTest, False) {
  Buffer[0] = 0x19;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  std::string buffer;
  JasonStringDumper dumper(buffer, JasonStringDumper::StrategyFail);
  dumper.dump(slice);
  ASSERT_EQ(std::string("false"), buffer);
}

TEST(BufferDumperTest, True) {
  Buffer[0] = 0x1a;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  JasonCharBuffer buffer;
  JasonBufferDumper dumper(buffer, JasonBufferDumper::StrategyFail);
  dumper.dump(slice);
  std::string output(buffer.data(), buffer.size());
  ASSERT_EQ(std::string("true"), output);
}

TEST(StringDumperTest, True) {
  Buffer[0] = 0x1a;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  std::string buffer;
  JasonStringDumper dumper(buffer, JasonStringDumper::StrategyFail);
  dumper.dump(slice);
  ASSERT_EQ(std::string("true"), buffer);
}

TEST(StringDumperTest, CustomWithoutHandler) {
  Buffer[0] = 0xf0;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  std::string buffer;
  JasonStringDumper dumper(buffer, JasonStringDumper::StrategyFail);
  EXPECT_JASON_EXCEPTION(dumper.dump(slice), JasonException::NoJsonEquivalent);
}

TEST(StringDumperTest, CustomWithCallback) {
  JasonBuilder b;
  b.add(Jason(JasonType::Object));
  uint8_t* p = b.add("_id", JasonPair(1ULL, JasonType::Custom));
  *p = 0xf0;
  b.close();

  bool sawCustom = false;
  std::string buffer;
  JasonStringDumper dumper(buffer, JasonStringDumper::StrategyFail);
  dumper.setCallback([&] (std::string*, JasonSlice const* slice, JasonSlice const*) -> bool {
    if (slice->type() == JasonType::Custom) {
      sawCustom = true;
      return true;
    }
    return false;
  });
  dumper.dump(b.slice());
  ASSERT_TRUE(sawCustom);
}

TEST(StringDumperTest, CustomWithCallbackWithContent) {
  JasonBuilder b;
  b.add(Jason(JasonType::Object));
  uint8_t* p = b.add("_id", JasonPair(1ULL, JasonType::Custom));
  *p = 0xf0;
  b.add("_key", Jason("this is a key"));
  b.close();

  std::string buffer;
  JasonStringDumper dumper(buffer, JasonStringDumper::StrategyFail);

  dumper.setCallback([] (std::string* buffer, JasonSlice const* slice, JasonSlice const* parent) -> bool {
    if (slice->type() == JasonType::Custom) {
      EXPECT_TRUE(parent->isObject());
      auto key = parent->get("_key");
      EXPECT_EQ(JasonType::String, key.type());
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
  JasonStringDumper dumper(buffer, JasonStringDumper::StrategyFail);
  dumper.appendString(std::string("this is a simple string"));

  ASSERT_EQ(std::string("\"this is a simple string\""), buffer);
}

TEST(StringDumperTest, AppendStringTest) {
  std::string buffer;
  JasonStringDumper dumper(buffer, JasonStringDumper::StrategyFail);
  dumper.appendString("this is a simple string");

  ASSERT_EQ(std::string("\"this is a simple string\""), buffer);
}

TEST(StringDumperTest, AppendCharTestSpecialChars) {
  std::string buffer;
  JasonStringDumper dumper(buffer, JasonStringDumper::StrategyFail);
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
  JasonStringDumper dumper(buffer, JasonStringDumper::StrategyFail);
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
  JasonStringDumper dumper(buffer, JasonStringDumper::StrategyFail);

  std::string const s = "this is a string with special chars / \" \\ ' foo\n\r\t baz";
  JasonBuilder b;
  b.add(Jason(s));
  JasonSlice slice(b.start());
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
  JasonStringDumper dumper(buffer, JasonStringDumper::StrategyFail);

  std::string const s = "this is a string with special chars / \" \\ ' foo\n\r\t baz";
  JasonBuilder b;
  b.add(Jason(s));
  JasonSlice slice(b.start());
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

  JasonParser parser;
  parser.options.sortAttributeNames = false;
  parser.parse(value);

  JasonBuilder builder = parser.steal();
  JasonSlice slice(builder.start());

  std::string buffer;
  JasonStringDumper dumper(buffer, JasonStringDumper::StrategyFail);
  dumper.dump(slice);

  std::ostringstream out;
  out << dumper;
  
  ASSERT_EQ(std::string("{\"foo\":\"the quick brown fox\"}"), out.str());
}

TEST(StringDumperTest, UnsupportedTypeDoubleMinusInf) {
  double v = -3.33e307;
  v *= -v;
  JasonBuilder b;
  b.add(Jason(v));

  JasonSlice slice = b.slice();

  std::string buffer;
  JasonStringDumper dumper(buffer, JasonStringDumper::StrategyFail);
  EXPECT_JASON_EXCEPTION(dumper.dump(slice), JasonException::NoJsonEquivalent);
}

TEST(StringDumperTest, ConvertTypeDoubleMinusInf) {
  double v = -3.33e307;
  v *= -v;
  JasonBuilder b;
  b.add(Jason(v));

  JasonSlice slice = b.slice();

  std::string buffer;
  JasonStringDumper dumper(buffer, JasonStringDumper::StrategyNullify);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), buffer);
}

TEST(StringDumperTest, UnsupportedTypeDoublePlusInf) {
  double v = 3.33e307;
  v *= v;
  JasonBuilder b;
  b.add(Jason(v));

  JasonSlice slice = b.slice();

  std::string buffer;
  JasonStringDumper dumper(buffer, JasonStringDumper::StrategyFail);
  EXPECT_JASON_EXCEPTION(dumper.dump(slice), JasonException::NoJsonEquivalent);
}

TEST(StringDumperTest, ConvertTypeDoublePlusInf) {
  double v = 3.33e307;
  v *= v;
  JasonBuilder b;
  b.add(Jason(v));

  JasonSlice slice = b.slice();

  std::string buffer;
  JasonStringDumper dumper(buffer, JasonStringDumper::StrategyNullify);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), buffer);
}

TEST(StringDumperTest, UnsupportedTypeDoubleNan) {
  double v = std::nan("1");
  EXPECT_TRUE(std::isnan(v));
  JasonBuilder b;
  b.add(Jason(v));

  JasonSlice slice = b.slice();

  std::string buffer;
  JasonStringDumper dumper(buffer, JasonStringDumper::StrategyFail);
  EXPECT_JASON_EXCEPTION(dumper.dump(slice), JasonException::NoJsonEquivalent);
}

TEST(StringDumperTest, ConvertTypeDoubleNan) {
  double v = std::nan("1");
  EXPECT_TRUE(std::isnan(v));
  JasonBuilder b;
  b.add(Jason(v));

  JasonSlice slice = b.slice();

  std::string buffer;
  JasonStringDumper dumper(buffer, JasonStringDumper::StrategyNullify);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), buffer);
}

TEST(StringDumperTest, UnsupportedTypeBinary) {
  JasonBuilder b;
  b.add(Jason(std::string("der fuchs"), JasonType::Binary));

  JasonSlice slice = b.slice();

  std::string buffer;
  JasonStringDumper dumper(buffer, JasonStringDumper::StrategyFail);
  EXPECT_JASON_EXCEPTION(dumper.dump(slice), JasonException::NoJsonEquivalent);
}

TEST(StringDumperTest, ConvertTypeBinary) {
  JasonBuilder b;
  b.add(Jason(std::string("der fuchs"), JasonType::Binary));

  JasonSlice slice = b.slice();

  std::string buffer;
  JasonStringDumper dumper(buffer, JasonStringDumper::StrategyNullify);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), buffer);
}

TEST(StringDumperTest, UnsupportedTypeUTCDate) {
  int64_t v = 0;
  JasonBuilder b;
  b.add(Jason(v, JasonType::UTCDate));

  JasonSlice slice = b.slice();

  std::string buffer;
  JasonStringDumper dumper(buffer, JasonStringDumper::StrategyFail);
  EXPECT_JASON_EXCEPTION(dumper.dump(slice), JasonException::NoJsonEquivalent);
}

TEST(StringDumperTest, ConvertTypeUTCDate) {
  int64_t v = 0;
  JasonBuilder b;
  b.add(Jason(v, JasonType::UTCDate));

  JasonSlice slice = b.slice();

  std::string buffer;
  JasonStringDumper dumper(buffer, JasonStringDumper::StrategyNullify);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), buffer);
}

int main (int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

