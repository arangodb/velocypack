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
#include <fstream>
#include <string>

#include "Jason.h"
#include "JasonBuffer.h"
#include "JasonBuilder.h"
#include "JasonDump.h"
#include "JasonException.h"
#include "JasonOptions.h"
#include "JasonParser.h"
#include "JasonSlice.h"
#include "JasonType.h"

#include "gtest/gtest.h"

using Jason             = arangodb::jason::Jason;
using JasonCharBuffer   = arangodb::jason::JasonCharBuffer;
using JasonBuilder      = arangodb::jason::JasonBuilder;
using JasonBufferDumper = arangodb::jason::JasonBufferDumper;
using JasonPrettyDumper = arangodb::jason::JasonStringPrettyDumper;
using JasonStringDumper = arangodb::jason::JasonStringDumper;
using JasonException    = arangodb::jason::JasonException;
using JasonLength       = arangodb::jason::JasonLength;
using JasonPair         = arangodb::jason::JasonPair;
using JasonParser       = arangodb::jason::JasonParser;
using JasonSlice        = arangodb::jason::JasonSlice;
using JasonType         = arangodb::jason::JasonType;

// helper for catching Jason-specific exceptions
#define EXPECT_JASON_EXCEPTION(operation, code) \
  try {                                         \
    operation;                                  \
    EXPECT_FALSE(true);                         \
  }                                             \
  catch (JasonException const& ex) {            \
    EXPECT_EQ(code, ex.errorCode());            \
  }                                             \
  catch (...) {                                 \
    EXPECT_FALSE(true);                         \
  } 
  
static char Buffer[4096];

static void dumpDouble (double x, uint8_t* p) {
  uint64_t u;
  memcpy(&u, &x, sizeof(double));
  for (size_t i = 0; i < 8; i++) {
    p[i] = u & 0xff;
    u >>= 8;
  }
}

static std::string readFile (std::string const& filename) {
  std::string s;
  std::ifstream ifs(filename.c_str(), std::ifstream::in);

  if (! ifs.is_open()) {
    throw "cannot open input file";
  }
  
  char buffer[4096];
  while (ifs.good()) {
    ifs.read(&buffer[0], sizeof(buffer));
    s.append(buffer, ifs.gcount());
  }
  ifs.close();
  return s;
}

static bool parseFile (std::string const& filename) {
  std::string const data = readFile(filename);
 
  JasonParser parser;
  try {
    parser.parse(data);
    return true;
  }
  catch (...) {
    return false;
  }
}

// This function is used to use the dumper to produce JSON and verify
// the result. When we have parsed previously, we usually can take the
// original input, otherwise we provide a knownGood result.

static void checkDump (JasonSlice s, std::string const& knownGood) {
  JasonCharBuffer buffer;
  JasonBufferDumper dumper(buffer, JasonBufferDumper::StrategyFail);
  dumper.dump(s);
  std::string output(buffer.data(), buffer.size());
  ASSERT_EQ(knownGood, output);
}

// With the following function we check type determination and size
// of the produced Jason value:

static void checkBuild (JasonSlice s, JasonType t, JasonLength byteSize) {
  ASSERT_EQ(t, s.type());
  ASSERT_TRUE(s.isType(t));
  JasonType other = (t == JasonType::String) ? JasonType::Int
                                             : JasonType::String;
  ASSERT_FALSE(s.isType(other));
  ASSERT_FALSE(other == s.type());

  ASSERT_EQ(byteSize, s.byteSize());

  switch (t) {
    case JasonType::None:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      break;
    case JasonType::Null:
      ASSERT_TRUE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::Bool:
      ASSERT_FALSE(s.isNull());
      ASSERT_TRUE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::Double:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_TRUE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_TRUE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::Array:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_TRUE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::Object:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_TRUE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::External:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_TRUE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::UTCDate:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_TRUE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::Int:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_TRUE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_TRUE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::UInt:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_TRUE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_TRUE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::SmallInt:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_TRUE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_TRUE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::String:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_TRUE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::Binary:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_TRUE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::BCD:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_TRUE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::MinKey:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_TRUE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::MaxKey:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_TRUE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::Custom:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_TRUE(s.isCustom());
      break;
  }
}


// Let the tests begin...

TEST(StaticFilesTest, CommitsJson) {
  ASSERT_TRUE(parseFile("jsonSample/commits.json"));
}

TEST(StaticFilesTest, SampleJson) {
  ASSERT_TRUE(parseFile("jsonSample/sample.json"));
}

TEST(StaticFilesTest, SampleNoWhiteJson) {
  ASSERT_TRUE(parseFile("jsonSample/sampleNoWhite.json"));
}

TEST(StaticFilesTest, SmallJson) {
  ASSERT_TRUE(parseFile("jsonSample/small.json"));
}

TEST(StaticFilesTest, Fail2Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail2.json"));
}

TEST(StaticFilesTest, Fail3Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail3.json"));
}

TEST(StaticFilesTest, Fail4Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail4.json"));
}

TEST(StaticFilesTest, Fail5Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail5.json"));
}

TEST(StaticFilesTest, Fail6Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail6.json"));
}

TEST(StaticFilesTest, Fail7Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail7.json"));
}

TEST(StaticFilesTest, Fail8Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail8.json"));
}

TEST(StaticFilesTest, Fail9Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail9.json"));
}

TEST(StaticFilesTest, Fail10Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail10.json"));
}

TEST(StaticFilesTest, Fail11Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail11.json"));
}

TEST(StaticFilesTest, Fail12Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail12.json"));
}

TEST(StaticFilesTest, Fail13Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail13.json"));
}

TEST(StaticFilesTest, Fail14Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail14.json"));
}

TEST(StaticFilesTest, Fail15Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail15.json"));
}

TEST(StaticFilesTest, Fail16Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail16.json"));
}

TEST(StaticFilesTest, Fail17Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail17.json"));
}

TEST(StaticFilesTest, Fail19Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail19.json"));
}

TEST(StaticFilesTest, Fail20Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail20.json"));
}

TEST(StaticFilesTest, Fail21Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail21.json"));
}

TEST(StaticFilesTest, Fail22Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail22.json"));
}

TEST(StaticFilesTest, Fail23Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail23.json"));
}

TEST(StaticFilesTest, Fail24Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail24.json"));
}

TEST(StaticFilesTest, Fail25Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail25.json"));
}

TEST(StaticFilesTest, Fail26Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail26.json"));
}

TEST(StaticFilesTest, Fail27Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail27.json"));
}

TEST(StaticFilesTest, Fail28Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail28.json"));
}

TEST(StaticFilesTest, Fail29Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail29.json"));
}

TEST(StaticFilesTest, Fail30Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail30.json"));
}

TEST(StaticFilesTest, Fail31Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail31.json"));
}

TEST(StaticFilesTest, Fail32Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail32.json"));
}

TEST(StaticFilesTest, Fail33Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail33.json"));
}

TEST(TypesTest, TestNames) {
  ASSERT_EQ(0, strcmp("none", JasonTypeName(JasonType::None)));
  ASSERT_EQ(0, strcmp("null", JasonTypeName(JasonType::Null)));
  ASSERT_EQ(0, strcmp("bool", JasonTypeName(JasonType::Bool)));
  ASSERT_EQ(0, strcmp("double", JasonTypeName(JasonType::Double)));
  ASSERT_EQ(0, strcmp("string", JasonTypeName(JasonType::String)));
  ASSERT_EQ(0, strcmp("array", JasonTypeName(JasonType::Array)));
  ASSERT_EQ(0, strcmp("object", JasonTypeName(JasonType::Object)));
  ASSERT_EQ(0, strcmp("external", JasonTypeName(JasonType::External)));
  ASSERT_EQ(0, strcmp("utc-date", JasonTypeName(JasonType::UTCDate)));
  ASSERT_EQ(0, strcmp("int", JasonTypeName(JasonType::Int)));
  ASSERT_EQ(0, strcmp("uint", JasonTypeName(JasonType::UInt)));
  ASSERT_EQ(0, strcmp("smallint", JasonTypeName(JasonType::SmallInt)));
  ASSERT_EQ(0, strcmp("binary", JasonTypeName(JasonType::Binary)));
  ASSERT_EQ(0, strcmp("bcd", JasonTypeName(JasonType::BCD)));
  ASSERT_EQ(0, strcmp("min-key", JasonTypeName(JasonType::MinKey)));
  ASSERT_EQ(0, strcmp("max-key", JasonTypeName(JasonType::MaxKey)));
  ASSERT_EQ(0, strcmp("custom", JasonTypeName(JasonType::Custom)));
}

TEST(TypesTest, TestNamesArrays) {
  uint8_t const arrays[] = { 0x04, 0x05, 0x06, 0x07 };
  ASSERT_EQ(0, strcmp("array", JasonTypeName(JasonSlice(&arrays[0]).type())));
  ASSERT_EQ(0, strcmp("array", JasonTypeName(JasonSlice(&arrays[1]).type())));
  ASSERT_EQ(0, strcmp("array", JasonTypeName(JasonSlice(&arrays[2]).type())));
  ASSERT_EQ(0, strcmp("array", JasonTypeName(JasonSlice(&arrays[3]).type())));
}

TEST(TypesTest, TestNamesObjects) {
  uint8_t const objects[] = { 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d };
  ASSERT_EQ(0, strcmp("object", JasonTypeName(JasonSlice(&objects[0]).type())));
  ASSERT_EQ(0, strcmp("object", JasonTypeName(JasonSlice(&objects[1]).type())));
  ASSERT_EQ(0, strcmp("object", JasonTypeName(JasonSlice(&objects[2]).type())));
  ASSERT_EQ(0, strcmp("object", JasonTypeName(JasonSlice(&objects[3]).type())));
  ASSERT_EQ(0, strcmp("object", JasonTypeName(JasonSlice(&objects[4]).type())));
  ASSERT_EQ(0, strcmp("object", JasonTypeName(JasonSlice(&objects[5]).type())));
}

TEST(OutStreamTest, StringifyComplexObject) {
  std::string const value("{\"foo\":\"bar\",\"baz\":[1,2,3,[4]],\"bark\":[{\"troet\\nmann\":1,\"mötör\":[2,3.4,-42.5,true,false,null,\"some\\nstring\"]}]}");

  JasonParser parser;
  parser.options.sortAttributeNames = false;
  parser.parse(value);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());

  std::ostringstream result;
  result << s;

  ASSERT_EQ("[JasonSlice object, byteSize: 125]", result.str());
  
  std::string prettyResult = JasonPrettyDumper::Dump(s);
  ASSERT_EQ(std::string("{\n  \"foo\" : \"bar\",\n  \"baz\" : [\n    1,\n    2,\n    3,\n    [\n      4\n    ]\n  ],\n  \"bark\" : [\n    {\n      \"troet\\nmann\" : 1,\n      \"mötör\" : [\n        2,\n        3.4,\n        -42.5,\n        true,\n        false,\n        null,\n        \"some\\nstring\"\n      ]\n    }\n  ]\n}"), prettyResult);
}

TEST(PrettyDumperTest, SimpleObject) {
  std::string const value("{\"foo\":\"bar\"}");

  JasonParser parser;
  parser.parse(value);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());

  std::string result = JasonPrettyDumper::Dump(s);
  ASSERT_EQ(std::string("{\n  \"foo\" : \"bar\"\n}"), result);
}

TEST(PrettyDumperTest, ComplexObject) {
  std::string const value("{\"foo\":\"bar\",\"baz\":[1,2,3,[4]],\"bark\":[{\"troet\\nmann\":1,\"mötör\":[2,3.4,-42.5,true,false,null,\"some\\nstring\"]}]}");

  JasonParser parser;
  parser.options.sortAttributeNames = false;
  parser.parse(value);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());

  std::string result = JasonPrettyDumper::Dump(s);
  ASSERT_EQ(std::string("{\n  \"foo\" : \"bar\",\n  \"baz\" : [\n    1,\n    2,\n    3,\n    [\n      4\n    ]\n  ],\n  \"bark\" : [\n    {\n      \"troet\\nmann\" : 1,\n      \"mötör\" : [\n        2,\n        3.4,\n        -42.5,\n        true,\n        false,\n        null,\n        \"some\\nstring\"\n      ]\n    }\n  ]\n}"), result);
}

TEST(BufferDumperTest, Null) {
  Buffer[0] = 0x1;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  JasonCharBuffer buffer;
  JasonBufferDumper dumper(buffer, JasonBufferDumper::StrategyFail);
  dumper.dump(slice);
  std::string output(buffer.data(), buffer.size());
  ASSERT_EQ(std::string("null"), output);
}

TEST(StringDumperTest, Null) {
  Buffer[0] = 0x1;

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
    i = pp+1; check();
    i = pp-1; check();
    i = -pp; check();
    i = -pp+1; check();
    i = -pp-1; check();

    pp *= 2;
  }
}

TEST(BufferDumperTest, False) {
  Buffer[0] = 0x2;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  JasonCharBuffer buffer;
  JasonBufferDumper dumper(buffer, JasonBufferDumper::StrategyFail);
  dumper.dump(slice);
  std::string output(buffer.data(), buffer.size());
  ASSERT_EQ(std::string("false"), output);
}

TEST(StringDumperTest, False) {
  Buffer[0] = 0x2;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  std::string buffer;
  JasonStringDumper dumper(buffer, JasonStringDumper::StrategyFail);
  dumper.dump(slice);
  ASSERT_EQ(std::string("false"), buffer);
}

TEST(BufferDumperTest, True) {
  Buffer[0] = 0x3;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  JasonCharBuffer buffer;
  JasonBufferDumper dumper(buffer, JasonBufferDumper::StrategyFail);
  dumper.dump(slice);
  std::string output(buffer.data(), buffer.size());
  ASSERT_EQ(std::string("true"), output);
}

TEST(StringDumperTest, True) {
  Buffer[0] = 0x3;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  std::string buffer;
  JasonStringDumper dumper(buffer, JasonStringDumper::StrategyFail);
  dumper.dump(slice);
  ASSERT_EQ(std::string("true"), buffer);
}

TEST(StringDumperTest, CustomWithoutHandler) {
  Buffer[0] = static_cast<char>(0xf0);

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

TEST(StringDumperTest, ArangoDBIdCallbackMulti) {
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
  dumper.appendString(std::string("this is a string with special chars / \" \\ ' foo\n\r\t baz"));

  ASSERT_EQ(std::string("\"this is a string with special chars \\/ \\\" \\\\ ' foo\\n\\r\\t baz\""), buffer);
}

TEST(StringDumperTest, AppendStringTestSpecialChars) {
  std::string buffer;
  JasonStringDumper dumper(buffer, JasonStringDumper::StrategyFail);
  dumper.appendString("this is a string with special chars / \" \\ ' foo\n\r\t baz");

  ASSERT_EQ(std::string("\"this is a string with special chars \\/ \\\" \\\\ ' foo\\n\\r\\t baz\""), buffer);
}

TEST(StringDumperTest, AppendStringSlice) {
  std::string buffer;
  JasonStringDumper dumper(buffer, JasonStringDumper::StrategyFail);

  std::string const s = "this is a string with special chars / \" \\ ' foo\n\r\t baz";
  JasonBuilder b;
  b.add(Jason(s));
  JasonSlice slice(b.start());
  dumper.append(slice);

  ASSERT_EQ(std::string("\"this is a string with special chars \\/ \\\" \\\\ ' foo\\n\\r\\t baz\""), buffer);
}

TEST(StringDumperTest, AppendStringSliceRef) {
  std::string buffer;
  JasonStringDumper dumper(buffer, JasonStringDumper::StrategyFail);

  std::string const s = "this is a string with special chars / \" \\ ' foo\n\r\t baz";
  JasonBuilder b;
  b.add(Jason(s));
  JasonSlice slice(b.start());
  dumper.append(&slice);

  ASSERT_EQ(std::string("\"this is a string with special chars \\/ \\\" \\\\ ' foo\\n\\r\\t baz\""), buffer);
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

TEST(SliceTest, Null) {
  Buffer[0] = 0x1;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::Null, slice.type());
  ASSERT_TRUE(slice.isNull());
  ASSERT_EQ(1ULL, slice.byteSize());
}

TEST(SliceTest, False) {
  Buffer[0] = 0x2;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::Bool, slice.type());
  ASSERT_TRUE(slice.isBool());
  ASSERT_EQ(1ULL, slice.byteSize());
  ASSERT_FALSE(slice.getBool());
}

TEST(SliceTest, True) {
  Buffer[0] = 0x3;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::Bool, slice.type());
  ASSERT_TRUE(slice.isBool());
  ASSERT_EQ(1ULL, slice.byteSize());
  ASSERT_TRUE(slice.getBool());
}

TEST(SliceTest, MinKey) {
  Buffer[0] = 0x11;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::MinKey, slice.type());
  ASSERT_TRUE(slice.isMinKey());
  ASSERT_EQ(1ULL, slice.byteSize());
}

TEST(SliceTest, MaxKey) {
  Buffer[0] = 0x12;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::MaxKey, slice.type());
  ASSERT_TRUE(slice.isMaxKey());
  ASSERT_EQ(1ULL, slice.byteSize());
}

TEST(SliceTest, Double) {
  Buffer[0] = 0x0e;

  double value = 23.5;
  dumpDouble(value, reinterpret_cast<uint8_t*>(Buffer) + 1);

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::Double, slice.type());
  ASSERT_TRUE(slice.isDouble());
  ASSERT_EQ(9ULL, slice.byteSize());
  ASSERT_DOUBLE_EQ(value, slice.getDouble());
}

TEST(SliceTest, DoubleNegative) {
  Buffer[0] = 0x0e;

  double value = -999.91355;
  dumpDouble(value, reinterpret_cast<uint8_t*>(Buffer) + 1);

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::Double, slice.type());
  ASSERT_TRUE(slice.isDouble());
  ASSERT_EQ(9ULL, slice.byteSize());
  ASSERT_DOUBLE_EQ(value, slice.getDouble());
}

TEST(SliceTest, SmallInt) {
  int64_t expected[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -6, -5, -4, -3, -2, -1 };

  for (int i = 0; i < 16; ++i) {
    Buffer[0] = 0x30 + i;

    JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));
    ASSERT_EQ(JasonType::SmallInt, slice.type());
    ASSERT_TRUE(slice.isSmallInt());
    ASSERT_EQ(1ULL, slice.byteSize());

    ASSERT_EQ(expected[i], slice.getSmallInt());
  } 
}

TEST(SliceTest, Int1) {
  Buffer[0] = 0x20;
  uint8_t value = 0x33;
  memcpy(&Buffer[1], (void*) &value, sizeof(value));

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(2ULL, slice.byteSize());

  ASSERT_EQ(value, slice.getInt());
}

TEST(SliceTest, Int2) {
  Buffer[0] = 0x21;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(3ULL, slice.byteSize());
  ASSERT_EQ(0x4223LL, slice.getInt());
}

TEST(SliceTest, Int3) {
  Buffer[0] = 0x22;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(4ULL, slice.byteSize());
  ASSERT_EQ(0x664223LL, slice.getInt());
}

TEST(SliceTest, Int4) {
  Buffer[0] = 0x23;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0x7c;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(5ULL, slice.byteSize());
  ASSERT_EQ(0x7c664223LL, slice.getInt());
}

TEST(SliceTest, Int5) {
  Buffer[0] = 0x24;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;
  *p++ = 0x6f;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(6ULL, slice.byteSize());
  ASSERT_EQ(0x6fac664223LL, slice.getInt());
}

TEST(SliceTest, Int6) {
  Buffer[0] = 0x25;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;
  *p++ = 0xff;
  *p++ = 0x3f;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(7ULL, slice.byteSize());
  ASSERT_EQ(0x3fffac664223LL, slice.getInt());
}

TEST(SliceTest, Int7) {
  Buffer[0] = 0x26;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;
  *p++ = 0xff;
  *p++ = 0x3f;
  *p++ = 0x5a;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(8ULL, slice.byteSize());
  ASSERT_EQ(0x5a3fffac664223LL, slice.getInt());
}

TEST(SliceTest, Int8) {
  Buffer[0] = 0x27;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;
  *p++ = 0xff;
  *p++ = 0x3f;
  *p++ = 0xfa;
  *p++ = 0x6f;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(9ULL, slice.byteSize());
  ASSERT_EQ(0x6ffa3fffac664223LL, slice.getInt());
}

TEST(SliceTest, NegInt1) {
  Buffer[0] = 0x20;
  uint8_t value = 0xa3;
  memcpy(&Buffer[1], (void*) &value, sizeof(value));

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(2ULL, slice.byteSize());

  ASSERT_EQ(static_cast<int64_t>(0xffffffffffffffa3ULL), slice.getInt());
}

TEST(SliceTest, NegInt2) {
  Buffer[0] = 0x21;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0xe2;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(3ULL, slice.byteSize());
  ASSERT_EQ(static_cast<int64_t>(0xffffffffffffe223ULL), slice.getInt());
}

TEST(SliceTest, NegInt3) {
  Buffer[0] = 0x22;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0xd6;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(4ULL, slice.byteSize());
  ASSERT_EQ(static_cast<int64_t>(0xffffffffffd64223ULL), slice.getInt());
}

TEST(SliceTest, NegInt4) {
  Buffer[0] = 0x23;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(5ULL, slice.byteSize());
  ASSERT_EQ(static_cast<int64_t>(0xffffffffac664223ULL), slice.getInt());
}

TEST(SliceTest, NegInt5) {
  Buffer[0] = 0x24;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;
  *p++ = 0xff;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(6ULL, slice.byteSize());
  ASSERT_EQ(static_cast<int64_t>(0xffffffffac664223ULL), slice.getInt());
}

TEST(SliceTest, NegInt6) {
  Buffer[0] = 0x25;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;
  *p++ = 0xff;
  *p++ = 0xef;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(7ULL, slice.byteSize());
  ASSERT_EQ(static_cast<int64_t>(0xffffefffac664223ULL), slice.getInt());
}

TEST(SliceTest, NegInt7) {
  Buffer[0] = 0x26;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;
  *p++ = 0xff;
  *p++ = 0xef;
  *p++ = 0xfa;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(8ULL, slice.byteSize());
  ASSERT_EQ(static_cast<int64_t>(0xfffaefffac664223ULL), slice.getInt());
}

TEST(SliceTest, NegInt8) {
  Buffer[0] = 0x27;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;
  *p++ = 0xff;
  *p++ = 0xef;
  *p++ = 0xfa;
  *p++ = 0x8e;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::Int, slice.type());
  ASSERT_TRUE(slice.isInt());
  ASSERT_EQ(9ULL, slice.byteSize());
  ASSERT_EQ(static_cast<int64_t>(0x8efaefffac664223ULL), slice.getInt());
}

TEST(SliceTest, UInt1) {
  Buffer[0] = 0x28;
  uint8_t value = 0x33;
  memcpy(&Buffer[1], (void*) &value, sizeof(value));

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::UInt, slice.type());
  ASSERT_TRUE(slice.isUInt());
  ASSERT_EQ(2ULL, slice.byteSize());
  ASSERT_EQ(value, slice.getUInt());
}

TEST(SliceTest, UInt2) {
  Buffer[0] = 0x29;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::UInt, slice.type());
  ASSERT_TRUE(slice.isUInt());
  ASSERT_EQ(3ULL, slice.byteSize());
  ASSERT_EQ(0x4223ULL, slice.getUInt());
}

TEST(SliceTest, UInt3) {
  Buffer[0] = 0x2a;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::UInt, slice.type());
  ASSERT_TRUE(slice.isUInt());
  ASSERT_EQ(4ULL, slice.byteSize());
  ASSERT_EQ(0x664223ULL, slice.getUInt());
}

TEST(SliceTest, UInt4) {
  Buffer[0] = 0x2b;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::UInt, slice.type());
  ASSERT_TRUE(slice.isUInt());
  ASSERT_EQ(5ULL, slice.byteSize());
  ASSERT_EQ(0xac664223ULL, slice.getUInt());
}

TEST(SliceTest, UInt5) {
  Buffer[0] = 0x2c;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;
  *p++ = 0xff;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::UInt, slice.type());
  ASSERT_TRUE(slice.isUInt());
  ASSERT_EQ(6ULL, slice.byteSize());
  ASSERT_EQ(0xffac664223ULL, slice.getUInt());
}

TEST(SliceTest, UInt6) {
  Buffer[0] = 0x2d;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;
  *p++ = 0xff;
  *p++ = 0xee;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::UInt, slice.type());
  ASSERT_TRUE(slice.isUInt());
  ASSERT_EQ(7ULL, slice.byteSize());
  ASSERT_EQ(0xeeffac664223ULL, slice.getUInt());
}

TEST(SliceTest, UInt7) {
  Buffer[0] = 0x2e;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;
  *p++ = 0xff;
  *p++ = 0xee;
  *p++ = 0x59;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::UInt, slice.type());
  ASSERT_TRUE(slice.isUInt());
  ASSERT_EQ(8ULL, slice.byteSize());
  ASSERT_EQ(0x59eeffac664223ULL, slice.getUInt());
}

TEST(SliceTest, UInt8) {
  Buffer[0] = 0x2f;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x23;
  *p++ = 0x42;
  *p++ = 0x66;
  *p++ = 0xac;
  *p++ = 0xff;
  *p++ = 0xee;
  *p++ = 0x59;
  *p++ = 0xab;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::UInt, slice.type());
  ASSERT_TRUE(slice.isUInt());
  ASSERT_EQ(9ULL, slice.byteSize());
  ASSERT_EQ(0xab59eeffac664223ULL, slice.getUInt());
}

TEST(SliceTest, ArrayEmpty) {
  Buffer[0] = 0x05;
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = 0x02;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::Array, slice.type());
  ASSERT_TRUE(slice.isArray());
  ASSERT_EQ(2ULL, slice.byteSize());
  ASSERT_EQ(0ULL, slice.length());
}

TEST(SliceTest, StringEmpty) {
  Buffer[0] = 0x40;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  ASSERT_EQ(JasonType::String, slice.type());
  ASSERT_TRUE(slice.isString());
  ASSERT_EQ(1ULL, slice.byteSize());
  JasonLength len;
  char const* s = slice.getString(len);
  ASSERT_EQ(0ULL, len);
  ASSERT_EQ(0, strncmp(s, "", len));

  ASSERT_EQ("", slice.copyString());
}

TEST(SliceTest, String1) {
  Buffer[0] = 0x40 + static_cast<char>(strlen("foobar"));

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));
  uint8_t* p = (uint8_t*) &Buffer[1];
  *p++ = (uint8_t) 'f';
  *p++ = (uint8_t) 'o';
  *p++ = (uint8_t) 'o';
  *p++ = (uint8_t) 'b';
  *p++ = (uint8_t) 'a';
  *p++ = (uint8_t) 'r';

  ASSERT_EQ(JasonType::String, slice.type());
  ASSERT_TRUE(slice.isString());
  ASSERT_EQ(7ULL, slice.byteSize());
  JasonLength len;
  char const* s = slice.getString(len);
  ASSERT_EQ(6ULL, len);
  ASSERT_EQ(0, strncmp(s, "foobar", len));

  ASSERT_EQ("foobar", slice.copyString());
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

  ASSERT_EQ(JasonType::String, slice.type());
  ASSERT_TRUE(slice.isString());
  ASSERT_EQ(9ULL, slice.byteSize());
  JasonLength len;
  char const* s = slice.getString(len);
  ASSERT_EQ(8ULL, len);
  ASSERT_EQ(0, strncmp(s, "123f\r\t\nx", len));

  ASSERT_EQ("123f\r\t\nx", slice.copyString());
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

  ASSERT_EQ(JasonType::String, slice.type());
  ASSERT_TRUE(slice.isString());
  ASSERT_EQ(9ULL, slice.byteSize());
  JasonLength len;
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
  Buffer[0] = static_cast<char>(0xbf);

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));
  uint8_t* p = (uint8_t*) &Buffer[1];
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

  ASSERT_EQ(JasonType::String, slice.type());
  ASSERT_TRUE(slice.isString());
  ASSERT_EQ(15ULL, slice.byteSize());
  JasonLength len;
  char const* s = slice.getString(len);
  ASSERT_EQ(6ULL, len);
  ASSERT_EQ(0, strncmp(s, "foobar", len));

  ASSERT_EQ("foobar", slice.copyString());
}

TEST(SliceTest, IterateArrayValues) {
  std::string const value("[1,2,3,4,null,true,\"foo\",\"bar\"]");

  JasonParser parser;
  parser.parse(value);
  JasonSlice s(parser.jason());

  size_t state = 0;
  s.iterateArray([&state] (JasonSlice const& value) -> bool {
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

TEST(SliceTest, IterateObjectKeys) {
  std::string const value("{\"1foo\":\"bar\",\"2baz\":\"quux\",\"3number\":1,\"4boolean\":true,\"5empty\":null}");

  JasonParser parser;
  parser.parse(value);
  JasonSlice s(parser.jason());

  size_t state = 0;
  s.iterateObject([&state] (JasonSlice const& key, JasonSlice const& value) -> bool {
    switch (state++) {
      case 0:
        EXPECT_EQ("1foo", key.copyString());
        EXPECT_TRUE(value.isString());
        EXPECT_EQ("bar", value.copyString());
        break;
      case 1:
        EXPECT_EQ("2baz", key.copyString());
        EXPECT_TRUE(value.isString());
        EXPECT_EQ("quux", value.copyString());
        break;
      case 2:
        EXPECT_EQ("3number", key.copyString());
        EXPECT_TRUE(value.isNumber());
        EXPECT_EQ(1ULL, value.getUInt());
        break;
      case 3:
        EXPECT_EQ("4boolean", key.copyString());
        EXPECT_TRUE(value.isBoolean());
        EXPECT_TRUE(value.getBoolean());
        break;
      case 4:
        EXPECT_EQ("5empty", key.copyString());
        EXPECT_TRUE(value.isNull());
        break;
    }
    return true;
  });
  ASSERT_EQ(5U, state);
}

TEST(SliceTest, IterateObjectValues) {
  std::string const value("{\"1foo\":\"bar\",\"2baz\":\"quux\",\"3number\":1,\"4boolean\":true,\"5empty\":null}");

  JasonParser parser;
  parser.parse(value);
  JasonSlice s(parser.jason());

  std::vector<std::string> seenKeys;
  s.iterateObject([&] (JasonSlice const& key, JasonSlice const&) -> bool {
    seenKeys.emplace_back(key.copyString());
    return true;
  });

  ASSERT_EQ(5U, seenKeys.size());
  ASSERT_EQ("1foo", seenKeys[0]);
  ASSERT_EQ("2baz", seenKeys[1]);
  ASSERT_EQ("3number", seenKeys[2]);
  ASSERT_EQ("4boolean", seenKeys[3]);
  ASSERT_EQ("5empty", seenKeys[4]);
}

TEST(SliceTest, ObjectKeys) {
  std::string const value("{\"1foo\":\"bar\",\"2baz\":\"quux\",\"3number\":1,\"4boolean\":true,\"5empty\":null}");

  JasonParser parser;
  parser.parse(value);
  JasonSlice s(parser.jason());

  std::vector<std::string> keys = s.keys();
  ASSERT_EQ(5U, keys.size());
  ASSERT_EQ("1foo", keys[0]);
  ASSERT_EQ("2baz", keys[1]);
  ASSERT_EQ("3number", keys[2]);
  ASSERT_EQ("4boolean", keys[3]);
  ASSERT_EQ("5empty", keys[4]);
}

TEST(SliceTest, ObjectKeysRef) {
  std::string const value("{\"1foo\":\"bar\",\"2baz\":\"quux\",\"3number\":1,\"4boolean\":true,\"5empty\":null}");

  JasonParser parser;
  parser.parse(value);
  JasonSlice s(parser.jason());

  std::vector<std::string> keys;
  s.keys(keys);
  ASSERT_EQ(5U, keys.size());
  ASSERT_EQ("1foo", keys[0]);
  ASSERT_EQ("2baz", keys[1]);
  ASSERT_EQ("3number", keys[2]);
  ASSERT_EQ("4boolean", keys[3]);
  ASSERT_EQ("5empty", keys[4]);
}

TEST(BuilderTest, Null) {
  JasonBuilder b;
  b.add(Jason());
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t const correctResult[] 
    = { 0x01 };

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, False) {
  JasonBuilder b;
  b.add(Jason(false));
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t const correctResult[] 
    = { 0x02 };

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, True) {
  JasonBuilder b;
  b.add(Jason(true));
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t const correctResult[] 
    = { 0x03 };

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, Double) {
  static double value = 123.456;
  JasonBuilder b;
  b.add(Jason(value));
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t correctResult[9] 
    = { 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  ASSERT_EQ(8ULL, sizeof(double));
  dumpDouble(value, correctResult + 1);

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
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

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, ArrayEmpty) {
  JasonBuilder b;
  b.add(Jason(JasonType::Array));
  b.close();
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t correctResult[] 
    = { 0x04, 0x02 };

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, ArraySingleEntry) {
  JasonBuilder b;
  b.add(Jason(JasonType::Array));
  b.add(Jason(uint64_t(1)));
  b.close();
  uint8_t* result = b.start();
  ASSERT_EQ(0x04U, *result);
  JasonLength len = b.size();

  static uint8_t correctResult[] 
    = { 0x04, 0x04, 0x31, 0x01 };

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, ArraySingleEntryLong) {
  std::string const value("ngdddddljjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjsdddffffffffffffmmmmmmmmmmmmmmmsfdlllllllllllllllllllllllllllllllllllllllllllllllllrjjjjjjsddddddddddddddddddhhhhhhkkkkkkkksssssssssssssssssssssssssssssssssdddddddddddddddddkkkkkkkkkkkkksddddddddddddssssssssssfvvvvvvvvvvvvvvvvvvvvvvvvvvvfvgfff");
  JasonBuilder b;
  b.add(Jason(JasonType::Array));
  b.add(Jason(value));
  b.close();
  uint8_t* result = b.start();
  ASSERT_EQ(0x04U, *result);
  JasonLength len = b.size(); 

  static uint8_t correctResult[] = {
    0x04, 0x00, 0x2e, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbf, 0x1a, 0x01, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x6e, 0x67, 0x64, 0x64, 0x64, 0x64, 0x64, 0x6c, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 
    0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 
    0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x73, 0x64, 0x64, 0x64, 0x66, 0x66, 
    0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x6d, 0x6d, 0x6d, 0x6d, 0x6d, 0x6d, 
    0x6d, 0x6d, 0x6d, 0x6d, 0x6d, 0x6d, 0x6d, 0x6d, 0x6d, 0x73, 0x66, 0x64, 0x6c, 0x6c, 0x6c, 0x6c, 
    0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 
    0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 
    0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x72, 0x6a, 0x6a, 
    0x6a, 0x6a, 0x6a, 0x6a, 0x73, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 
    0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x6b, 0x6b, 0x6b, 
    0x6b, 0x6b, 0x6b, 0x6b, 0x6b, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 
    0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 
    0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 
    0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x6b, 0x6b, 0x6b, 0x6b, 0x6b, 0x6b, 0x6b, 0x6b, 0x6b, 
    0x6b, 0x6b, 0x6b, 0x6b, 0x73, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 
    0x64, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x73, 0x66, 0x76, 0x76, 0x76, 0x76, 
    0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 
    0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x76, 0x66, 0x76, 0x67, 0x66, 0x66, 0x66, 0x01,     
  };

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, ArraySameSizeEntries) {
  JasonBuilder b;
  b.add(Jason(JasonType::Array));
  b.add(Jason(uint64_t(1)));
  b.add(Jason(uint64_t(2)));
  b.add(Jason(uint64_t(3)));
  b.close();
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t correctResult[] 
    = { 0x04, 0x06, 0x31, 0x32, 0x33, 0x03 };

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, Array4) {
  double value = 2.3;
  JasonBuilder b;
  b.add(Jason(JasonType::Array));
  b.add(Jason(uint64_t(1200)));
  b.add(Jason(value));
  b.add(Jason("abc"));
  b.add(Jason(true));
  b.close();

  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t correctResult[] 
    = { 0x05, 0x1c,
        0x29, 0xb0, 0x04,   // uint(1200) = 0x4b0
        0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // double(2.3)
        0x43, 0x61, 0x62, 0x63,
        0x03,
        0x02, 0x00, 0x05, 0x00, 0x0e, 0x00, 0x12, 0x00,
        0x04};
  dumpDouble(value, correctResult + 6);

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, ObjectEmpty) {
  JasonBuilder b;
  b.add(Jason(JasonType::Object));
  b.close();
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t correctResult[] 
    = { 0x08, 0x02 };

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, ObjectSorted) {
  double value = 2.3;
  JasonBuilder b;
  b.options.sortAttributeNames = true;
  b.add(Jason(JasonType::Object));
  b.add("d", Jason(uint64_t(1200)));
  b.add("c", Jason(value));
  b.add("b", Jason("abc"));
  b.add("a", Jason(true));
  b.close();

  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t correctResult[] 
    = { 0x08, 0x24,
        0x41, 0x64, 0x29, 0xb0, 0x04,        // "d": uint(1200) = 0x4b0
        0x41, 0x63, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   
                                             // "c": double(2.3)
        0x41, 0x62, 0x43, 0x61, 0x62, 0x63,  // "b": "abc"
        0x41, 0x61, 0x03,                    // "a": true
        0x18, 0x00, 0x12, 0x00, 0x07, 0x00, 0x02, 0x00,
        0x04
      };
  dumpDouble(value, correctResult + 10);

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, ObjectUnsorted) {
  double value = 2.3;
  JasonBuilder b;
  b.options.sortAttributeNames = false;
  b.add(Jason(JasonType::Object));
  b.add("d", Jason(uint64_t(1200)));
  b.add("c", Jason(value));
  b.add("b", Jason("abc"));
  b.add("a", Jason(true));
  b.close();

  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t correctResult[] 
    = { 0x0b, 0x24,
        0x41, 0x64, 0x29, 0xb0, 0x04,        // "d": uint(1200) = 0x4b0
        0x41, 0x63, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   
                                             // "c": double(2.3)
        0x41, 0x62, 0x43, 0x61, 0x62, 0x63,  // "b": "abc"
        0x41, 0x61, 0x03,                    // "a": true
        0x02, 0x00, 0x07, 0x00, 0x12, 0x00, 0x18, 0x00,
        0x04
      };
  dumpDouble(value, correctResult + 10);

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, Object4) {
  double value = 2.3;
  JasonBuilder b;
  b.add(Jason(JasonType::Object));
  b.add("a", Jason(uint64_t(1200)));
  b.add("b", Jason(value));
  b.add("c", Jason("abc"));
  b.add("d", Jason(true));
  b.close();

  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t correctResult[] 
    = { 0x08, 0x24,
        0x41, 0x61, 0x29, 0xb0, 0x04,        // "a": uint(1200) = 0x4b0
        0x41, 0x62, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   
                                             // "b": double(2.3)
        0x41, 0x63, 0x43, 0x61, 0x62, 0x63,  // "c": "abc"
        0x41, 0x64, 0x03,                    // "d": true
        0x02, 0x00, 0x07, 0x00, 0x12, 0x00, 0x18, 0x00,
        0x04
      };
  dumpDouble(value, correctResult + 10);

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, External) {
  uint8_t externalStuff[] = { 0x01 };
  JasonBuilder b;
  b.add(Jason(const_cast<void const*>(static_cast<void*>(externalStuff)), 
              JasonType::External));
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t correctResult[1 + sizeof(char*)] 
    = { 0x00 };
  correctResult[0] = 0x10;
  uint8_t* p = externalStuff;
  memcpy(correctResult + 1, &p, sizeof(uint8_t*));

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, ExternalUTCDate) {
  int64_t const v = -24549959465;
  JasonBuilder bExternal;
  bExternal.add(Jason(v, JasonType::UTCDate));

  JasonBuilder b;
  b.add(Jason(const_cast<void const*>(static_cast<void*>(bExternal.start()))));
  
  JasonSlice s(b.start());
  ASSERT_EQ(JasonType::External, s.type());
#ifdef JASON_64BIT
  ASSERT_EQ(9ULL, s.byteSize());
#else
  ASSERT_EQ(5ULL, s.byteSize());
#endif
  JasonSlice sExternal(s.getExternal());
  ASSERT_EQ(9ULL, sExternal.byteSize());
  ASSERT_EQ(JasonType::UTCDate, sExternal.type());
  ASSERT_EQ(v, sExternal.getUTCDate());
}

TEST(BuilderTest, ExternalDouble) {
  double const v = -134.494401;
  JasonBuilder bExternal;
  bExternal.add(Jason(v));

  JasonBuilder b;
  b.add(Jason(const_cast<void const*>(static_cast<void*>(bExternal.start()))));
  
  JasonSlice s(b.start());
  ASSERT_EQ(JasonType::External, s.type());
#ifdef JASON_64BIT
  ASSERT_EQ(9ULL, s.byteSize());
#else
  ASSERT_EQ(5ULL, s.byteSize());
#endif 

  JasonSlice sExternal(s.getExternal());
  ASSERT_EQ(9ULL, sExternal.byteSize());
  ASSERT_EQ(JasonType::Double, sExternal.type());
  ASSERT_DOUBLE_EQ(v, sExternal.getDouble());
}

TEST(BuilderTest, ExternalBinary) {
  char const* p = "the quick brown FOX jumped over the lazy dog";
  JasonBuilder bExternal;
  bExternal.add(Jason(std::string(p), JasonType::Binary));

  JasonBuilder b;
  b.add(Jason(const_cast<void const*>(static_cast<void*>(bExternal.start()))));
  
  JasonSlice s(b.start());
  ASSERT_EQ(JasonType::External, s.type());
#ifdef JASON_64BIT
  ASSERT_EQ(9ULL, s.byteSize());
#else
  ASSERT_EQ(5ULL, s.byteSize());
#endif 
 
  JasonSlice sExternal(s.getExternal());
  ASSERT_EQ(2 + strlen(p), sExternal.byteSize());
  ASSERT_EQ(JasonType::Binary, sExternal.type());
  JasonLength len;
  uint8_t const* str = sExternal.getBinary(len);
  ASSERT_EQ(strlen(p), len);
  ASSERT_EQ(0, memcmp(str, p, len));
}

TEST(BuilderTest, ExternalString) {
  char const* p = "the quick brown FOX jumped over the lazy dog";
  JasonBuilder bExternal;
  bExternal.add(Jason(std::string(p)));

  JasonBuilder b;
  b.add(Jason(const_cast<void const*>(static_cast<void*>(bExternal.start()))));
  
  JasonSlice s(b.start());
  ASSERT_EQ(JasonType::External, s.type());
#ifdef JASON_64BIT
  ASSERT_EQ(9ULL, s.byteSize());
#else
  ASSERT_EQ(5ULL, s.byteSize());
#endif 
 
  JasonSlice sExternal(s.getExternal());
  ASSERT_EQ(1 + strlen(p), sExternal.byteSize());
  ASSERT_EQ(JasonType::String, sExternal.type());
  JasonLength len;
  char const* str = sExternal.getString(len);
  ASSERT_EQ(strlen(p), len);
  ASSERT_EQ(0, strncmp(str, p, len));
}

TEST(BuilderTest, ExternalExternal) {
  char const* p = "the quick brown FOX jumped over the lazy dog";
  JasonBuilder bExternal;
  bExternal.add(Jason(std::string(p)));

  JasonBuilder bExExternal;
  bExExternal.add(Jason(const_cast<void const*>(static_cast<void*>(bExternal.start()))));
  bExExternal.add(Jason(std::string(p)));

  JasonBuilder b;
  b.add(Jason(const_cast<void const*>(static_cast<void*>(bExExternal.start()))));
  
  JasonSlice s(b.start());
  ASSERT_EQ(JasonType::External, s.type());
#ifdef JASON_64BIT
  ASSERT_EQ(9ULL, s.byteSize());
#else
  ASSERT_EQ(5ULL, s.byteSize());
#endif

  JasonSlice sExternal(s.getExternal());
  ASSERT_EQ(JasonType::External, sExternal.type());
#ifdef JASON_64BIT
  ASSERT_EQ(9ULL, sExternal.byteSize());
#else
  ASSERT_EQ(5ULL, sExternal.byteSize());
#endif 

  JasonSlice sExExternal(sExternal.getExternal());
  ASSERT_EQ(1 + strlen(p), sExExternal.byteSize());
  ASSERT_EQ(JasonType::String, sExExternal.type());
  JasonLength len;
  char const* str = sExExternal.getString(len);
  ASSERT_EQ(strlen(p), len);
  ASSERT_EQ(0, strncmp(str, p, len));
}

TEST(BuilderTest, UInt) {
  uint64_t value = 0x12345678abcdef;
  JasonBuilder b;
  b.add(Jason(value));
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t correctResult[]
    = { 0x2e, 0xef, 0xcd, 0xab, 0x78, 0x56, 0x34, 0x12 };

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, IntPos) {
  int64_t value = 0x12345678abcdef;
  JasonBuilder b;
  b.add(Jason(value));
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t correctResult[]
    = { 0x26, 0xef, 0xcd, 0xab, 0x78, 0x56, 0x34, 0x12 };

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, IntNeg) {
  int64_t value = -0x12345678abcdef;
  JasonBuilder b;
  b.add(Jason(value));
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t correctResult[]
    = { 0x26, 0x11, 0x32, 0x54, 0x87, 0xa9, 0xcb, 0xed };

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, Int1Limits) {
  int64_t values[] = {-0x80LL, 0x7fLL, -0x81LL, 0x80LL,
                      -0x8000LL, 0x7fffLL, -0x8001LL, 0x8000LL,
                      -0x800000LL, 0x7fffffLL, -0x800001LL, 0x800000LL,
                      -0x80000000LL, 0x7fffffffLL, -0x80000001LL, 0x80000000LL,
                      -0x8000000000LL, 0x7fffffffffLL,
                      -0x8000000001LL, 0x8000000000LL,
                      -0x800000000000LL, 0x7fffffffffffLL, 
                      -0x800000000001LL, 0x800000000000LL,
                      -0x80000000000000LL, 0x7fffffffffffffLL, 
                      -0x80000000000001LL, 0x80000000000000LL,
                      arangodb::jason::toInt64(0x8000000000000000ULL),
                      0x7fffffffffffffffLL};
  for (size_t i = 0; i < sizeof(values) / sizeof(int64_t); i++) {
    int64_t v = values[i];
    JasonBuilder b;
    b.add(Jason(v));
    uint8_t* result = b.start();
    JasonSlice s(result);
    ASSERT_TRUE(s.isInt());
    ASSERT_EQ(v, s.getInt());
  }
}

TEST(BuilderTest, StringChar) {
  char const* value = "der fuxx ging in den wald und aß pilze";
  size_t const valueLen = strlen(value);
  JasonBuilder b;
  b.add(Jason(value));

  JasonSlice slice = JasonSlice(b.start());
  ASSERT_TRUE(slice.isString());
 
  JasonLength len;
  char const* s = slice.getString(len);
  ASSERT_EQ(valueLen, len);
  ASSERT_EQ(0, strncmp(s, value, valueLen));

  std::string c = slice.copyString();
  ASSERT_EQ(valueLen, c.size());
  ASSERT_EQ(0, strncmp(value, c.c_str(), valueLen));
}

TEST(BuilderTest, StringString) {
  std::string const value("der fuxx ging in den wald und aß pilze");
  JasonBuilder b;
  b.add(Jason(value));

  JasonSlice slice = JasonSlice(b.start());
  ASSERT_TRUE(slice.isString());
 
  JasonLength len;
  char const* s = slice.getString(len);
  ASSERT_EQ(value.size(), len);
  ASSERT_EQ(0, strncmp(s, value.c_str(), value.size()));

  std::string c = slice.copyString();
  ASSERT_EQ(value.size(), c.size());
  ASSERT_EQ(value, c);
}

TEST(BuilderTest, Binary) {
  uint8_t binaryStuff[] = { 0x02, 0x03, 0x05, 0x08, 0x0d };

  JasonBuilder b;
  b.add(JasonPair(binaryStuff, sizeof(binaryStuff)));
  uint8_t* result = b.start();
  JasonLength len = b.size();

  static uint8_t correctResult[]
    = { 0xc0, 0x05, 0x02, 0x03, 0x05, 0x08, 0x0d };

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

TEST(BuilderTest, UTCDate) {
  int64_t const value = 12345678;
  JasonBuilder b;
  b.add(Jason(value, JasonType::UTCDate));

  JasonSlice s(b.start());
  ASSERT_EQ(0x0fU, s.head());
  ASSERT_TRUE(s.isUTCDate());
  ASSERT_EQ(9UL, s.byteSize());
  ASSERT_EQ(value, s.getUTCDate());
}

TEST(BuilderTest, UTCDateZero) {
  int64_t const value = 0;
  JasonBuilder b;
  b.add(Jason(value, JasonType::UTCDate));

  JasonSlice s(b.start());
  ASSERT_EQ(0x0fU, s.head());
  ASSERT_TRUE(s.isUTCDate());
  ASSERT_EQ(9UL, s.byteSize());
  ASSERT_EQ(value, s.getUTCDate());
}

TEST(BuilderTest, UTCDateMin) {
  int64_t const value = INT64_MIN;
  JasonBuilder b;
  b.add(Jason(value, JasonType::UTCDate));

  JasonSlice s(b.start());
  ASSERT_EQ(0x0fU, s.head());
  ASSERT_TRUE(s.isUTCDate());
  ASSERT_EQ(9UL, s.byteSize());
  ASSERT_EQ(value, s.getUTCDate());
}

TEST(BuilderTest, UTCDateMax) {
  int64_t const value = INT64_MAX;
  JasonBuilder b;
  b.add(Jason(value, JasonType::UTCDate));

  JasonSlice s(b.start());
  ASSERT_EQ(0x0fU, s.head());
  ASSERT_TRUE(s.isUTCDate());
  ASSERT_EQ(9UL, s.byteSize());
  ASSERT_EQ(value, s.getUTCDate());
}

TEST(BuilderTest, ID) {
  // This is somewhat tautological, nevertheless...
  static uint8_t const correctResult[]
    = { 0xf1, 0x2b, 0x78, 0x56, 0x34, 0x12,
        0x45, 0x02, 0x03, 0x05, 0x08, 0x0d };

  JasonBuilder b;
  uint8_t* p = b.add(JasonPair(sizeof(correctResult), JasonType::Custom));
  memcpy(p, correctResult, sizeof(correctResult));
  uint8_t* result = b.start();
  JasonLength len = b.size();

  ASSERT_EQ(sizeof(correctResult), len);
  ASSERT_EQ(0, memcmp(result, correctResult, len));
}

/* TODO: activate & fix this test
TEST(BuilderTest, ArangoDB_id) {
  JasonBuilder b;
  b.add(Jason(JasonType::Object));
  uint8_t* p = b.add("_id", JasonPair(1ULL, JasonType::Custom));
  *p = 0xf0;
  b.close();

  JasonSlice s(b.start());
  ASSERT_EQ(8ULL, s.byteSize());

  JasonSlice ss = s.keyAt(0);
  checkBuild(ss, JasonType::String, 4);
  std::string correct = "_id";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(0);
  ASSERT_EQ(JasonType::Custom, ss.type());
  checkBuild(ss, JasonType::Custom, 1);
}
*/

TEST(ParserTest, Garbage1) {
  std::string const value("z");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(0u, parser.errorPos());
}

TEST(ParserTest, Garbage2) {
  std::string const value("foo");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(1u, parser.errorPos());
}

TEST(ParserTest, Garbage3) {
  std::string const value("truth");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(3u, parser.errorPos());
}

TEST(ParserTest, Garbage4) {
  std::string const value("tru");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(2u, parser.errorPos());
}

TEST(ParserTest, Garbage5) {
  std::string const value("truebar");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(4u, parser.errorPos());
}

TEST(ParserTest, Garbage6) {
  std::string const value("fals");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(3u, parser.errorPos());
}

TEST(ParserTest, Garbage7) {
  std::string const value("falselaber");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(5u, parser.errorPos());
}

TEST(ParserTest, Garbage8) {
  std::string const value("zauberzauber");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(0u, parser.errorPos());
}

TEST(ParserTest, Garbage9) {
  std::string const value("true,");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(4u, parser.errorPos());
}

TEST(ParserTest, Punctuation1) {
  std::string const value(",");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(0u, parser.errorPos());
}

TEST(ParserTest, Punctuation2) {
  std::string const value("/");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(0u, parser.errorPos());
}

TEST(ParserTest, Punctuation3) {
  std::string const value("@");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(0u, parser.errorPos());
}

TEST(ParserTest, Punctuation4) {
  std::string const value(":");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(0u, parser.errorPos());
}

TEST(ParserTest, Punctuation5) {
  std::string const value("!");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(0u, parser.errorPos());
}

TEST(ParserTest, Null) {
  std::string const value("null");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Null, 1ULL);

  checkDump(s, value);
}

TEST(ParserTest, False) {
  std::string const value("false");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Bool, 1ULL);
  ASSERT_FALSE(s.getBool());

  checkDump(s, value);
}

TEST(ParserTest, True) {
  std::string const value("true");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Bool, 1ULL);
  ASSERT_TRUE(s.getBool());

  checkDump(s, value);
}

TEST(ParserTest, Zero) {
  std::string const value("0");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::SmallInt, 1ULL);
  ASSERT_EQ(0, s.getSmallInt());

  checkDump(s, value);
}

TEST(ParserTest, ZeroInvalid) {
  std::string const value("00");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(1u, parser.errorPos());
}

TEST(ParserTest, NumberIncomplete) {
  std::string const value("-");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(0u, parser.errorPos());
}

TEST(ParserTest, Int1) {
  std::string const value("1");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::SmallInt, 1ULL);
  ASSERT_EQ(1, s.getSmallInt());

  checkDump(s, value);
}

TEST(ParserTest, IntM1) {
  std::string const value("-1");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::SmallInt, 1ULL);
  ASSERT_EQ(-1LL, s.getSmallInt());

  checkDump(s, value);
}

TEST(ParserTest, Int2) {
  std::string const value("100000");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::UInt, 4ULL);
  ASSERT_EQ(100000ULL, s.getUInt());

  checkDump(s, value);
}

TEST(ParserTest, Int3) {
  std::string const value("-100000");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Int, 4ULL);
  ASSERT_EQ(-100000LL, s.getInt());

  checkDump(s, value);
}

TEST(ParserTest, UIntMaxNeg) {
  std::string value("-");
  value.append(std::to_string(UINT64_MAX));

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Double, 9ULL);
  // handle rounding errors
  ASSERT_DOUBLE_EQ(-18446744073709551615., s.getDouble());
}

TEST(ParserTest, IntMin) {
  std::string const value(std::to_string(INT64_MIN));

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Int, 9ULL);
  ASSERT_EQ(INT64_MIN, s.getInt());

  checkDump(s, value);
}

TEST(ParserTest, IntMinMinusOne) {
  std::string const value("-9223372036854775809"); // INT64_MIN - 1

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Double, 9ULL);
  ASSERT_DOUBLE_EQ(-9223372036854775809., s.getDouble());
}

TEST(ParserTest, IntMax) {
  std::string const value(std::to_string(INT64_MAX));

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::UInt, 9ULL);
  ASSERT_EQ(static_cast<uint64_t>(INT64_MAX), s.getUInt());

  checkDump(s, value);
}

TEST(ParserTest, IntMaxPlusOne) {
  std::string const value("9223372036854775808"); // INT64_MAX + 1

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::UInt, 9ULL);
  ASSERT_EQ(static_cast<uint64_t>(INT64_MAX) + 1, s.getUInt());

  checkDump(s, value);
}

TEST(ParserTest, UIntMax) {
  std::string const value(std::to_string(UINT64_MAX));

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::UInt, 9ULL);
  ASSERT_EQ(UINT64_MAX, s.getUInt());

  checkDump(s, value);
}

TEST(ParserTest, UIntMaxPlusOne) {
  std::string const value("18446744073709551616"); // UINT64_MAX + 1

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Double, 9ULL);
  ASSERT_DOUBLE_EQ(18446744073709551616., s.getDouble());
}

TEST(ParserTest, Double1) {
  std::string const value("1.0124");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Double, 9ULL);
  ASSERT_EQ(1.0124, s.getDouble());

  checkDump(s, value);
}

TEST(ParserTest, Double2) {
  std::string const value("-1.0124");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Double, 9ULL);
  ASSERT_EQ(-1.0124, s.getDouble());

  checkDump(s, value);
}

TEST(ParserTest, DoubleScientific1) {
  std::string const value("-1.0124e42");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Double, 9ULL);
  ASSERT_EQ(-1.0124e42, s.getDouble());

  std::string const valueOut("-1.0124e+42");
  checkDump(s, valueOut);
}

TEST(ParserTest, DoubleScientific2) {
  std::string const value("-1.0124e+42");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Double, 9ULL);
  ASSERT_EQ(-1.0124e42, s.getDouble());

  checkDump(s, value);
}

TEST(ParserTest, DoubleScientific3) {
  std::string const value("3122243.0124e-42");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Double, 9ULL);
  ASSERT_EQ(3122243.0124e-42, s.getDouble());

  std::string const valueOut("3.1222430124e-36");
  checkDump(s, valueOut);
}

TEST(ParserTest, DoubleScientific4) {
  std::string const value("2335431.0124E-42");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Double, 9ULL);
  ASSERT_EQ(2335431.0124E-42, s.getDouble());

  std::string const valueOut("2.3354310124e-36");
  checkDump(s, valueOut);
}

TEST(ParserTest, IntMinusInf) {
  std::string const value("-999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::NumberOutOfRange);
}

TEST(ParserTest, IntPlusInf) {
  std::string const value("999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::NumberOutOfRange);
}

TEST(ParserTest, DoubleMinusInf) {
  std::string const value("-1.2345e999");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::NumberOutOfRange);
}

TEST(ParserTest, DoublePlusInf) {
  std::string const value("1.2345e999");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::NumberOutOfRange);
}

TEST(ParserTest, Empty) {
  std::string const value("");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(0u, parser.errorPos());
}

TEST(ParserTest, WhitespaceOnly) {
  std::string const value("  ");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(1u, parser.errorPos());
}

TEST(ParserTest, UnterminatedStringLiteral) {
  std::string const value("\"der hund");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(8u, parser.errorPos());
}

TEST(ParserTest, StringLiteral) {
  std::string const value("\"der hund ging in den wald und aß den fuxx\"");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  std::string const correct = "der hund ging in den wald und aß den fuxx";
  checkBuild(s, JasonType::String, 1 + correct.size());
  char const* p = s.getString(len);
  ASSERT_EQ(correct.size(), len);
  ASSERT_EQ(0, strncmp(correct.c_str(), p, len));
  std::string out = s.copyString();
  ASSERT_EQ(correct, out);

  std::string valueOut = "\"der hund ging in den wald und aß den fuxx\"";
  checkDump(s, valueOut);
}

TEST(ParserTest, StringLiteralEmpty) {
  std::string const value("\"\"");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::String, 1ULL);
  char const* p = s.getString(len);
  ASSERT_EQ(0, strncmp("", p, len));
  ASSERT_EQ(0ULL, len);
  std::string out = s.copyString();
  std::string empty;
  ASSERT_EQ(empty, out);

  checkDump(s, value);
}

TEST(ParserTest, StringLiteralInvalidUtfValue1) {
  std::string value;
  value.push_back('"');
  value.push_back(static_cast<unsigned char>(0x80));
  value.push_back('"');

  JasonParser parser;
  parser.options.validateUtf8Strings = true;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::InvalidUtf8Sequence);
  ASSERT_EQ(1u, parser.errorPos());
  parser.options.validateUtf8Strings = false;
  ASSERT_EQ(1ULL, parser.parse(value));
}

TEST(ParserTest, StringLiteralInvalidUtfValue2) {
  std::string value;
  value.push_back('"');
  value.push_back(static_cast<unsigned char>(0xff));
  value.push_back(static_cast<unsigned char>(0xff));
  value.push_back('"');

  JasonParser parser;
  parser.options.validateUtf8Strings = true;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::InvalidUtf8Sequence);
  ASSERT_EQ(1u, parser.errorPos());
  parser.options.validateUtf8Strings = false;
  ASSERT_EQ(1ULL, parser.parse(value));
}

TEST(ParserTest, StringLiteralControlCharacter) {
  for (char c = 0; c < 0x20; c++) {
    std::string value;
    value.push_back('"');
    value.push_back(c);
    value.push_back('"');

    JasonParser parser;
    EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::UnexpectedControlCharacter);
    ASSERT_EQ(1u, parser.errorPos());
  }
}

TEST(ParserTest, StringLiteralUnfinishedUtfSequence1) {
  std::string const value("\"\\u\"");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(3u, parser.errorPos());
}

TEST(ParserTest, StringLiteralUnfinishedUtfSequence2) {
  std::string const value("\"\\u0\"");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(4u, parser.errorPos());
}

TEST(ParserTest, StringLiteralUnfinishedUtfSequence3) {
  std::string const value("\"\\u01\"");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(5u, parser.errorPos());
}

TEST(ParserTest, StringLiteralUnfinishedUtfSequence4) {
  std::string const value("\"\\u012\"");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(6u, parser.errorPos());
}

TEST(ParserTest, StringLiteralUtf8SequenceLowerCase) {
  std::string const value("\"der m\\u00d6ter\"");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::String, 11ULL);
  char const* p = s.getString(len);
  ASSERT_EQ(10ULL, len);
  std::string correct = "der m\xc3\x96ter";
  ASSERT_EQ(0, strncmp(correct.c_str(), p, len));
  std::string out = s.copyString();
  ASSERT_EQ(correct, out);

  std::string const valueOut("\"der mÖter\"");
  checkDump(s, valueOut);
}

TEST(ParserTest, StringLiteralUtf8SequenceUpperCase) {
  std::string const value("\"der m\\u00D6ter\"");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  std::string correct = "der mÖter";
  checkBuild(s, JasonType::String, 1 + correct.size());
  char const* p = s.getString(len);
  ASSERT_EQ(correct.size(), len);
  ASSERT_EQ(0, strncmp(correct.c_str(), p, len));
  std::string out = s.copyString();
  ASSERT_EQ(correct, out);

  checkDump(s, std::string("\"der mÖter\""));
}

TEST(ParserTest, StringLiteralUtf8Chars) {
  std::string const value("\"der mötör klötörte mät dän fößen\"");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  std::string correct = "der mötör klötörte mät dän fößen";
  checkBuild(s, JasonType::String, 1 + correct.size());
  char const* p = s.getString(len);
  ASSERT_EQ(correct.size(), len);
  ASSERT_EQ(0, strncmp(correct.c_str(), p, len));
  std::string out = s.copyString();
  ASSERT_EQ(correct, out);

//  std::string const valueOut("\"der mötör kö\\u00F6t\\u00F6r kl\\u00F6t\\u00F6rte m\\u00E4t d\\u00E4n f\\u00F6\\u00DFen\"");
  checkDump(s, value);
}

TEST(ParserTest, StringLiteralWithSpecials) {
  std::string const value("  \"der\\thund\\nging\\rin\\fden\\\\wald\\\"und\\b\\nden'fux\"  ");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  std::string correct = "der\thund\nging\rin\fden\\wald\"und\b\nden'fux";
  checkBuild(s, JasonType::String, 1 + correct.size());
  char const* p = s.getString(len);
  ASSERT_EQ(correct.size(), len);
  ASSERT_EQ(0, strncmp(correct.c_str(), p, len));
  std::string out = s.copyString();
  ASSERT_EQ(correct, out);

  std::string const valueOut("\"der\\thund\\nging\\rin\\fden\\\\wald\\\"und\\b\\nden'fux\"");
  checkDump(s, valueOut);
}

TEST(ParserTest, StringLiteralWithSurrogatePairs) {
  std::string const value("\"\\ud800\\udc00\\udbff\\udfff\\udbc8\\udf45\"");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  std::string correct = "\xf0\x90\x80\x80\xf4\x8f\xbf\xbf\xf4\x82\x8d\x85";
  checkBuild(s, JasonType::String, 1 + correct.size());
  char const* p = s.getString(len);
  ASSERT_EQ(correct.size(), len);
  ASSERT_EQ(0, strncmp(correct.c_str(), p, len));
  std::string out = s.copyString();
  ASSERT_EQ(correct, out);

  std::string const valueOut("\"\xf0\x90\x80\x80\xf4\x8f\xbf\xbf\xf4\x82\x8d\x85\"");
  checkDump(s, valueOut);
}

TEST(ParserTest, EmptyArray) {
  std::string const value("[]");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Array, 2);
  ASSERT_EQ(0ULL, s.length());

  checkDump(s, value);
}

TEST(ParserTest, WhitespacedArray) {
  std::string const value("  [    ]   ");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Array, 2);
  ASSERT_EQ(0ULL, s.length());

  std::string const valueOut = "[]";
  checkDump(s, valueOut);
}

TEST(ParserTest, Array1) {
  std::string const value("[1]");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Array, 4);
  ASSERT_EQ(1ULL, s.length());
  JasonSlice ss = s[0];
  checkBuild(ss, JasonType::SmallInt, 1);
  ASSERT_EQ(1ULL, ss.getUInt());

  checkDump(s, value);
}

TEST(ParserTest, Array2) {
  std::string const value("[1,2]");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Array, 5);
  ASSERT_EQ(2ULL, s.length());
  JasonSlice ss = s[0];
  checkBuild(ss, JasonType::SmallInt, 1);
  ASSERT_EQ(1ULL, ss.getUInt());
  ss = s[1];
  checkBuild(ss, JasonType::SmallInt, 1);
  ASSERT_EQ(2ULL, ss.getUInt());

  checkDump(s, value);
}

TEST(ParserTest, Array3) {
  std::string const value("[-1,2, 4.5, 3, -99.99]");
  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Array, 34);
  ASSERT_EQ(5ULL, s.length());

  JasonSlice ss = s[0];
  checkBuild(ss, JasonType::SmallInt, 1);
  ASSERT_EQ(-1LL, ss.getInt());

  ss = s[1];
  checkBuild(ss, JasonType::SmallInt, 1);
  ASSERT_EQ(2ULL, ss.getUInt());

  ss = s[2];
  checkBuild(ss, JasonType::Double, 9);
  ASSERT_EQ(4.5, ss.getDouble());

  ss = s[3];
  checkBuild(ss, JasonType::SmallInt, 1);
  ASSERT_EQ(3ULL, ss.getUInt());

  ss = s[4];
  checkBuild(ss, JasonType::Double, 9);
  ASSERT_EQ(-99.99, ss.getDouble());

  std::string const valueOut = "[-1,2,4.5,3,-99.99]";
  checkDump(s, valueOut);
}

TEST(ParserTest, Array4) {
  std::string const value("[\"foo\", \"bar\", \"baz\", null, true, false, -42.23 ]");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Array, 41);
  ASSERT_EQ(7ULL, s.length());

  JasonSlice ss = s[0];
  checkBuild(ss, JasonType::String, 4);
  std::string correct = "foo";
  ASSERT_EQ(correct, ss.copyString());

  ss = s[1];
  checkBuild(ss, JasonType::String, 4);
  correct = "bar";
  ASSERT_EQ(correct, ss.copyString());

  ss = s[2];
  checkBuild(ss, JasonType::String, 4);
  correct = "baz";
  ASSERT_EQ(correct, ss.copyString());

  ss = s[3];
  checkBuild(ss, JasonType::Null, 1);

  ss = s[4];
  checkBuild(ss, JasonType::Bool, 1);
  ASSERT_TRUE(ss.getBool());

  ss = s[5];
  checkBuild(ss, JasonType::Bool, 1);
  ASSERT_FALSE(ss.getBool());

  ss = s[6];
  checkBuild(ss, JasonType::Double, 9);
  ASSERT_EQ(-42.23, ss.getDouble());

  std::string const valueOut = "[\"foo\",\"bar\",\"baz\",null,true,false,-42.23]";
  checkDump(s, valueOut);
}

TEST(ParserTest, NestedArray1) {
  std::string const value("[ [ ] ]");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Array, 5);
  ASSERT_EQ(1ULL, s.length());

  JasonSlice ss = s[0];
  checkBuild(ss, JasonType::Array, 2);
  ASSERT_EQ(0ULL, ss.length());

  std::string const valueOut = "[[]]";
  checkDump(s, valueOut);
}

TEST(ParserTest, NestedArray2) {
  std::string const value("[ [ ],[[]],[],[ [[ [], [ ], [ ] ], [ ] ] ], [] ]");
  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Array, 45);
  ASSERT_EQ(5ULL, s.length());

  JasonSlice ss = s[0];
  checkBuild(ss, JasonType::Array, 2);
  ASSERT_EQ(0ULL, ss.length());

  ss = s[1];
  checkBuild(ss, JasonType::Array, 5);
  ASSERT_EQ(1ULL, ss.length());

  JasonSlice sss = ss[0];
  checkBuild(sss, JasonType::Array, 2);
  ASSERT_EQ(0ULL, sss.length());

  ss = s[2];
  checkBuild(ss, JasonType::Array, 2);
  ASSERT_EQ(0ULL, ss.length());

  ss = s[3];
  checkBuild(ss, JasonType::Array, 21);
  ASSERT_EQ(1ULL, ss.length());

  sss = ss[0];
  checkBuild(sss, JasonType::Array, 18);
  ASSERT_EQ(2ULL, sss.length());

  JasonSlice ssss = sss[0];
  checkBuild(ssss, JasonType::Array, 9);
  ASSERT_EQ(3ULL, ssss.length());

  JasonSlice sssss = ssss[0];
  checkBuild(sssss, JasonType::Array, 2);
  ASSERT_EQ(0ULL, sssss.length());

  sssss = ssss[1];
  checkBuild(sssss, JasonType::Array, 2);
  ASSERT_EQ(0ULL, sssss.length());

  sssss = ssss[2];
  checkBuild(sssss, JasonType::Array, 2);
  ASSERT_EQ(0ULL, sssss.length());

  ssss = sss[1];
  checkBuild(ssss, JasonType::Array, 2);
  ASSERT_EQ(0ULL, ssss.length());

  ss = s[4];
  checkBuild(ss, JasonType::Array, 2);
  ASSERT_EQ(0ULL, ss.length());

  std::string const valueOut = "[[],[[]],[],[[[[],[],[]],[]]],[]]";
  checkDump(s, valueOut);
}

TEST(ParserTest, NestedArray3) {
  std::string const value("[ [ \"foo\", [ \"bar\", \"baz\", null ], true, false ], -42.23 ]");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Array, 51);
  ASSERT_EQ(2ULL, s.length());

  JasonSlice ss = s[0];
  checkBuild(ss, JasonType::Array, 35);
  ASSERT_EQ(4ULL, ss.length());

  JasonSlice sss = ss[0];
  checkBuild(sss, JasonType::String, 4);
  std::string correct = "foo";
  ASSERT_EQ(correct, sss.copyString());

  sss = ss[1];
  checkBuild(sss, JasonType::Array, 18);
  ASSERT_EQ(3ULL, sss.length());

  JasonSlice ssss = sss[0];
  checkBuild(ssss, JasonType::String, 4);
  correct = "bar";
  ASSERT_EQ(correct, ssss.copyString());

  ssss = sss[1];
  checkBuild(ssss, JasonType::String, 4);
  correct = "baz";
  ASSERT_EQ(correct, ssss.copyString());

  ssss = sss[2];
  checkBuild(ssss, JasonType::Null, 1);

  sss = ss[2];
  checkBuild(sss, JasonType::Bool, 1);
  ASSERT_TRUE(sss.getBool());

  sss = ss[3];
  checkBuild(sss, JasonType::Bool, 1);
  ASSERT_FALSE(sss.getBool());

  ss = s[1];
  checkBuild(ss, JasonType::Double, 9);
  ASSERT_EQ(-42.23, ss.getDouble());

  std::string const valueOut = "[[\"foo\",[\"bar\",\"baz\",null],true,false],-42.23]";
  checkDump(s, valueOut);
}

TEST(ParserTest, NestedArrayInvalid1) {
  std::string const value("[ [ ]");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(4u, parser.errorPos());
}

TEST(ParserTest, NestedArrayInvalid2) {
  std::string const value("[ ] ]");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(4u, parser.errorPos());
}

TEST(ParserTest, NestedArrayInvalid3) {
  std::string const value("[ [ \"foo\", [ \"bar\", \"baz\", null ] ]");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(34u, parser.errorPos());
}

TEST(ParserTest, BrokenArray1) {
  std::string const value("[");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(0u, parser.errorPos());
}

TEST(ParserTest, BrokenArray2) {
  std::string const value("[,");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(1u, parser.errorPos());
}

TEST(ParserTest, BrokenArray3) {
  std::string const value("[1,");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(2u, parser.errorPos());
}

TEST(ParserTest, ShortArrayMembers) {
  std::string value("[");
  for (size_t i = 0; i < 255; ++i) {
    if (i > 0) {
      value.push_back(',');
    }
    value.append(std::to_string(i));
  }
  value.push_back(']');

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  ASSERT_EQ(5ULL, s.head()); // short array
  checkBuild(s, JasonType::Array, 1021);
  ASSERT_EQ(255ULL, s.length());
  
  for (size_t i = 0; i < 255; ++i) {
    JasonSlice ss = s[i];
    if (i <= 9) {
      checkBuild(ss, JasonType::SmallInt, 1);
    }
    else {
      checkBuild(ss, JasonType::UInt, 2);
    }
    ASSERT_EQ(i, ss.getUInt());
  }
}

TEST(ParserTest, LongArrayFewMembers) {
  std::string single("0123456789abcdef");
  single.append(single);
  single.append(single);
  single.append(single);
  single.append(single);
  single.append(single);
  single.append(single); // 1024 bytes

  std::string value("[");
  for (size_t i = 0; i < 65; ++i) {
    if (i > 0) {
      value.push_back(',');
    }
    value.push_back('"');
    value.append(single);
    value.push_back('"');
  }
  value.push_back(']');

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  ASSERT_EQ(4ULL, s.head()); // array without index table
  checkBuild(s, JasonType::Array, 67156);
  ASSERT_EQ(65ULL, s.length());
  
  for (size_t i = 0; i < 65; ++i) {
    JasonSlice ss = s[i];
    checkBuild(ss, JasonType::String, 1033);
    JasonLength len;
    char const* s = ss.getString(len);
    ASSERT_EQ(1024ULL, len);
    ASSERT_EQ(0, strncmp(s, single.c_str(), len));
  }
}

TEST(ParserTest, LongArrayManyMembers) {
  std::string value("[");
  for (size_t i = 0; i < 256; ++i) {
    if (i > 0) {
      value.push_back(',');
    }
    value.append(std::to_string(i));
  }
  value.push_back(']');

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  ASSERT_EQ(5ULL, s.head()); // array without index table
  checkBuild(s, JasonType::Array, 1033);
  ASSERT_EQ(256ULL, s.length());
  
  for (size_t i = 0; i < 256; ++i) {
    JasonSlice ss = s[i];
    if (i <= 9) {
      checkBuild(ss, JasonType::SmallInt, 1);
    }
    else {
      checkBuild(ss, JasonType::UInt, 2);
    }
    ASSERT_EQ(i, ss.getUInt());
  }
}

TEST(ParserTest, EmptyObject) {
  std::string const value("{}");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Object, 2);
  ASSERT_EQ(0ULL, s.length());

  checkDump(s, value);
}

TEST(ParserTest, BrokenObject1) {
  std::string const value("{");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(0u, parser.errorPos());
}

TEST(ParserTest, BrokenObject2) {
  std::string const value("{,");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(0u, parser.errorPos());
}

TEST(ParserTest, BrokenObject3) {
  std::string const value("{1,");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(0u, parser.errorPos());
}

TEST(ParserTest, BrokenObject4) {
  std::string const value("{\"foo");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(4u, parser.errorPos());
}

TEST(ParserTest, BrokenObject5) {
  std::string const value("{\"foo\"");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(5u, parser.errorPos());
}

TEST(ParserTest, BrokenObject6) {
  std::string const value("{\"foo\":");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(6u, parser.errorPos());
}

TEST(ParserTest, BrokenObject7) {
  std::string const value("{\"foo\":\"foo");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(10u, parser.errorPos());
}

TEST(ParserTest, BrokenObject8) {
  std::string const value("{\"foo\":\"foo\", ");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(13u, parser.errorPos());
}

TEST(ParserTest, BrokenObject9) {
  std::string const value("{\"foo\":\"foo\", }");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(13u, parser.errorPos());
}

TEST(ParserTest, BrokenObject10) {
  std::string const value("{\"foo\" }");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(6u, parser.errorPos());
}

TEST(ParserTest, ObjectSimple1) {
  std::string const value("{ \"foo\" : 1}");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Object, 8);
  ASSERT_EQ(1ULL, s.length());

  JasonSlice ss = s.keyAt(0);
  checkBuild(ss, JasonType::String, 4);

  std::string correct = "foo";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(0);
  checkBuild(ss, JasonType::SmallInt, 1);
  ASSERT_EQ(1, ss.getSmallInt());

  std::string valueOut = "{\"foo\":1}";
  checkDump(s, valueOut);
}

TEST(ParserTest, ObjectSimple2) {
  std::string const value("{ \"foo\" : \"bar\", \"baz\":true}");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Object, 20);
  ASSERT_EQ(2ULL, s.length());

  JasonSlice ss = s.keyAt(0);
  checkBuild(ss, JasonType::String, 4);
  std::string correct = "baz";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(0);
  checkBuild(ss, JasonType::Bool, 1);
  ASSERT_TRUE(ss.getBool());

  ss = s.keyAt(1);
  checkBuild(ss, JasonType::String, 4);
  correct = "foo";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(1);
  checkBuild(ss, JasonType::String, 4);
  correct = "bar";
  ASSERT_EQ(correct, ss.copyString());

  std::string valueOut = "{\"baz\":true,\"foo\":\"bar\"}";
  checkDump(s, valueOut);
}

TEST(ParserTest, ObjectDenseNotation) {
  std::string const value("{\"a\":\"b\",\"c\":\"d\"}");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Object, 15);
  ASSERT_EQ(2ULL, s.length());

  JasonSlice ss = s.keyAt(0);
  checkBuild(ss, JasonType::String, 2);
  std::string correct = "a";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(0);
  checkBuild(ss, JasonType::String, 2);
  correct = "b";
  ASSERT_EQ(correct, ss.copyString());

  ss = s.keyAt(1);
  checkBuild(ss, JasonType::String, 2);
  correct = "c";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(1);
  checkBuild(ss, JasonType::String, 2);
  correct = "d";
  ASSERT_EQ(correct, ss.copyString());

  checkDump(s, value);
}

TEST(ParserTest, ObjectReservedKeys) {
  std::string const value("{ \"null\" : \"true\", \"false\":\"bar\", \"true\":\"foo\"}");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Object, 38);
  ASSERT_EQ(3ULL, s.length());

  JasonSlice ss = s.keyAt(0);
  checkBuild(ss, JasonType::String, 6);
  std::string correct = "false";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(0);
  checkBuild(ss, JasonType::String, 4);
  correct = "bar";
  ASSERT_EQ(correct, ss.copyString());

  ss = s.keyAt(1);
  checkBuild(ss, JasonType::String, 5);
  correct = "null";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(1);
  checkBuild(ss, JasonType::String, 5);
  correct = "true";
  ASSERT_EQ(correct, ss.copyString());

  ss = s.keyAt(2);
  checkBuild(ss, JasonType::String, 5);
  correct = "true";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(2);
  checkBuild(ss, JasonType::String, 4);
  correct = "foo";
  ASSERT_EQ(correct, ss.copyString());

  std::string const valueOut = "{\"false\":\"bar\",\"null\":\"true\",\"true\":\"foo\"}";
  checkDump(s, valueOut);
}

TEST(ParserTest, ObjectMixed) {
  std::string const value("{\"foo\":null,\"bar\":true,\"baz\":13.53,\"qux\":[1],\"quz\":{}}");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Object, 50);
  ASSERT_EQ(5ULL, s.length());

  JasonSlice ss = s.keyAt(0);
  checkBuild(ss, JasonType::String, 4);
  std::string correct = "bar";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(0);
  checkBuild(ss, JasonType::Bool, 1);
  ASSERT_TRUE(ss.getBool());

  ss = s.keyAt(1);
  checkBuild(ss, JasonType::String, 4);
  correct = "baz";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(1);
  checkBuild(ss, JasonType::Double, 9);
  ASSERT_EQ(13.53, ss.getDouble());

  ss = s.keyAt(2);
  checkBuild(ss, JasonType::String, 4);
  correct = "foo";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(2);
  checkBuild(ss, JasonType::Null, 1);

  ss = s.keyAt(3);
  checkBuild(ss, JasonType::String, 4);
  correct = "qux";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(3);
  checkBuild(ss, JasonType::Array, 4);

  JasonSlice sss = ss[0];
  checkBuild(sss, JasonType::SmallInt, 1);
  ASSERT_EQ(1ULL, sss.getUInt());

  ss = s.keyAt(4);
  checkBuild(ss, JasonType::String, 4);
  correct = "quz";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(4);
  checkBuild(ss, JasonType::Object, 2);
  ASSERT_EQ(0ULL, ss.length());

  std::string const valueOut("{\"bar\":true,\"baz\":13.53,\"foo\":null,\"qux\":[1],\"quz\":{}}");
  checkDump(s, valueOut);
}

TEST(ParserTest, ObjectInvalidQuotes) {
  std::string const value("{'foo':'bar' }");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
}

TEST(ParserTest, ObjectMissingQuotes) {
  std::string const value("{foo:\"bar\" }");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
}

TEST(ParserTest, ShortObjectMembers) {
  std::string value("{");
  for (size_t i = 0; i < 255; ++i) {
    if (i > 0) {
      value.push_back(',');
    }
    value.append("\"test");
    if (i < 100) {
      value.push_back('0');
      if (i < 10) {
        value.push_back('0');
      }
    }
    value.append(std::to_string(i));
    value.append("\":");
    value.append(std::to_string(i));
  }
  value.push_back('}');

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  ASSERT_EQ(8ULL, s.head()); // object with offset size 2
  checkBuild(s, JasonType::Object, 3061);
  ASSERT_EQ(255ULL, s.length());
  
  for (size_t i = 0; i < 255; ++i) {
    JasonSlice sk = s.keyAt(i);
    JasonLength len;
    char const* str = sk.getString(len);
    std::string key("test");
    if (i < 100) {
      key.push_back('0');
      if (i < 10) {
        key.push_back('0');
      }
    }
    key.append(std::to_string(i));

    ASSERT_EQ(key.size(), len);
    ASSERT_EQ(0, strncmp(str, key.c_str(), len));
    JasonSlice sv = s.valueAt(i);
    if (i <= 9) {
      checkBuild(sv, JasonType::SmallInt, 1);
    }
    else {
      checkBuild(sv, JasonType::UInt, 2);
    }
    ASSERT_EQ(i, sv.getUInt());
  }
}

TEST(ParserTest, LongObjectFewMembers) {
  std::string single("0123456789abcdef");
  single.append(single);
  single.append(single);
  single.append(single);
  single.append(single);
  single.append(single);
  single.append(single); // 1024 bytes

  std::string value("{");
  for (size_t i = 0; i < 64; ++i) {
    if (i > 0) {
      value.push_back(',');
    }
    value.append("\"test");
    if (i < 100) {
      value.push_back('0');
      if (i < 10) {
        value.push_back('0');
      }
    }
    value.append(std::to_string(i));
    value.append("\":\"");
    value.append(single);
    value.push_back('\"');
  }
  value.push_back('}');

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  ASSERT_EQ(9ULL, s.head()); // object with offset size 4
  checkBuild(s, JasonType::Object, 66891);
  ASSERT_EQ(64ULL, s.length());
  
  for (size_t i = 0; i < 64; ++i) {
    JasonSlice sk = s.keyAt(i);
    JasonLength len;
    char const* str = sk.getString(len);
    std::string key("test");
    if (i < 100) {
      key.push_back('0');
      if (i < 10) {
        key.push_back('0');
      }
    }
    key.append(std::to_string(i));

    ASSERT_EQ(key.size(), len);
    ASSERT_EQ(0, strncmp(str, key.c_str(), len));
    JasonSlice sv = s.valueAt(i);
    str = sv.getString(len);
    ASSERT_EQ(1024ULL, len);
    ASSERT_EQ(0, strncmp(str, single.c_str(), len));
  }
}

TEST(ParserTest, LongObjectManyMembers) {
  std::string value("{");
  for (size_t i = 0; i < 256; ++i) {
    if (i > 0) {
      value.push_back(',');
    }
    value.append("\"test");
    if (i < 100) {
      value.push_back('0');
      if (i < 10) {
        value.push_back('0');
      }
    }
    value.append(std::to_string(i));
    value.append("\":");
    value.append(std::to_string(i));
  }
  value.push_back('}');

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  ASSERT_EQ(8ULL, s.head()); // long object
  checkBuild(s, JasonType::Object, 3081);
  ASSERT_EQ(256ULL, s.length());
  
  for (size_t i = 0; i < 256; ++i) {
    JasonSlice sk = s.keyAt(i);
    JasonLength len;
    char const* str = sk.getString(len);
    std::string key("test");
    if (i < 100) {
      key.push_back('0');
      if (i < 10) {
        key.push_back('0');
      }
    }
    key.append(std::to_string(i));

    ASSERT_EQ(key.size(), len);
    ASSERT_EQ(0, strncmp(str, key.c_str(), len));
    JasonSlice sv = s.valueAt(i);
    if (i <= 9) {
      checkBuild(sv, JasonType::SmallInt, 1);
    }
    else {
      checkBuild(sv, JasonType::UInt, 2);
    }
    ASSERT_EQ(i, sv.getUInt());
  }
}

TEST(ParserTest, Utf8Bom) {
  std::string const value("\xef\xbb\xbf{\"foo\":1}");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Object, 8);
  ASSERT_EQ(1ULL, s.length());

  JasonSlice ss = s.keyAt(0);
  checkBuild(ss, JasonType::String, 4);
  std::string correct = "foo";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(0);
  checkBuild(ss, JasonType::SmallInt, 1);
  ASSERT_EQ(1ULL, ss.getUInt());

  std::string valueOut = "{\"foo\":1}";
  checkDump(s, valueOut);
}

TEST(ParserTest, Utf8BomBroken) {
  std::string const value("\xef\xbb");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
}

TEST(ParserTest, DuplicateAttributesAllowed) {
  std::string const value("{\"foo\":1,\"foo\":2}");

  JasonParser parser;
  parser.parse(value);
  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());

  JasonSlice v = s.get("foo");
  ASSERT_TRUE(v.isNumber());
  ASSERT_EQ(1ULL, v.getUInt());
}

TEST(ParserTest, DuplicateAttributesDisallowed) {
  std::string const value("{\"foo\":1,\"foo\":2}");

  JasonParser parser;
  parser.options.checkAttributeUniqueness = true;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::DuplicateAttributeName);
}

TEST(ParserTest, DuplicateAttributesDisallowedUnsortedObject) {
  std::string const value("{\"foo\":1,\"bar\":3,\"foo\":2}");

  JasonParser parser;
  parser.options.sortAttributeNames = false;
  parser.options.checkAttributeUniqueness = true;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::DuplicateAttributeName);
}

TEST(ParserTest, DuplicateSubAttributesAllowed) {
  std::string const value("{\"foo\":{\"bar\":1},\"baz\":{\"bar\":2},\"bar\":{\"foo\":23,\"baz\":9}}");

  JasonParser parser;
  parser.options.checkAttributeUniqueness = true;
  parser.parse(value);
  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  JasonSlice v = s.get(std::vector<std::string>({ "foo", "bar" })); 
  ASSERT_TRUE(v.isNumber());
  ASSERT_EQ(1ULL, v.getUInt());
}

TEST(ParserTest, DuplicateSubAttributesDisallowed) {
  std::string const value("{\"roo\":{\"bar\":1,\"abc\":true,\"def\":7,\"abc\":2}}");

  JasonParser parser;
  parser.options.checkAttributeUniqueness = true;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::DuplicateAttributeName);
}

TEST(LookupTest, LookupShortObject) {
  std::string const value("{\"foo\":null,\"bar\":true,\"baz\":13.53,\"qux\":[1],\"quz\":{}}");

  JasonParser parser;
  parser.parse(value);
  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());

  JasonSlice v;
  v = s.get("foo"); 
  ASSERT_TRUE(v.isNull());
 
  v = s.get("bar");  
  ASSERT_TRUE(v.isBool());
  ASSERT_EQ(true, v.getBool());

  v = s.get("baz");  
  ASSERT_TRUE(v.isDouble());
  ASSERT_DOUBLE_EQ(13.53, v.getDouble());

  v = s.get("qux");  
  ASSERT_TRUE(v.isArray());
  ASSERT_TRUE(v.isType(JasonType::Array));
  ASSERT_EQ(1ULL, v.length());

  v = s.get("quz");  
  ASSERT_TRUE(v.isObject());
  ASSERT_TRUE(v.isType(JasonType::Object));
  ASSERT_EQ(0ULL, v.length());

  // non-present attributes
  v = s.get("nada");  
  ASSERT_TRUE(v.isNone());

  v = s.get(std::string("foo\0", 4));  
  ASSERT_TRUE(v.isNone());

  v = s.get("Foo");  
  ASSERT_TRUE(v.isNone());

  v = s.get("food");  
  ASSERT_TRUE(v.isNone());

  v = s.get("");  
  ASSERT_TRUE(v.isNone());
}

TEST(LookupTest, LookupSubattributes) {
  std::string const value("{\"foo\":{\"bar\":1,\"bark\":[],\"baz\":{\"qux\":{\"qurz\":null}}}}");

  JasonParser parser;
  parser.parse(value);
  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());

  JasonSlice v;
  v = s.get(std::vector<std::string>({ "foo" })); 
  ASSERT_TRUE(v.isObject());
 
  v = s.get(std::vector<std::string>({ "foo", "bar" })); 
  ASSERT_TRUE(v.isNumber());
  ASSERT_EQ(1ULL, v.getUInt());

  v = s.get(std::vector<std::string>({ "boo" })); 
  ASSERT_TRUE(v.isNone());

  v = s.get(std::vector<std::string>({ "boo", "far" })); 
  ASSERT_TRUE(v.isNone());

  v = s.get(std::vector<std::string>({ "foo", "bark" })); 
  ASSERT_TRUE(v.isArray());

  v = s.get(std::vector<std::string>({ "foo", "bark", "baz" })); 
  ASSERT_TRUE(v.isNone());

  v = s.get(std::vector<std::string>({ "foo", "baz" })); 
  ASSERT_TRUE(v.isObject());

  v = s.get(std::vector<std::string>({ "foo", "baz", "qux" })); 
  ASSERT_TRUE(v.isObject());

  v = s.get(std::vector<std::string>({ "foo", "baz", "qux", "qurz" })); 
  ASSERT_TRUE(v.isNull());

  v = s.get(std::vector<std::string>({ "foo", "baz", "qux", "qurk" })); 
  ASSERT_TRUE(v.isNone());

  v = s.get(std::vector<std::string>({ "foo", "baz", "qux", "qurz", "p0rk" })); 
  ASSERT_TRUE(v.isNone());
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
  ASSERT_TRUE(v.isNumber());
  ASSERT_EQ(4ULL, v.getUInt());

  v = s.get("test10"); 
  ASSERT_TRUE(v.isNumber());
  ASSERT_EQ(10ULL, v.getUInt());

  v = s.get("test42"); 
  ASSERT_TRUE(v.isNumber());
  ASSERT_EQ(42ULL, v.getUInt());

  v = s.get("test100"); 
  ASSERT_TRUE(v.isNumber());
  ASSERT_EQ(100ULL, v.getUInt());
  
  v = s.get("test932"); 
  ASSERT_TRUE(v.isNumber());
  ASSERT_EQ(932ULL, v.getUInt());

  v = s.get("test1000"); 
  ASSERT_TRUE(v.isNumber());
  ASSERT_EQ(1000ULL, v.getUInt());

  v = s.get("test1023"); 
  ASSERT_TRUE(v.isNumber());
  ASSERT_EQ(1023ULL, v.getUInt());

  // none existing
  v = s.get("test0"); 
  ASSERT_TRUE(v.isNone());

  v = s.get("test1"); 
  ASSERT_TRUE(v.isNone());

  v = s.get("test1024"); 
  ASSERT_TRUE(v.isNone());
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
  ASSERT_TRUE(v.isNumber());
  ASSERT_EQ(0ULL, v.getUInt());
  
  v = s.get("test1"); 
  ASSERT_TRUE(v.isNumber());
  ASSERT_EQ(1ULL, v.getUInt());

  v = s.get("test2"); 
  ASSERT_TRUE(v.isNumber());
  ASSERT_EQ(2ULL, v.getUInt());
  
  v = s.get("test3"); 
  ASSERT_TRUE(v.isNumber());
  ASSERT_EQ(3ULL, v.getUInt());
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
  
    ASSERT_TRUE(v.isNumber());
    ASSERT_EQ(i, v.getUInt());
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
  
    ASSERT_TRUE(v.isNumber());
    ASSERT_EQ(i, v.getUInt());
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
  
    ASSERT_TRUE(v.isNumber());
    ASSERT_EQ(i, v.getUInt());
  } 
}

int main (int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

