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
 
  Options options;
  options.prettyPrint = true; 
  std::string prettyResult = Dumper::toString(s, options);
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

  Options options;
  options.prettyPrint = true; 
  std::string prettyResult = Dumper::toString(s, options);
  ASSERT_EQ(std::string("{\n  \"foo\" : \"bar\"\n}"), prettyResult);
}

TEST(PrettyDumperTest, ComplexObject) {
  std::string const value("{\"foo\":\"bar\",\"baz\":[1,2,3,[4]],\"bark\":[{\"troet\\nmann\":1,\"mötör\":[2,3.4,-42.5,true,false,null,\"some\\nstring\"]}]}");

  Parser parser;
  parser.options.sortAttributeNames = false;
  parser.parse(value);

  Builder builder = parser.steal();
  Slice s(builder.start());

  Options options;
  options.prettyPrint = true; 
  std::string result = Dumper::toString(s, options);
  ASSERT_EQ(std::string("{\n  \"foo\" : \"bar\",\n  \"baz\" : [\n    1,\n    2,\n    3,\n    [\n      4\n    ]\n  ],\n  \"bark\" : [\n    {\n      \"troet\\nmann\" : 1,\n      \"mötör\" : [\n        2,\n        3.4,\n        -42.5,\n        true,\n        false,\n        null,\n        \"some\\nstring\"\n      ]\n    }\n  ]\n}"), result);
}

TEST(StreamDumperTest, SimpleObject) {
  std::string const value("{\"foo\":\"bar\"}");

  Parser parser;
  parser.parse(value);

  Builder builder = parser.steal();
  Slice s(builder.start());
  
  Options options;
  options.prettyPrint = true; 
  std::ostringstream result;
  StreamSink<decltype(result)> sink(&result);
  Dumper dumper(&sink, options);
  dumper.dump(s);
  ASSERT_EQ(std::string("{\n  \"foo\" : \"bar\"\n}"), result.str());
}

TEST(StreamDumperTest, ComplexObject) {
  std::string const value("{\"foo\":\"bar\",\"baz\":[1,2,3,[4]],\"bark\":[{\"troet\\nmann\":1,\"mötör\":[2,3.4,-42.5,true,false,null,\"some\\nstring\"]}]}");

  Parser parser;
  parser.options.sortAttributeNames = false;
  parser.parse(value);

  Builder builder = parser.steal();
  Slice s(builder.start());

  Options options;
  options.prettyPrint = true; 
  std::ostringstream result;
  StreamSink<decltype(result)> sink(&result);
  Dumper dumper(&sink, options);
  dumper.dump(s);
  ASSERT_EQ(std::string("{\n  \"foo\" : \"bar\",\n  \"baz\" : [\n    1,\n    2,\n    3,\n    [\n      4\n    ]\n  ],\n  \"bark\" : [\n    {\n      \"troet\\nmann\" : 1,\n      \"mötör\" : [\n        2,\n        3.4,\n        -42.5,\n        true,\n        false,\n        null,\n        \"some\\nstring\"\n      ]\n    }\n  ]\n}"), result.str());
}

TEST(BufferDumperTest, Null) {
  LocalBuffer[0] = 0x18;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  CharBufferSink sink;
  Dumper dumper(&sink);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), std::string(sink.buffer.data(), sink.buffer.size()));
}

TEST(StringDumperTest, Null) {
  LocalBuffer[0] = 0x18;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  StringSink sink;
  Dumper dumper(&sink);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), sink.buffer);
}

TEST(StringDumperTest, Numbers) {
  int64_t pp = 2;
  for (int p = 1; p <= 62; p++) {
    int64_t i;

    auto check = [&] () -> void {
      Builder b;
      b.add(Value(i));
      Slice s(b.start());

      StringSink sink;
      Dumper dumper(&sink);
      dumper.dump(s);
      ASSERT_EQ(std::to_string(i), sink.buffer);
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

  CharBufferSink sink;
  Dumper dumper(&sink);
  dumper.dump(slice);
  ASSERT_EQ(std::string("false"), std::string(sink.buffer.data(), sink.buffer.size()));
}

TEST(StringDumperTest, False) {
  LocalBuffer[0] = 0x19;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  StringSink sink;
  Dumper dumper(&sink);
  dumper.dump(slice);
  ASSERT_EQ(std::string("false"), sink.buffer);
}

TEST(BufferDumperTest, True) {
  LocalBuffer[0] = 0x1a;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  CharBufferSink sink;
  Dumper dumper(&sink);
  dumper.dump(slice);
  ASSERT_EQ(std::string("true"), std::string(sink.buffer.data(), sink.buffer.size()));
}

TEST(StringDumperTest, True) {
  LocalBuffer[0] = 0x1a;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  StringSink sink;
  Dumper dumper(&sink);
  dumper.dump(slice);
  ASSERT_EQ(std::string("true"), sink.buffer);
}

TEST(StringDumperTest, CustomWithoutHandler) {
  LocalBuffer[0] = 0xf0;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  StringSink sink;
  Dumper dumper(&sink);
  ASSERT_VELOCYPACK_EXCEPTION(dumper.dump(slice), Exception::NeedCustomTypeHandler);
}

TEST(StringDumperTest, CustomWithCallback) {
  Builder b;
  b.add(Value(ValueType::Object));
  uint8_t* p = b.add("_id", ValuePair(1ULL, ValueType::Custom));
  *p = 0xf0;
  b.close();

  struct MyCustomTypeHandler : public CustomTypeHandler {
    void toJson (Slice const& value, Sink*, Slice const&) {
      ASSERT_EQ(ValueType::Custom, value.type());
      ASSERT_EQ(0xf0UL, value.head());
      sawCustom = true;
    }
    ValueLength byteSize (Slice const&) {
      EXPECT_TRUE(false);
      return 0;
    }
    bool sawCustom = false;
  };

  MyCustomTypeHandler handler;
  StringSink sink;
  Options options;
  options.customTypeHandler = &handler;
  Dumper dumper(&sink, options);
  dumper.dump(b.slice());
  ASSERT_TRUE(handler.sawCustom);
}

TEST(StringDumperTest, CustomStringWithCallback) {
  Builder b;
  b.add(Value(ValueType::Object));
  uint8_t* p = b.add("foo", ValuePair(5ULL, ValueType::Custom));
  *p++ = 0xf1;
  *p++ = 0x03;
  *p++ = 'b';
  *p++ = 'a';
  *p++ = 'r';
  b.close();

  struct MyCustomTypeHandler : public CustomTypeHandler {
    void toJson (Slice const& value, Sink* sink, Slice const&) {
      ASSERT_EQ(ValueType::Custom, value.type());
      ASSERT_EQ(0xf1UL, value.head());
      uint8_t length = *(value.start() + 1);
      sink->push_back('"');
      sink->append(value.start() + 2, length);
      sink->push_back('"');
      sawCustom = true;
    }
    ValueLength byteSize (Slice const&) {
      EXPECT_TRUE(false);
      return 0;
    }
    bool sawCustom = false;
  };

  MyCustomTypeHandler handler;
  StringSink sink;
  Options options;
  options.customTypeHandler = &handler;
  Dumper dumper(&sink, options);
  dumper.dump(b.slice());
  ASSERT_TRUE(handler.sawCustom);
  
  ASSERT_EQ(std::string("{\"foo\":\"bar\"}"), sink.buffer);
}

TEST(StringDumperTest, CustomWithCallbackWithContent) {
  Builder b;
  b.add(Value(ValueType::Object));
  uint8_t* p = b.add("_id", ValuePair(1ULL, ValueType::Custom));
  *p = 0xf0;
  b.add("_key", Value("this is a key"));
  b.close();

  struct MyCustomTypeHandler : public CustomTypeHandler {
    void toJson (Slice const& value, Sink* sink, Slice const& base) {
      ASSERT_EQ(ValueType::Custom, value.type());

      EXPECT_TRUE(base.isObject());
      auto key = base.get("_key");
      EXPECT_EQ(ValueType::String, key.type());
      sink->append("\"foobar/");
      sink->append(key.copyString());
      sink->push_back('"');
    }
    ValueLength byteSize (Slice const&) {
      EXPECT_TRUE(false);
      return 0;
    }
  };

  MyCustomTypeHandler handler;
  StringSink sink;
  Options options;
  options.customTypeHandler = &handler;
  Dumper dumper(&sink, options);
  dumper.dump(b.slice());

  ASSERT_EQ(std::string("{\"_id\":\"foobar/this is a key\",\"_key\":\"this is a key\"}"), sink.buffer);
}

TEST(StringDumperTest, ArrayWithCustom) {
  struct MyCustomTypeHandler : public CustomTypeHandler {
    int byteSizeCalled = 0;
    void toJson (Slice const& value, Sink* sink, Slice const& base) {
      ASSERT_EQ(ValueType::Custom, value.type());

      EXPECT_TRUE(base.isArray());
      if (value.head() == 0xf0) {
        sink->append("\"foobar\"");
      }
      else if (value.head() == 0xf1) {
        sink->append("1234");
      }
      else if (value.head() == 0xf2) {
        sink->append("[]");
      }
      else if (value.head() == 0xf3) {
        sink->append("{\"qux\":2}");
      }
      else {
        EXPECT_TRUE(false);
      }
    }
    ValueLength byteSize (Slice const& value) {
      EXPECT_EQ(ValueType::Custom, value.type());
      ++byteSizeCalled;
      return 1;
    }
  };

  MyCustomTypeHandler handler;
  Options options;
  options.customTypeHandler = &handler;

  uint8_t* p;

  Builder b(options);
  b.add(Value(ValueType::Array));
  p = b.add(ValuePair(1ULL, ValueType::Custom));
  *p = 0xf0;
  p = b.add(ValuePair(1ULL, ValueType::Custom));
  *p = 0xf1;
  p = b.add(ValuePair(1ULL, ValueType::Custom));
  *p = 0xf2;
  p = b.add(ValuePair(1ULL, ValueType::Custom));
  *p = 0xf3;
  b.close();
 
  // array with same sizes
  ASSERT_EQ(0x02, b.slice().head());

  StringSink sink;
  Dumper dumper(&sink, options);
  dumper.dump(b.slice());
  ASSERT_TRUE(handler.byteSizeCalled >= 4);

  ASSERT_EQ(std::string("[\"foobar\",1234,[],{\"qux\":2}]"), sink.buffer);
}

TEST(StringDumperTest, AppendCharTest) {
  StringSink sink;
  Dumper dumper(&sink);
  dumper.appendString(std::string("this is a simple string"));

  ASSERT_EQ(std::string("\"this is a simple string\""), sink.buffer);
}

TEST(StringDumperTest, AppendStringTest) {
  StringSink sink;
  Dumper dumper(&sink);
  dumper.appendString("this is a simple string");

  ASSERT_EQ(std::string("\"this is a simple string\""), sink.buffer);
}

TEST(StringDumperTest, AppendCharTestSpecialChars1) {
  StringSink sink;
  Dumper dumper(&sink);
  dumper.options.escapeForwardSlashes = true;
  dumper.appendString(std::string("this is a string with special chars / \" \\ ' foo\n\r\t baz"));

  ASSERT_EQ(std::string("\"this is a string with special chars \\/ \\\" \\\\ ' foo\\n\\r\\t baz\""), sink.buffer);
}

TEST(StringDumperTest, AppendCharTestSpecialChars2) {
  StringSink sink;
  Dumper dumper(&sink);
  dumper.options.escapeForwardSlashes = false;
  dumper.appendString(std::string("this is a string with special chars / \" \\ ' foo\n\r\t baz"));

  ASSERT_EQ(std::string("\"this is a string with special chars / \\\" \\\\ ' foo\\n\\r\\t baz\""), sink.buffer);
}

TEST(StringDumperTest, AppendStringTestSpecialChars1) {
  StringSink sink;
  Dumper dumper(&sink);
  dumper.options.escapeForwardSlashes = true;
  dumper.appendString("this is a string with special chars / \" \\ ' foo\n\r\t baz");

  ASSERT_EQ(std::string("\"this is a string with special chars \\/ \\\" \\\\ ' foo\\n\\r\\t baz\""), sink.buffer);
}

TEST(StringDumperTest, AppendStringTestSpecialChars2) {
  StringSink sink;
  Dumper dumper(&sink);
  dumper.options.escapeForwardSlashes = false;
  dumper.appendString("this is a string with special chars / \" \\ ' foo\n\r\t baz");

  ASSERT_EQ(std::string("\"this is a string with special chars / \\\" \\\\ ' foo\\n\\r\\t baz\""), sink.buffer);
}

TEST(StringDumperTest, AppendStringSlice1) {
  StringSink sink;
  Dumper dumper(&sink);

  std::string const s = "this is a string with special chars / \" \\ ' foo\n\r\t baz";
  Builder b;
  b.add(Value(s));
  Slice slice(b.start());
  dumper.options.escapeForwardSlashes = true;
  dumper.append(slice);

  ASSERT_EQ(std::string("\"this is a string with special chars \\/ \\\" \\\\ ' foo\\n\\r\\t baz\""), sink.buffer);
}

TEST(StringDumperTest, AppendStringSlice2) {
  StringSink sink;
  Dumper dumper(&sink);

  std::string const s = "this is a string with special chars / \" \\ ' foo\n\r\t baz";
  Builder b;
  b.add(Value(s));
  Slice slice(b.start());

  dumper.options.escapeForwardSlashes = false;
  dumper.append(slice);
  ASSERT_EQ(std::string("\"this is a string with special chars / \\\" \\\\ ' foo\\n\\r\\t baz\""), sink.buffer);
}

TEST(StringDumperTest, AppendStringSliceRef1) {
  StringSink sink;
  Dumper dumper(&sink);

  std::string const s = "this is a string with special chars / \" \\ ' foo\n\r\t baz";
  Builder b;
  b.add(Value(s));
  Slice slice(b.start());
  dumper.options.escapeForwardSlashes = true;
  dumper.append(&slice);

  ASSERT_EQ(std::string("\"this is a string with special chars \\/ \\\" \\\\ ' foo\\n\\r\\t baz\""), sink.buffer);
}

TEST(StringDumperTest, AppendStringSliceRef2) {
  StringSink sink;
  Dumper dumper(&sink);

  std::string const s = "this is a string with special chars / \" \\ ' foo\n\r\t baz";
  Builder b;
  b.add(Value(s));
  Slice slice(b.start());
  dumper.options.escapeForwardSlashes = false;
  dumper.append(&slice);
  ASSERT_EQ(std::string("\"this is a string with special chars / \\\" \\\\ ' foo\\n\\r\\t baz\""), sink.buffer);
}

TEST(StringDumperTest, UnsupportedTypeDoubleMinusInf) {
  double v = -3.33e307;
  v *= -v;
  Builder b;
  b.add(Value(v));

  Slice slice = b.slice();

  StringSink sink;
  Dumper dumper(&sink);
  ASSERT_VELOCYPACK_EXCEPTION(dumper.dump(slice), Exception::NoJsonEquivalent);
}

TEST(StringDumperTest, ConvertTypeDoubleMinusInf) {
  double v = -3.33e307;
  v *= -v;
  Builder b;
  b.add(Value(v));

  Slice slice = b.slice();

  Options options;
  options.unsupportedTypeBehavior = NullifyUnsupportedType;
  StringSink sink;
  Dumper dumper(&sink, options);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), sink.buffer);
}

TEST(StringDumperTest, UnsupportedTypeDoublePlusInf) {
  double v = 3.33e307;
  v *= v;
  Builder b;
  b.add(Value(v));

  Slice slice = b.slice();

  StringSink sink;
  Dumper dumper(&sink);
  ASSERT_VELOCYPACK_EXCEPTION(dumper.dump(slice), Exception::NoJsonEquivalent);
}

TEST(StringDumperTest, ConvertTypeDoublePlusInf) {
  double v = 3.33e307;
  v *= v;
  Builder b;
  b.add(Value(v));

  Slice slice = b.slice();

  Options options;
  options.unsupportedTypeBehavior = NullifyUnsupportedType;
  StringSink sink;
  Dumper dumper(&sink, options);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), sink.buffer);
}

TEST(StringDumperTest, UnsupportedTypeDoubleNan) {
  double v = std::nan("1");
  ASSERT_TRUE(std::isnan(v));
  Builder b;
  b.add(Value(v));

  Slice slice = b.slice();

  StringSink sink;
  Dumper dumper(&sink);
  ASSERT_VELOCYPACK_EXCEPTION(dumper.dump(slice), Exception::NoJsonEquivalent);
}

TEST(StringDumperTest, ConvertTypeDoubleNan) {
  double v = std::nan("1");
  ASSERT_TRUE(std::isnan(v));
  Builder b;
  b.add(Value(v));

  Slice slice = b.slice();

  Options options;
  options.unsupportedTypeBehavior = NullifyUnsupportedType;
  StringSink sink;
  Dumper dumper(&sink, options);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), sink.buffer);
}

TEST(StringDumperTest, UnsupportedTypeBinary) {
  Builder b;
  b.add(Value(std::string("der fuchs"), ValueType::Binary));

  Slice slice = b.slice();

  StringSink sink;
  Dumper dumper(&sink);
  ASSERT_VELOCYPACK_EXCEPTION(dumper.dump(slice), Exception::NoJsonEquivalent);
}

TEST(StringDumperTest, ConvertTypeBinary) {
  Builder b;
  b.add(Value(std::string("der fuchs"), ValueType::Binary));

  Slice slice = b.slice();

  Options options;
  options.unsupportedTypeBehavior = NullifyUnsupportedType;
  StringSink sink;
  Dumper dumper(&sink, options);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), sink.buffer);
}

TEST(StringDumperTest, UnsupportedTypeUTCDate) {
  int64_t v = 0;
  Builder b;
  b.add(Value(v, ValueType::UTCDate));

  Slice slice = b.slice();

  StringSink sink;
  Dumper dumper(&sink);
  ASSERT_VELOCYPACK_EXCEPTION(dumper.dump(slice), Exception::NoJsonEquivalent);
}

TEST(StringDumperTest, ConvertTypeUTCDate) {
  int64_t v = 0;
  Builder b;
  b.add(Value(v, ValueType::UTCDate));

  Slice slice = b.slice();

  Options options;
  options.unsupportedTypeBehavior = NullifyUnsupportedType;
  StringSink sink;
  Dumper dumper(&sink, options);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), sink.buffer);
}

int main (int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

