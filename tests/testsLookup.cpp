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
  
TEST(LookupTest, LookupShortObject) {
  std::string const value("{\"foo\":null,\"bar\":true,\"baz\":13.53,\"qux\":[1],\"quz\":{}}");

  Parser parser;
  parser.parse(value);
  Builder builder = parser.steal();
  Slice s(builder.start());

  Slice v;
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
  ASSERT_TRUE(v.isType(ValueType::Array));
  ASSERT_EQ(1ULL, v.length());

  v = s.get("quz");  
  ASSERT_TRUE(v.isObject());
  ASSERT_TRUE(v.isType(ValueType::Object));
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

  Parser parser;
  parser.parse(value);
  Builder builder = parser.steal();
  Slice s(builder.start());

  Slice v;
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

  Parser parser;
  parser.parse(value);
  Builder builder = parser.steal();
  Slice s(builder.start());

  Slice v;
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

  Parser parser;
  parser.parse(value);
  Builder builder = parser.steal();
  Slice s(builder.start());

  Slice v;
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

  Parser parser;
  parser.parse(value);
  Builder builder = parser.steal();
  Slice s(builder.start());

  for (size_t i = 0; i < 128; ++i) {
    std::string key = "test";
    key.append(std::to_string(i));
    Slice v = s.get(key);
  
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

  Parser parser;
  parser.parse(value);
  Builder builder = parser.steal();
  Slice s(builder.start());

  for (size_t i = 0; i < 1127; ++i) {
    std::string key = "test";
    key.append(std::to_string(i));
    Slice v = s.get(key);
  
    ASSERT_TRUE(v.isNumber());
    ASSERT_EQ(i, v.getUInt());
  } 
}

int main (int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

