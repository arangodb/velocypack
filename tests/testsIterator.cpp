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

#include <string>

#include "tests-common.h"
  
TEST(IteratorTest, IterateNonArray1) {
  std::string const value("null");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  EXPECT_VELOCYPACK_EXCEPTION(ArrayIterator(s), Exception::InvalidValueType);
}

TEST(IteratorTest, IterateNonArray2) {
  std::string const value("true");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  EXPECT_VELOCYPACK_EXCEPTION(ArrayIterator(s), Exception::InvalidValueType);
}

TEST(IteratorTest, IterateNonArray3) {
  std::string const value("1");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  EXPECT_VELOCYPACK_EXCEPTION(ArrayIterator(s), Exception::InvalidValueType);
}

TEST(IteratorTest, IterateNonArray4) {
  std::string const value("\"abc\"");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  EXPECT_VELOCYPACK_EXCEPTION(ArrayIterator(s), Exception::InvalidValueType);
}

TEST(IteratorTest, IterateNonArray5) {
  std::string const value("{}");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  EXPECT_VELOCYPACK_EXCEPTION(ArrayIterator(s), Exception::InvalidValueType);
}

TEST(IteratorTest, IterateArrayEmpty) {
  std::string const value("[]");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ArrayIterator it(s);
  EXPECT_FALSE(it.valid());
  
  EXPECT_VELOCYPACK_EXCEPTION(it.value(), Exception::IndexOutOfBounds);
  
  EXPECT_FALSE(it.next());
}

TEST(IteratorTest, IterateArray) {
  std::string const value("[1,2,3,4,null,true,\"foo\",\"bar\"]");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ArrayIterator it(s);

  EXPECT_TRUE(it.valid());
  Slice current = it.value();
  EXPECT_TRUE(current.isNumber());  
  EXPECT_EQ(1UL, current.getUInt());

  EXPECT_TRUE(it.next());
  
  EXPECT_TRUE(it.valid());
  current = it.value();
  EXPECT_TRUE(current.isNumber());  
  EXPECT_EQ(2UL, current.getUInt());
  
  EXPECT_TRUE(it.next());
  
  EXPECT_TRUE(it.valid());
  current = it.value();
  EXPECT_TRUE(current.isNumber());  
  EXPECT_EQ(3UL, current.getUInt());
  
  EXPECT_TRUE(it.next());
  
  EXPECT_TRUE(it.valid());
  current = it.value();
  EXPECT_TRUE(current.isNumber());  
  EXPECT_EQ(4UL, current.getUInt());
  
  EXPECT_TRUE(it.next());
  
  EXPECT_TRUE(it.valid());
  current = it.value();
  EXPECT_TRUE(current.isNull());
  
  EXPECT_TRUE(it.next());
  
  EXPECT_TRUE(it.valid());
  current = it.value();
  EXPECT_TRUE(current.isBool());
  EXPECT_TRUE(current.getBool());
  
  EXPECT_TRUE(it.next());
  
  EXPECT_TRUE(it.valid());
  current = it.value();
  EXPECT_TRUE(current.isString());
  EXPECT_EQ("foo", current.copyString());
  
  EXPECT_TRUE(it.next());
  
  EXPECT_TRUE(it.valid());
  current = it.value();
  EXPECT_TRUE(current.isString());
  EXPECT_EQ("bar", current.copyString());
  
  EXPECT_FALSE(it.next());
  EXPECT_FALSE(it.valid());

  EXPECT_VELOCYPACK_EXCEPTION(it.value(), Exception::IndexOutOfBounds);
}

TEST(IteratorTest, IterateSubArray) {
  std::string const value("[[1,2,3],[\"foo\",\"bar\"]]");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ArrayIterator it(s);

  EXPECT_TRUE(it.valid());
  Slice current = it.value();
  EXPECT_TRUE(current.isArray());  

  ArrayIterator it2(current);
  EXPECT_TRUE(it2.valid());
  Slice sub = it2.value();
  EXPECT_TRUE(sub.isNumber());
  EXPECT_EQ(1UL, sub.getUInt());

  EXPECT_TRUE(it2.next());
  
  EXPECT_TRUE(it2.valid());
  sub = it2.value();
  EXPECT_TRUE(sub.isNumber());  
  EXPECT_EQ(2UL, sub.getUInt());
  
  EXPECT_TRUE(it2.next());
  
  EXPECT_TRUE(it2.valid());
  sub = it2.value();
  EXPECT_TRUE(sub.isNumber());  
  EXPECT_EQ(3UL, sub.getUInt());
  
  EXPECT_FALSE(it2.next());
  EXPECT_FALSE(it2.valid());
  EXPECT_VELOCYPACK_EXCEPTION(it2.value(), Exception::IndexOutOfBounds);
  
  EXPECT_TRUE(it.next());

  EXPECT_TRUE(it.valid());
  current = it.value();
  EXPECT_TRUE(current.isArray());  

  it2 = ArrayIterator(current);

  EXPECT_TRUE(it2.valid());
  sub = it2.value();
  EXPECT_TRUE(sub.isString());
  EXPECT_EQ("foo", sub.copyString());

  EXPECT_TRUE(it2.next());
  
  EXPECT_TRUE(it2.valid());
  sub = it2.value();
  EXPECT_TRUE(sub.isString());
  EXPECT_EQ("bar", sub.copyString());
  
  EXPECT_FALSE(it2.next());
  EXPECT_FALSE(it2.valid());
  EXPECT_VELOCYPACK_EXCEPTION(it2.value(), Exception::IndexOutOfBounds);

  EXPECT_FALSE(it.next());
  EXPECT_FALSE(it.valid());
  EXPECT_VELOCYPACK_EXCEPTION(it.value(), Exception::IndexOutOfBounds);
}

TEST(IteratorTest, IterateNonObject1) {
  std::string const value("null");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  EXPECT_VELOCYPACK_EXCEPTION(ObjectIterator(s), Exception::InvalidValueType);
}

TEST(IteratorTest, IterateNonObject2) {
  std::string const value("true");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  EXPECT_VELOCYPACK_EXCEPTION(ObjectIterator(s), Exception::InvalidValueType);
}

TEST(IteratorTest, IterateNonObject3) {
  std::string const value("1");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  EXPECT_VELOCYPACK_EXCEPTION(ObjectIterator(s), Exception::InvalidValueType);
}

TEST(IteratorTest, IterateNonObject4) {
  std::string const value("\"abc\"");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  EXPECT_VELOCYPACK_EXCEPTION(ObjectIterator(s), Exception::InvalidValueType);
}

TEST(IteratorTest, IterateNonObject5) {
  std::string const value("[]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  EXPECT_VELOCYPACK_EXCEPTION(ObjectIterator(s), Exception::InvalidValueType);
}

TEST(IteratorTest, IterateObjectEmpty) {
  std::string const value("{}");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ObjectIterator it(s);
  EXPECT_FALSE(it.valid());
  
  EXPECT_VELOCYPACK_EXCEPTION(it.key(), Exception::IndexOutOfBounds);
  EXPECT_VELOCYPACK_EXCEPTION(it.value(), Exception::IndexOutOfBounds);
  
  EXPECT_FALSE(it.next());
}

TEST(IteratorTest, IterateObject) {
  std::string const value("{\"a\":1,\"b\":2,\"c\":3,\"d\":4,\"e\":null,\"f\":true,\"g\":\"foo\",\"h\":\"bar\"}");

  Parser parser;
  parser.options.sortAttributeNames = false;
  parser.parse(value);
  Slice s(parser.start());

  ObjectIterator it(s);

  EXPECT_TRUE(it.valid());
  Slice key = it.key();
  Slice current = it.value();
  EXPECT_EQ("a", key.copyString());
  EXPECT_TRUE(current.isNumber());  
  EXPECT_EQ(1UL, current.getUInt());

  EXPECT_TRUE(it.next());
  
  EXPECT_TRUE(it.valid());
  key = it.key();
  current = it.value();
  EXPECT_EQ("b", key.copyString());
  EXPECT_TRUE(current.isNumber());  
  EXPECT_EQ(2UL, current.getUInt());
  
  EXPECT_TRUE(it.next());
  
  EXPECT_TRUE(it.valid());
  key = it.key();
  current = it.value();
  EXPECT_EQ("c", key.copyString());
  EXPECT_TRUE(current.isNumber());  
  EXPECT_EQ(3UL, current.getUInt());
  
  EXPECT_TRUE(it.next());
  
  EXPECT_TRUE(it.valid());
  key = it.key();
  current = it.value();
  EXPECT_EQ("d", key.copyString());
  EXPECT_TRUE(current.isNumber());  
  EXPECT_EQ(4UL, current.getUInt());
  
  EXPECT_TRUE(it.next());
  
  EXPECT_TRUE(it.valid());
  key = it.key();
  current = it.value();
  EXPECT_EQ("e", key.copyString());
  EXPECT_TRUE(current.isNull());
  
  EXPECT_TRUE(it.next());
  
  EXPECT_TRUE(it.valid());
  key = it.key();
  current = it.value();
  EXPECT_EQ("f", key.copyString());
  EXPECT_TRUE(current.isBool());
  EXPECT_TRUE(current.getBool());
  
  EXPECT_TRUE(it.next());
  
  EXPECT_TRUE(it.valid());
  key = it.key();
  current = it.value();
  EXPECT_EQ("g", key.copyString());
  EXPECT_TRUE(current.isString());
  EXPECT_EQ("foo", current.copyString());
  
  EXPECT_TRUE(it.next());
  
  EXPECT_TRUE(it.valid());
  key = it.key();
  current = it.value();
  EXPECT_EQ("h", key.copyString());
  EXPECT_TRUE(current.isString());
  EXPECT_EQ("bar", current.copyString());
  
  EXPECT_FALSE(it.next());
  EXPECT_FALSE(it.valid());

  EXPECT_VELOCYPACK_EXCEPTION(it.key(), Exception::IndexOutOfBounds);
  EXPECT_VELOCYPACK_EXCEPTION(it.value(), Exception::IndexOutOfBounds);
}

int main (int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

