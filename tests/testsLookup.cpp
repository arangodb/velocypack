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
#include <fstream>
#include <string>

#include "tests-common.h"

TEST(LookupTest, HasKeyShortObject) {
  std::string const value("{\"foo\":null,\"bar\":true,\"baz\":13.53,\"qux\":[1],\"quz\":{}}");

  Parser parser;
  parser.parse(value);
  Builder builder = parser.steal();
  Slice s(builder.start());

  EXPECT_TRUE(s.hasKey("foo"));
  EXPECT_TRUE(s.hasKey("bar"));
  EXPECT_TRUE(s.hasKey("baz"));
  EXPECT_TRUE(s.hasKey("qux"));
  EXPECT_TRUE(s.hasKey("quz"));
  EXPECT_FALSE(s.hasKey("nada"));
  EXPECT_FALSE(s.hasKey("Foo"));
  EXPECT_FALSE(s.hasKey("food"));
  EXPECT_FALSE(s.hasKey("quxx"));
  EXPECT_FALSE(s.hasKey("q"));
  EXPECT_FALSE(s.hasKey(""));
}

TEST(LookupTest, HasKeyLongObject) {
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

  Parser parser;
  parser.parse(value);
  Builder builder = parser.steal();
  Slice s(builder.start());

  EXPECT_TRUE(s.hasKey("test4")); 
  EXPECT_TRUE(s.hasKey("test10")); 
  EXPECT_TRUE(s.hasKey("test42")); 
  EXPECT_TRUE(s.hasKey("test100")); 
  EXPECT_TRUE(s.hasKey("test932")); 
  EXPECT_TRUE(s.hasKey("test1000")); 
  EXPECT_TRUE(s.hasKey("test1023")); 
  EXPECT_FALSE(s.hasKey("test0")); 
  EXPECT_FALSE(s.hasKey("test1")); 
  EXPECT_FALSE(s.hasKey("test2")); 
  EXPECT_FALSE(s.hasKey("test3")); 
  EXPECT_FALSE(s.hasKey("test1024")); 
}

TEST(LookupTest, HasKeySubattributes) {
  std::string const value("{\"foo\":{\"bar\":1,\"bark\":[],\"baz\":{\"qux\":{\"qurz\":null}}}}");

  Parser parser;
  parser.parse(value);
  Builder builder = parser.steal();
  Slice s(builder.start());

  EXPECT_TRUE(s.hasKey(std::vector<std::string>({ "foo" }))); 
  EXPECT_TRUE(s.hasKey(std::vector<std::string>({ "foo", "bar" }))); 
  EXPECT_FALSE(s.hasKey(std::vector<std::string>({ "boo" }))); 
  EXPECT_FALSE(s.hasKey(std::vector<std::string>({ "boo", "far" }))); 
  EXPECT_TRUE(s.hasKey(std::vector<std::string>({ "foo", "bark" }))); 
  EXPECT_FALSE(s.hasKey(std::vector<std::string>({ "foo", "bark", "baz" }))); 
  EXPECT_TRUE(s.hasKey(std::vector<std::string>({ "foo", "baz" }))); 
  EXPECT_TRUE(s.hasKey(std::vector<std::string>({ "foo", "baz", "qux" }))); 
  EXPECT_TRUE(s.hasKey(std::vector<std::string>({ "foo", "baz", "qux", "qurz" }))); 
  EXPECT_FALSE(s.hasKey(std::vector<std::string>({ "foo", "baz", "qux", "qurk" }))); 
  EXPECT_FALSE(s.hasKey(std::vector<std::string>({ "foo", "baz", "qux", "qurz", "p0rk" }))); 
}
  
TEST(LookupTest, LookupShortObject) {
  std::string const value("{\"foo\":null,\"bar\":true,\"baz\":13.53,\"qux\":[1],\"quz\":{}}");

  Parser parser;
  parser.parse(value);
  Builder builder = parser.steal();
  Slice s(builder.start());

  Slice v;
  v = s.get("foo"); 
  EXPECT_TRUE(v.isNull());
 
  v = s.get("bar");  
  EXPECT_TRUE(v.isBool());
  EXPECT_EQ(true, v.getBool());

  v = s.get("baz");  
  EXPECT_TRUE(v.isDouble());
  EXPECT_DOUBLE_EQ(13.53, v.getDouble());

  v = s.get("qux");  
  EXPECT_TRUE(v.isArray());
  EXPECT_TRUE(v.isType(ValueType::Array));
  EXPECT_EQ(1ULL, v.length());

  v = s.get("quz");  
  EXPECT_TRUE(v.isObject());
  EXPECT_TRUE(v.isType(ValueType::Object));
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

  Parser parser;
  parser.parse(value);
  Builder builder = parser.steal();
  Slice s(builder.start());

  Slice v;
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

  Parser parser;
  parser.parse(value);
  Builder builder = parser.steal();
  Slice s(builder.start());

  Slice v;
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

TEST(LookupTest, LookupLongObjectUnsorted) {
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

  Parser parser;
  parser.options.sortAttributeNames = false;
  parser.parse(value);
  Builder builder = parser.steal();
  Slice s(builder.start());

  Slice v;
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

  Parser parser;
  parser.parse(value);
  Builder builder = parser.steal();
  Slice s(builder.start());

  Slice v;
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

  Parser parser;
  parser.parse(value);
  Builder builder = parser.steal();
  Slice s(builder.start());

  for (size_t i = 0; i < 128; ++i) {
    std::string key = "test";
    key.append(std::to_string(i));
    Slice v = s.get(key);
  
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

  Parser parser;
  parser.parse(value);
  Builder builder = parser.steal();
  Slice s(builder.start());

  for (size_t i = 0; i < 128; ++i) {
    std::string key = "test";
    for (size_t j = 0; j < i; ++j) {
      key.append("x");
    }
    Slice v = s.get(key);
  
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

  Parser parser;
  parser.parse(value);
  Builder builder = parser.steal();
  Slice s(builder.start());

  for (size_t i = 0; i < 1127; ++i) {
    std::string key = "test";
    key.append(std::to_string(i));
    Slice v = s.get(key);
  
    EXPECT_TRUE(v.isNumber());
    EXPECT_EQ(i, v.getUInt());
  } 
}

int main (int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

