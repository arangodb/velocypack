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

TEST(DumperTest, CreateWithoutOptions) {
  ASSERT_VELOCYPACK_EXCEPTION(new Dumper(nullptr), Exception::InternalError);

  std::string result;
  StringSink sink(&result);
  ASSERT_VELOCYPACK_EXCEPTION(new Dumper(&sink, nullptr),
                              Exception::InternalError);

  ASSERT_VELOCYPACK_EXCEPTION(new Dumper(nullptr, nullptr),
                              Exception::InternalError);
}

TEST(DumperTest, InvokeOnSlice) {
  LocalBuffer[0] = 0x18;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), buffer);
}

TEST(DumperTest, InvokeOnSlicePointer) {
  LocalBuffer[0] = 0x18;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  dumper.dump(&slice);
  ASSERT_EQ(std::string("null"), buffer);
}

TEST(SinkTest, CharBufferAppenders) {
  Buffer<char> buffer;
  CharBufferSink sink(&buffer);
  sink.push_back('1');
  ASSERT_EQ(1UL, buffer.length());
  ASSERT_EQ(0, memcmp("1", buffer.data(), buffer.length()));

  sink.append(std::string("abcdef"));
  ASSERT_EQ(7UL, buffer.length());
  ASSERT_EQ(0, memcmp("1abcdef", buffer.data(), buffer.length()));

  sink.append("foobar", strlen("foobar"));
  ASSERT_EQ(13UL, buffer.length());
  ASSERT_EQ(0, memcmp("1abcdeffoobar", buffer.data(), buffer.length()));

  sink.append("quetzalcoatl");
  ASSERT_EQ(25UL, buffer.length());
  ASSERT_EQ(
      0, memcmp("1abcdeffoobarquetzalcoatl", buffer.data(), buffer.length()));

  sink.push_back('*');
  ASSERT_EQ(26UL, buffer.length());
  ASSERT_EQ(
      0, memcmp("1abcdeffoobarquetzalcoatl*", buffer.data(), buffer.length()));
}

TEST(SinkTest, StringAppenders) {
  std::string buffer;
  StringSink sink(&buffer);
  sink.push_back('1');
  ASSERT_EQ("1", buffer);

  sink.append(std::string("abcdef"));
  ASSERT_EQ("1abcdef", buffer);

  sink.append("foobar", strlen("foobar"));
  ASSERT_EQ("1abcdeffoobar", buffer);

  sink.append("quetzalcoatl");
  ASSERT_EQ("1abcdeffoobarquetzalcoatl", buffer);

  sink.push_back('*');
  ASSERT_EQ("1abcdeffoobarquetzalcoatl*", buffer);
}

TEST(SinkTest, OStreamAppenders) {
  std::ostringstream result;

  StringStreamSink sink(&result);
  sink.push_back('1');
  ASSERT_EQ("1", result.str());

  sink.append(std::string("abcdef"));
  ASSERT_EQ("1abcdef", result.str());

  sink.append("foobar", strlen("foobar"));
  ASSERT_EQ("1abcdeffoobar", result.str());

  sink.append("quetzalcoatl");
  ASSERT_EQ("1abcdeffoobarquetzalcoatl", result.str());

  sink.push_back('*');
  ASSERT_EQ("1abcdeffoobarquetzalcoatl*", result.str());
}

TEST(OutStreamTest, StringifyComplexObject) {
  Options options;
  options.sortAttributeNames = false;

  std::string const value(
      "{\"foo\":\"bar\",\"baz\":[1,2,3,[4]],\"bark\":[{\"troet\\nmann\":1,"
      "\"mötör\":[2,3.4,-42.5,true,false,null,\"some\\nstring\"]}]}");

  Parser parser(&options);
  parser.parse(value);

  std::shared_ptr<Builder> builder = parser.steal();
  Slice s(builder->start());

  std::ostringstream result;
  result << s;

  ASSERT_EQ("[Slice object (0x0f), byteSize: 107]", result.str());

  Options dumperOptions;
  dumperOptions.prettyPrint = true;
  std::string prettyResult = Dumper::toString(s, &dumperOptions);
  ASSERT_EQ(std::string(
                "{\n  \"foo\" : \"bar\",\n  \"baz\" : [\n    1,\n    2,\n    "
                "3,\n    [\n      4\n    ]\n  ],\n  \"bark\" : [\n    {\n      "
                "\"troet\\nmann\" : 1,\n      \"mötör\" : [\n        2,\n      "
                "  3.4,\n        -42.5,\n        true,\n        false,\n       "
                " null,\n        \"some\\nstring\"\n      ]\n    }\n  ]\n}"),
            prettyResult);
}

TEST(PrettyDumperTest, SimpleObject) {
  std::string const value("{\"foo\":\"bar\"}");

  Parser parser;
  parser.parse(value);

  std::shared_ptr<Builder> builder = parser.steal();
  Slice s(builder->start());

  std::ostringstream result;
  result << s;

  ASSERT_EQ("[Slice object (0x0b), byteSize: 11]", result.str());

  Options dumperOptions;
  dumperOptions.prettyPrint = true;
  std::string prettyResult = Dumper::toString(s, &dumperOptions);
  ASSERT_EQ(std::string("{\n  \"foo\" : \"bar\"\n}"), prettyResult);
}

TEST(PrettyDumperTest, ComplexObject) {
  Options options;
  options.sortAttributeNames = false;

  std::string const value(
      "{\"foo\":\"bar\",\"baz\":[1,2,3,[4]],\"bark\":[{\"troet\\nmann\":1,"
      "\"mötör\":[2,3.4,-42.5,true,false,null,\"some\\nstring\"]}]}");

  Parser parser(&options);
  parser.parse(value);

  std::shared_ptr<Builder> builder = parser.steal();
  Slice s(builder->start());

  Options dumperOptions;
  dumperOptions.prettyPrint = true;
  std::string result = Dumper::toString(s, &dumperOptions);
  ASSERT_EQ(std::string(
                "{\n  \"foo\" : \"bar\",\n  \"baz\" : [\n    1,\n    2,\n    "
                "3,\n    [\n      4\n    ]\n  ],\n  \"bark\" : [\n    {\n      "
                "\"troet\\nmann\" : 1,\n      \"mötör\" : [\n        2,\n      "
                "  3.4,\n        -42.5,\n        true,\n        false,\n       "
                " null,\n        \"some\\nstring\"\n      ]\n    }\n  ]\n}"),
            result);
}

TEST(StreamDumperTest, SimpleObject) {
  std::string const value("{\"foo\":\"bar\"}");

  Parser parser;
  parser.parse(value);

  std::shared_ptr<Builder> builder = parser.steal();
  Slice s(builder->start());

  Options options;
  options.prettyPrint = true;
  std::ostringstream result;
  StringStreamSink sink(&result);
  Dumper dumper(&sink, &options);
  dumper.dump(s);
  ASSERT_EQ(std::string("{\n  \"foo\" : \"bar\"\n}"), result.str());
}

TEST(StreamDumperTest, UseStringStreamTypedef) {
  std::string const value("{\"foo\":\"bar\"}");

  Parser parser;
  parser.parse(value);

  std::shared_ptr<Builder> builder = parser.steal();
  Slice s(builder->start());

  Options options;
  options.prettyPrint = true;
  std::ostringstream result;
  StringStreamSink sink(&result);
  Dumper dumper(&sink, &options);
  dumper.dump(s);
  ASSERT_EQ(std::string("{\n  \"foo\" : \"bar\"\n}"), result.str());
}

TEST(StreamDumperTest, ComplexObject) {
  Options options;
  options.sortAttributeNames = false;

  std::string const value(
      "{\"foo\":\"bar\",\"baz\":[1,2,3,[4]],\"bark\":[{\"troet\\nmann\":1,"
      "\"mötör\":[2,3.4,-42.5,true,false,null,\"some\\nstring\"]}]}");

  Parser parser(&options);
  parser.parse(value);

  std::shared_ptr<Builder> builder = parser.steal();
  Slice s(builder->start());

  Options dumperOptions;
  dumperOptions.prettyPrint = true;
  std::ostringstream result;
  StringStreamSink sink(&result);
  Dumper dumper(&sink, &dumperOptions);
  dumper.dump(s);
  ASSERT_EQ(std::string(
                "{\n  \"foo\" : \"bar\",\n  \"baz\" : [\n    1,\n    2,\n    "
                "3,\n    [\n      4\n    ]\n  ],\n  \"bark\" : [\n    {\n      "
                "\"troet\\nmann\" : 1,\n      \"mötör\" : [\n        2,\n      "
                "  3.4,\n        -42.5,\n        true,\n        false,\n       "
                " null,\n        \"some\\nstring\"\n      ]\n    }\n  ]\n}"),
            result.str());
}

TEST(BufferDumperTest, Null) {
  LocalBuffer[0] = 0x18;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  Buffer<char> buffer;
  CharBufferSink sink(&buffer);
  Dumper dumper(&sink);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), std::string(buffer.data(), buffer.size()));
}

TEST(StringDumperTest, Null) {
  LocalBuffer[0] = 0x18;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), buffer);
}

TEST(StringDumperTest, Numbers) {
  int64_t pp = 2;
  for (int p = 1; p <= 61; p++) {
    int64_t i;

    auto check = [&]() -> void {
      Builder b;
      b.add(Value(i));
      Slice s(b.start());

      std::string buffer;
      StringSink sink(&buffer);
      Dumper dumper(&sink);
      dumper.dump(s);
      ASSERT_EQ(std::to_string(i), buffer);
    };

    i = pp;
    check();
    i = pp + 1;
    check();
    i = pp - 1;
    check();
    i = -pp;
    check();
    i = -pp + 1;
    check();
    i = -pp - 1;
    check();

    pp *= 2;
  }
}

TEST(BufferDumperTest, False) {
  LocalBuffer[0] = 0x19;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  Buffer<char> buffer;
  CharBufferSink sink(&buffer);
  Dumper dumper(&sink);
  dumper.dump(slice);
  ASSERT_EQ(std::string("false"), std::string(buffer.data(), buffer.size()));
}

TEST(StringDumperTest, False) {
  LocalBuffer[0] = 0x19;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  dumper.dump(slice);
  ASSERT_EQ(std::string("false"), buffer);
}

TEST(BufferDumperTest, True) {
  LocalBuffer[0] = 0x1a;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  Buffer<char> buffer;
  CharBufferSink sink(&buffer);
  Dumper dumper(&sink);
  dumper.dump(slice);
  ASSERT_EQ(std::string("true"), std::string(buffer.data(), buffer.size()));
}

TEST(StringDumperTest, True) {
  LocalBuffer[0] = 0x1a;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  dumper.dump(slice);
  ASSERT_EQ(std::string("true"), buffer);
}

TEST(StringDumperTest, StringSimple) {
  Builder b;
  b.add(Value("foobar"));

  Slice slice = b.slice();
  ASSERT_EQ(std::string("\"foobar\""), Dumper::toString(slice));
}

TEST(StringDumperTest, StringSpecialChars) {
  Builder b;
  b.add(Value("\"fo\r \n \\to''\\ \\bar\""));

  Slice slice = b.slice();
  ASSERT_EQ(std::string("\"\\\"fo\\r \\n \\\\to''\\\\ \\\\bar\\\"\""),
            Dumper::toString(slice));
}

TEST(StringDumperTest, StringControlChars) {
  Builder b;
  b.add(Value(std::string("\x00\x01\x02 baz \x03", 9)));

  Slice slice = b.slice();
  ASSERT_EQ(std::string("\"\\u0000\\u0001\\u0002 baz \\u0003\""),
            Dumper::toString(slice));
}

TEST(StringDumperTest, StringUTF8) {
  Builder b;
  b.add(Value("mötör"));

  Slice slice = b.slice();
  ASSERT_EQ(std::string("\"mötör\""), Dumper::toString(slice));
}

TEST(StringDumperTest, StringTwoByteUTF8) {
  Builder b;
  b.add(Value("\xc2\xa2"));

  Slice slice = b.slice();
  ASSERT_EQ(std::string("\"\xc2\xa2\""), Dumper::toString(slice));
}

TEST(StringDumperTest, StringThreeByteUTF8) {
  Builder b;
  b.add(Value("\xe2\x82\xac"));

  Slice slice = b.slice();
  ASSERT_EQ(std::string("\"\xe2\x82\xac\""), Dumper::toString(slice));
}

TEST(StringDumperTest, StringFourByteUTF8) {
  Builder b;
  b.add(Value("\xf0\xa4\xad\xa2"));

  Slice slice = b.slice();
  ASSERT_EQ(std::string("\"\xf0\xa4\xad\xa2\""), Dumper::toString(slice));
}

TEST(StringDumperTest, NumberDoubleZero) {
  Builder b;
  b.add(Value(0.0));
  Slice slice = b.slice();

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  dumper.dump(slice);
  ASSERT_EQ(std::string("0"), buffer);
}

TEST(StringDumperTest, NumberDouble1) {
  Builder b;
  b.add(Value(123456.67));
  Slice slice = b.slice();

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  dumper.dump(slice);
  ASSERT_EQ(std::string("123456.67"), buffer);
}

TEST(StringDumperTest, NumberDouble2) {
  Builder b;
  b.add(Value(-123456.67));
  Slice slice = b.slice();

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  dumper.dump(slice);
  ASSERT_EQ(std::string("-123456.67"), buffer);
}

TEST(StringDumperTest, NumberDouble3) {
  Builder b;
  b.add(Value(-0.000442));
  Slice slice = b.slice();

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  dumper.dump(slice);
  ASSERT_EQ(std::string("-0.000442"), buffer);
}

TEST(StringDumperTest, NumberDouble4) {
  Builder b;
  b.add(Value(0.1));
  Slice slice = b.slice();

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  dumper.dump(slice);
  ASSERT_EQ(std::string("0.1"), buffer);
}

TEST(StringDumperTest, NumberDoubleScientific1) {
  Builder b;
  b.add(Value(2.41e-109));
  Slice slice = b.slice();

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  dumper.dump(slice);
  ASSERT_EQ(std::string("2.41e-109"), buffer);
}

TEST(StringDumperTest, NumberDoubleScientific2) {
  Builder b;
  b.add(Value(-3.423e78));
  Slice slice = b.slice();

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  dumper.dump(slice);
  ASSERT_EQ(std::string("-3.423e+78"), buffer);
}

TEST(StringDumperTest, NumberDoubleScientific3) {
  Builder b;
  b.add(Value(3.423e123));
  Slice slice = b.slice();

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  dumper.dump(slice);
  ASSERT_EQ(std::string("3.423e+123"), buffer);
}

TEST(StringDumperTest, NumberDoubleScientific4) {
  Builder b;
  b.add(Value(3.4239493e104));
  Slice slice = b.slice();

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  dumper.dump(slice);
  ASSERT_EQ(std::string("3.4239493e+104"), buffer);
}

TEST(StringDumperTest, NumberInt1) {
  Builder b;
  b.add(Value(static_cast<int64_t>(123456789)));
  Slice slice = b.slice();

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  dumper.dump(slice);
  ASSERT_EQ(std::string("123456789"), buffer);
}

TEST(StringDumperTest, NumberInt2) {
  Builder b;
  b.add(Value(static_cast<int64_t>(-123456789)));
  Slice slice = b.slice();

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  dumper.dump(slice);
  ASSERT_EQ(std::string("-123456789"), buffer);
}

TEST(StringDumperTest, NumberZero) {
  Builder b;
  b.add(Value(static_cast<int64_t>(0)));
  Slice slice = b.slice();

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  dumper.dump(slice);
  ASSERT_EQ(std::string("0"), buffer);
}

TEST(StringDumperTest, External) {
  Builder b1;
  b1.add(Value("this is a test string"));
  Slice slice1 = b1.slice();

  // create an external pointer to the string
  Builder b2;
  b2.add(Value(static_cast<void const*>(slice1.start())));
  Slice slice2 = b2.slice();

  ASSERT_EQ(std::string("\"this is a test string\""), Dumper::toString(slice2));
}

TEST(StringDumperTest, CustomWithoutHandler) {
  LocalBuffer[0] = 0xf0;
  LocalBuffer[1] = 0x00;

  Slice slice(reinterpret_cast<uint8_t const*>(&LocalBuffer[0]));

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  ASSERT_VELOCYPACK_EXCEPTION(dumper.dump(slice),
                              Exception::NeedCustomTypeHandler);
}

TEST(StringDumperTest, CustomWithCallbackDefaultHandler) {
  Builder b;
  b.openObject();
  uint8_t* p = b.add("_id", ValuePair(9ULL, ValueType::Custom));
  *p = 0xf3;
  for (size_t i = 1; i <= 8; i++) {
    p[i] = i + '@';
  }
  b.close();

  struct MyCustomTypeHandler : public CustomTypeHandler {};
  
  MyCustomTypeHandler handler;
  std::string buffer;
  StringSink sink(&buffer);
  Options options;
  options.customTypeHandler = &handler;
  Dumper dumper(&sink, &options);
  ASSERT_VELOCYPACK_EXCEPTION(dumper.dump(b.slice()), Exception::NotImplemented);
  
  ASSERT_VELOCYPACK_EXCEPTION(handler.dump(b.slice(), &dumper, b.slice()), Exception::NotImplemented);
  ASSERT_VELOCYPACK_EXCEPTION(handler.toString(b.slice(), nullptr, b.slice()), Exception::NotImplemented);
}

TEST(StringDumperTest, CustomWithCallback) {
  Builder b;
  b.openObject();
  uint8_t* p = b.add("_id", ValuePair(9ULL, ValueType::Custom));
  *p = 0xf3;
  for (size_t i = 1; i <= 8; i++) {
    p[i] = i + '@';
  }
  b.close();

  struct MyCustomTypeHandler : public CustomTypeHandler {
    void dump(Slice const& value, Dumper* dumper, Slice const&) override {
      ASSERT_EQ(ValueType::Custom, value.type());
      ASSERT_EQ(0xf3UL, value.head());
      sawCustom = true;
      dumper->sink()->push_back('"');
      for (size_t i = 1; i <= 8; i++) {
        dumper->sink()->push_back(value.start()[i]);
      }
      dumper->sink()->push_back('"');
    }
    bool sawCustom = false;
  };

  MyCustomTypeHandler handler;
  std::string buffer;
  StringSink sink(&buffer);
  Options options;
  options.customTypeHandler = &handler;
  Dumper dumper(&sink, &options);
  dumper.dump(b.slice());
  ASSERT_TRUE(handler.sawCustom);
  ASSERT_EQ(R"({"_id":"ABCDEFGH"})", buffer);
}

TEST(StringDumperTest, CustomStringWithCallback) {
  Builder b;
  b.add(Value(ValueType::Object));
  uint8_t* p = b.add("foo", ValuePair(5ULL, ValueType::Custom));
  *p++ = 0xf5;
  *p++ = 0x03;
  *p++ = 'b';
  *p++ = 'a';
  *p++ = 'r';
  b.close();

  struct MyCustomTypeHandler : public CustomTypeHandler {
    void dump(Slice const& value, Dumper* dumper, Slice const&) {
      Sink* sink = dumper->sink();
      ASSERT_EQ(ValueType::Custom, value.type());
      ASSERT_EQ(0xf5UL, value.head());
      ValueLength length = static_cast<ValueLength>(*(value.start() + 1));
      sink->push_back('"');
      sink->append(value.startAs<char const>() + 2, length);
      sink->push_back('"');
      sawCustom = true;
    }
    bool sawCustom = false;
  };

  MyCustomTypeHandler handler;
  std::string buffer;
  StringSink sink(&buffer);
  Options options;
  options.customTypeHandler = &handler;
  Dumper dumper(&sink, &options);
  dumper.dump(b.slice());
  ASSERT_TRUE(handler.sawCustom);

  ASSERT_EQ(std::string("{\"foo\":\"bar\"}"), buffer);
}

TEST(StringDumperTest, CustomWithCallbackWithContent) {
  struct MyCustomTypeHandler : public CustomTypeHandler {
    void dump(Slice const& value, Dumper* dumper, Slice const& base) override {
      Sink* sink = dumper->sink();
      ASSERT_EQ(ValueType::Custom, value.type());

      EXPECT_TRUE(base.isObject());
      auto key = base.get("_key");
      EXPECT_EQ(ValueType::String, key.type());
      sink->append("\"foobar/");
      sink->append(key.copyString());
      sink->push_back('"');
    }
  };

  MyCustomTypeHandler handler;
  Options options;
  options.customTypeHandler = &handler;

  Builder b(&options);
  b.add(Value(ValueType::Object));
  uint8_t* p = b.add("_id", ValuePair(2ULL, ValueType::Custom));
  *p++ = 0xf0;
  *p = 0x12;
  b.add("_key", Value("this is a key"));
  b.close();

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink, &options);
  dumper.dump(b.slice());

  ASSERT_EQ(
      std::string(
          "{\"_id\":\"foobar/this is a key\",\"_key\":\"this is a key\"}"),
      buffer);
}

TEST(StringDumperTest, ArrayWithCustom) {
  struct MyCustomTypeHandler : public CustomTypeHandler {
    void dump(Slice const& value, Dumper* dumper, Slice const& base) {
      Sink* sink = dumper->sink();
      ASSERT_EQ(ValueType::Custom, value.type());

      EXPECT_TRUE(base.isArray());
      if (value.head() == 0xf0 && value.start()[1] == 0x01) {
        sink->append("\"foobar\"");
      } else if (value.head() == 0xf0 && value.start()[1] == 0x02) {
        sink->append("1234");
      } else if (value.head() == 0xf0 && value.start()[1] == 0x03) {
        sink->append("[]");
      } else if (value.head() == 0xf0 && value.start()[1] == 0x04) {
        sink->append("{\"qux\":2}");
      } else {
        EXPECT_TRUE(false);
      }
    }
  };

  MyCustomTypeHandler handler;
  Options options;
  options.customTypeHandler = &handler;

  uint8_t* p;

  Builder b(&options);
  b.add(Value(ValueType::Array));
  p = b.add(ValuePair(2ULL, ValueType::Custom));
  *p++ = 0xf0;
  *p = 1;
  p = b.add(ValuePair(2ULL, ValueType::Custom));
  *p++ = 0xf0;
  *p = 2;
  p = b.add(ValuePair(2ULL, ValueType::Custom));
  *p++ = 0xf0;
  *p = 3;
  p = b.add(ValuePair(2ULL, ValueType::Custom));
  *p++ = 0xf0;
  *p = 4;
  b.close();

  // array with same sizes
  ASSERT_EQ(0x02, b.slice().head());

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink, &options);
  dumper.dump(b.slice());

  ASSERT_EQ(std::string("[\"foobar\",1234,[],{\"qux\":2}]"), buffer);
}

TEST(StringDumperTest, AppendCharTest) {
  char const* p = "this is a simple string";
  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  dumper.appendString(p, strlen(p));

  ASSERT_EQ(std::string("\"this is a simple string\""), buffer);
}

TEST(StringDumperTest, AppendStringTest) {
  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  dumper.appendString("this is a simple string");

  ASSERT_EQ(std::string("\"this is a simple string\""), buffer);
}

TEST(StringDumperTest, AppendCharTestSpecialChars1) {
  Options options;
  options.escapeForwardSlashes = true;

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink, &options);
  dumper.appendString(std::string(
      "this is a string with special chars / \" \\ ' foo\n\r\t baz"));

  ASSERT_EQ(std::string(
                "\"this is a string with special chars \\/ \\\" \\\\ ' "
                "foo\\n\\r\\t baz\""),
            buffer);
}

TEST(StringDumperTest, AppendCharTestSpecialChars2) {
  Options options;
  options.escapeForwardSlashes = false;

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink, &options);
  dumper.appendString(std::string(
      "this is a string with special chars / \" \\ ' foo\n\r\t baz"));

  ASSERT_EQ(std::string(
                "\"this is a string with special chars / \\\" \\\\ ' "
                "foo\\n\\r\\t baz\""),
            buffer);
}

TEST(StringDumperTest, AppendStringTestSpecialChars1) {
  Options options;
  options.escapeForwardSlashes = true;

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink, &options);
  dumper.appendString(
      "this is a string with special chars / \" \\ ' foo\n\r\t baz");

  ASSERT_EQ(std::string(
                "\"this is a string with special chars \\/ \\\" \\\\ ' "
                "foo\\n\\r\\t baz\""),
            buffer);
}

TEST(StringDumperTest, AppendStringTestSpecialChars2) {
  Options options;
  options.escapeForwardSlashes = false;

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink, &options);
  dumper.appendString(
      "this is a string with special chars / \" \\ ' foo\n\r\t baz");

  ASSERT_EQ(std::string(
                "\"this is a string with special chars / \\\" \\\\ ' "
                "foo\\n\\r\\t baz\""),
            buffer);
}

TEST(StringDumperTest, AppendStringTestTruncatedTwoByteUtf8) {
  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  ASSERT_VELOCYPACK_EXCEPTION(dumper.appendString("\xc2"),
                              Exception::InvalidUtf8Sequence);
}

TEST(StringDumperTest, AppendStringTestTruncatedThreeByteUtf8) {
  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  ASSERT_VELOCYPACK_EXCEPTION(dumper.appendString("\xe2\x82"),
                              Exception::InvalidUtf8Sequence);
}

TEST(StringDumperTest, AppendStringTestTruncatedFourByteUtf8) {
  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  ASSERT_VELOCYPACK_EXCEPTION(dumper.appendString("\xf0\xa4\xad"),
                              Exception::InvalidUtf8Sequence);
}

TEST(StringDumperTest, AppendStringSlice1) {
  Options options;
  options.escapeForwardSlashes = true;

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink, &options);

  std::string const s =
      "this is a string with special chars / \" \\ ' foo\n\r\t baz";
  Builder b;
  b.add(Value(s));
  Slice slice(b.start());
  dumper.append(slice);

  ASSERT_EQ(std::string(
                "\"this is a string with special chars \\/ \\\" \\\\ ' "
                "foo\\n\\r\\t baz\""),
            buffer);
}

TEST(StringDumperTest, AppendStringSlice2) {
  Options options;
  options.escapeForwardSlashes = false;

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink, &options);

  std::string const s =
      "this is a string with special chars / \" \\ ' foo\n\r\t baz";
  Builder b;
  b.add(Value(s));
  Slice slice(b.start());

  dumper.append(slice);
  ASSERT_EQ(std::string(
                "\"this is a string with special chars / \\\" \\\\ ' "
                "foo\\n\\r\\t baz\""),
            buffer);
}

TEST(StringDumperTest, AppendStringSliceRef1) {
  Options options;
  options.escapeForwardSlashes = true;

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink, &options);

  std::string const s =
      "this is a string with special chars / \" \\ ' foo\n\r\t baz";
  Builder b;
  b.add(Value(s));
  Slice slice(b.start());
  dumper.append(&slice);

  ASSERT_EQ(std::string(
                "\"this is a string with special chars \\/ \\\" \\\\ ' "
                "foo\\n\\r\\t baz\""),
            buffer);
}

TEST(StringDumperTest, AppendStringSliceRef2) {
  Options options;
  options.escapeForwardSlashes = false;

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink, &options);

  std::string const s =
      "this is a string with special chars / \" \\ ' foo\n\r\t baz";
  Builder b;
  b.add(Value(s));
  Slice slice(b.start());
  dumper.append(&slice);
  ASSERT_EQ(std::string(
                "\"this is a string with special chars / \\\" \\\\ ' "
                "foo\\n\\r\\t baz\""),
            buffer);
}

TEST(StringDumperTest, AppendDoubleNan) {
  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  dumper.appendDouble(std::nan("1"));
  ASSERT_EQ(std::string("NaN"), buffer);
}

TEST(StringDumperTest, AppendDoubleMinusInf) {
  double v = -3.33e307;
  // go to -inf
  v *= 3.1e90;

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  dumper.appendDouble(v);
  ASSERT_EQ(std::string("-inf"), buffer);
}

TEST(StringDumperTest, AppendDoublePlusInf) {
  double v = 3.33e307;
  // go to +inf
  v *= v;

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  dumper.appendDouble(v);
  ASSERT_EQ(std::string("inf"), buffer);
}

TEST(StringDumperTest, UnsupportedTypeDoubleMinusInf) {
  double v = -3.33e307;
  // go to -inf
  v *= 3.1e90;
  Builder b;
  b.add(Value(v));

  Slice slice = b.slice();

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  ASSERT_VELOCYPACK_EXCEPTION(dumper.dump(slice), Exception::NoJsonEquivalent);
}

TEST(StringDumperTest, ConvertTypeDoubleMinusInf) {
  double v = -3.33e307;
  // go to -inf
  v *= 3.1e90;
  Builder b;
  b.add(Value(v));

  Slice slice = b.slice();

  Options options;
  options.unsupportedTypeBehavior = Options::NullifyUnsupportedType;
  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink, &options);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), buffer);
}

TEST(StringDumperTest, UnsupportedTypeDoublePlusInf) {
  double v = 3.33e307;
  // go to +inf
  v *= v;
  Builder b;
  b.add(Value(v));

  Slice slice = b.slice();

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  ASSERT_VELOCYPACK_EXCEPTION(dumper.dump(slice), Exception::NoJsonEquivalent);
}

TEST(StringDumperTest, ConvertTypeDoublePlusInf) {
  double v = 3.33e307;
  // go to +inf
  v *= v;
  Builder b;
  b.add(Value(v));

  Slice slice = b.slice();

  Options options;
  options.unsupportedTypeBehavior = Options::NullifyUnsupportedType;
  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink, &options);
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
  StringSink sink(&buffer);
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
  options.unsupportedTypeBehavior = Options::NullifyUnsupportedType;
  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink, &options);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), buffer);
}

TEST(StringDumperTest, UnsupportedTypeBinary) {
  Builder b;
  b.add(Value(std::string("der fuchs"), ValueType::Binary));

  Slice slice = b.slice();

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  ASSERT_VELOCYPACK_EXCEPTION(dumper.dump(slice), Exception::NoJsonEquivalent);
}

TEST(StringDumperTest, ConvertTypeBinary) {
  Builder b;
  b.add(Value(std::string("der fuchs"), ValueType::Binary));

  Slice slice = b.slice();

  Options options;
  options.unsupportedTypeBehavior = Options::NullifyUnsupportedType;
  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink, &options);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), buffer);
}

TEST(StringDumperTest, UnsupportedTypeUTCDate) {
  int64_t v = 0;
  Builder b;
  b.add(Value(v, ValueType::UTCDate));

  Slice slice = b.slice();

  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink);
  ASSERT_VELOCYPACK_EXCEPTION(dumper.dump(slice), Exception::NoJsonEquivalent);
}

TEST(StringDumperTest, ConvertTypeUTCDate) {
  int64_t v = 0;
  Builder b;
  b.add(Value(v, ValueType::UTCDate));

  Slice slice = b.slice();

  Options options;
  options.unsupportedTypeBehavior = Options::NullifyUnsupportedType;
  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink, &options);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), buffer);
}

TEST(StringDumperTest, ConvertUnsupportedTypeUTCDate) {
  int64_t v = 0;
  Builder b;
  b.add(Value(v, ValueType::UTCDate));

  Slice slice = b.slice();

  Options options;
  options.unsupportedTypeBehavior = Options::ConvertUnsupportedType;
  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink, &options);
  dumper.dump(slice);
  ASSERT_EQ(std::string("\"(non-representable type utc-date)\""), buffer);
}

TEST(StringDumperTest, UnsupportedTypeNone) {
  static uint8_t const b[] = {0x00};
  Slice slice(&b[0]);

  ASSERT_VELOCYPACK_EXCEPTION(Dumper::toString(slice),
                              Exception::NoJsonEquivalent);
}

TEST(StringDumperTest, ConvertTypeNone) {
  static uint8_t const b[] = {0x00};
  Slice slice(&b[0]);

  Options options;
  options.unsupportedTypeBehavior = Options::NullifyUnsupportedType;
  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink, &options);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), buffer);
}

TEST(StringDumperTest, UnsupportedTypeIllegal) {
  static uint8_t const b[] = {0x17};
  Slice slice(&b[0]);

  ASSERT_VELOCYPACK_EXCEPTION(Dumper::toString(slice),
                              Exception::NoJsonEquivalent);
}

TEST(StringDumperTest, ConvertTypeIllegal) {
  static uint8_t const b[] = {0x17};
  Slice slice(&b[0]);

  Options options;
  options.unsupportedTypeBehavior = Options::NullifyUnsupportedType;
  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink, &options);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), buffer);
}

TEST(StringDumperTest, ConvertUnsupportedTypeIllegal) {
  static uint8_t const b[] = {0x17};
  Slice slice(&b[0]);

  Options options;
  options.unsupportedTypeBehavior = Options::ConvertUnsupportedType;
  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink, &options);
  dumper.dump(slice);
  ASSERT_EQ(std::string("\"(non-representable type illegal)\""), buffer);
}

TEST(StringDumperTest, UnsupportedTypeMinKey) {
  static uint8_t const b[] = {0x1e};
  Slice slice(&b[0]);

  ASSERT_VELOCYPACK_EXCEPTION(Dumper::toString(slice),
                              Exception::NoJsonEquivalent);
}

TEST(StringDumperTest, ConvertTypeMinKey) {
  static uint8_t const b[] = {0x1e};
  Slice slice(&b[0]);

  Options options;
  options.unsupportedTypeBehavior = Options::NullifyUnsupportedType;
  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink, &options);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), buffer);
}

TEST(StringDumperTest, UnsupportedTypeMaxKey) {
  static uint8_t const b[] = {0x1f};
  Slice slice(&b[0]);

  ASSERT_VELOCYPACK_EXCEPTION(Dumper::toString(slice),
                              Exception::NoJsonEquivalent);
}

TEST(StringDumperTest, ConvertTypeMaxKey) {
  static uint8_t const b[] = {0x1f};
  Slice slice(&b[0]);

  Options options;
  options.unsupportedTypeBehavior = Options::NullifyUnsupportedType;
  std::string buffer;
  StringSink sink(&buffer);
  Dumper dumper(&sink, &options);
  dumper.dump(slice);
  ASSERT_EQ(std::string("null"), buffer);
}

TEST(StringDumperTest, BCD) {
  static uint8_t const b[] = {0xc8, 0x00, 0x00, 0x00};  // fake BCD value
  Slice slice(&b[0]);

  ASSERT_VELOCYPACK_EXCEPTION(Dumper::toString(slice),
                              Exception::NotImplemented);
}

TEST(StringDumperTest, BCDNeg) {
  static uint8_t const b[] = {0xd0, 0x00, 0x00, 0x00};  // fake BCD value
  Slice slice(&b[0]);

  ASSERT_VELOCYPACK_EXCEPTION(Dumper::toString(slice),
                              Exception::NotImplemented);
}

TEST(StringDumperTest, AttributeTranslationsNotSet) {
  std::unique_ptr<AttributeTranslator> translator(new AttributeTranslator);
  // intentionally don't add any translations
  translator->seal();

  AttributeTranslatorScope scope(translator.get());
  
  Options options;
  options.sortAttributeNames = false;
  options.attributeTranslator = translator.get();

  std::string const value("{\"test\":\"bar\"}");

  Parser parser(&options);
  parser.parse(value);

  std::shared_ptr<Builder> builder = parser.steal();
  Slice s(builder->start());

  std::string result = Dumper::toString(s, &options);
  ASSERT_EQ(value, result);
}

TEST(StringDumperTest, AttributeTranslations) {
  std::unique_ptr<AttributeTranslator> translator(new AttributeTranslator);

  translator->add("foo", 1);
  translator->add("bar", 2);
  translator->add("baz", 3);
  translator->add("bark", 4);
  translator->add("mötör", 5);
  translator->add("quetzalcoatl", 6);
  translator->seal();

  AttributeTranslatorScope scope(translator.get());

  Options options;
  options.sortAttributeNames = false;
  options.attributeTranslator = translator.get();

  std::string const value(
      "{\"foo\":\"bar\",\"baz\":[1,2,3,[4]],\"bark\":[{\"troet\\nmann\":1,"
      "\"mötör\":[2,3.4,-42.5,true,false,null,\"some\\nstring\"]}]}");

  Parser parser(&options);
  parser.parse(value);

  std::shared_ptr<Builder> builder = parser.steal();
  Slice s(builder->start());

  std::string result = Dumper::toString(s, &options);
  ASSERT_EQ(value, result);
}

TEST(StringDumperTest, AttributeTranslationsInSubObjects) {
  std::unique_ptr<AttributeTranslator> translator(new AttributeTranslator);

  translator->add("foo", 1);
  translator->add("bar", 2);
  translator->add("baz", 3);
  translator->add("bark", 4);
  translator->seal();

  AttributeTranslatorScope scope(translator.get());

  Options options;
  options.sortAttributeNames = false;
  options.attributeTranslator = translator.get();

  std::string const value(
      "{\"foo\":{\"bar\":{\"baz\":\"baz\"},\"bark\":3,\"foo\":true},\"bar\":"
      "1}");

  Parser parser(&options);
  parser.parse(value);

  std::shared_ptr<Builder> builder = parser.steal();
  Slice s(builder->start());

  std::string result = Dumper::toString(s, &options);
  ASSERT_EQ(value, result);
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
