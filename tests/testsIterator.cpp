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

  ASSERT_VELOCYPACK_EXCEPTION(ArrayIterator(s), Exception::InvalidValueType);
}

TEST(IteratorTest, IterateNonArray2) {
  std::string const value("true");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ASSERT_VELOCYPACK_EXCEPTION(ArrayIterator(s), Exception::InvalidValueType);
}

TEST(IteratorTest, IterateNonArray3) {
  std::string const value("1");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ASSERT_VELOCYPACK_EXCEPTION(ArrayIterator(s), Exception::InvalidValueType);
}

TEST(IteratorTest, IterateNonArray4) {
  std::string const value("\"abc\"");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ASSERT_VELOCYPACK_EXCEPTION(ArrayIterator(s), Exception::InvalidValueType);
}

TEST(IteratorTest, IterateNonArray5) {
  std::string const value("{}");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ASSERT_VELOCYPACK_EXCEPTION(ArrayIterator(s), Exception::InvalidValueType);
}

TEST(IteratorTest, IterateArrayEmpty) {
  std::string const value("[]");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ArrayIterator it(s);
  ASSERT_FALSE(it.valid());
  
  ASSERT_VELOCYPACK_EXCEPTION(it.value(), Exception::IndexOutOfBounds);
  
  ASSERT_FALSE(it.next());
}

TEST(IteratorTest, IterateArray) {
  std::string const value("[1,2,3,4,null,true,\"foo\",\"bar\"]");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ArrayIterator it(s);

  ASSERT_TRUE(it.valid());
  Slice current = it.value();
  ASSERT_TRUE(current.isNumber());  
  ASSERT_EQ(1UL, current.getUInt());

  ASSERT_TRUE(it.next());
  
  ASSERT_TRUE(it.valid());
  current = it.value();
  ASSERT_TRUE(current.isNumber());  
  ASSERT_EQ(2UL, current.getUInt());
  
  ASSERT_TRUE(it.next());
  
  ASSERT_TRUE(it.valid());
  current = it.value();
  ASSERT_TRUE(current.isNumber());  
  ASSERT_EQ(3UL, current.getUInt());
  
  ASSERT_TRUE(it.next());
  
  ASSERT_TRUE(it.valid());
  current = it.value();
  ASSERT_TRUE(current.isNumber());  
  ASSERT_EQ(4UL, current.getUInt());
  
  ASSERT_TRUE(it.next());
  
  ASSERT_TRUE(it.valid());
  current = it.value();
  ASSERT_TRUE(current.isNull());
  
  ASSERT_TRUE(it.next());
  
  ASSERT_TRUE(it.valid());
  current = it.value();
  ASSERT_TRUE(current.isBool());
  ASSERT_TRUE(current.getBool());
  
  ASSERT_TRUE(it.next());
  
  ASSERT_TRUE(it.valid());
  current = it.value();
  ASSERT_TRUE(current.isString());
  ASSERT_EQ("foo", current.copyString());
  
  ASSERT_TRUE(it.next());
  
  ASSERT_TRUE(it.valid());
  current = it.value();
  ASSERT_TRUE(current.isString());
  ASSERT_EQ("bar", current.copyString());
  
  ASSERT_FALSE(it.next());
  ASSERT_FALSE(it.valid());

  ASSERT_VELOCYPACK_EXCEPTION(it.value(), Exception::IndexOutOfBounds);
}

TEST(IteratorTest, IterateSubArray) {
  std::string const value("[[1,2,3],[\"foo\",\"bar\"]]");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ArrayIterator it(s);

  ASSERT_TRUE(it.valid());
  Slice current = it.value();
  ASSERT_TRUE(current.isArray());  

  ArrayIterator it2(current);
  ASSERT_TRUE(it2.valid());
  Slice sub = it2.value();
  ASSERT_TRUE(sub.isNumber());
  ASSERT_EQ(1UL, sub.getUInt());

  ASSERT_TRUE(it2.next());
  
  ASSERT_TRUE(it2.valid());
  sub = it2.value();
  ASSERT_TRUE(sub.isNumber());  
  ASSERT_EQ(2UL, sub.getUInt());
  
  ASSERT_TRUE(it2.next());
  
  ASSERT_TRUE(it2.valid());
  sub = it2.value();
  ASSERT_TRUE(sub.isNumber());  
  ASSERT_EQ(3UL, sub.getUInt());
  
  ASSERT_FALSE(it2.next());
  ASSERT_FALSE(it2.valid());
  ASSERT_VELOCYPACK_EXCEPTION(it2.value(), Exception::IndexOutOfBounds);
  
  ASSERT_TRUE(it.next());

  ASSERT_TRUE(it.valid());
  current = it.value();
  ASSERT_TRUE(current.isArray());  

  it2 = ArrayIterator(current);

  ASSERT_TRUE(it2.valid());
  sub = it2.value();
  ASSERT_TRUE(sub.isString());
  ASSERT_EQ("foo", sub.copyString());

  ASSERT_TRUE(it2.next());
  
  ASSERT_TRUE(it2.valid());
  sub = it2.value();
  ASSERT_TRUE(sub.isString());
  ASSERT_EQ("bar", sub.copyString());
  
  ASSERT_FALSE(it2.next());
  ASSERT_FALSE(it2.valid());
  ASSERT_VELOCYPACK_EXCEPTION(it2.value(), Exception::IndexOutOfBounds);

  ASSERT_FALSE(it.next());
  ASSERT_FALSE(it.valid());
  ASSERT_VELOCYPACK_EXCEPTION(it.value(), Exception::IndexOutOfBounds);
}

TEST(IteratorTest, IterateNonObject1) {
  std::string const value("null");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ASSERT_VELOCYPACK_EXCEPTION(ObjectIterator(s), Exception::InvalidValueType);
}

TEST(IteratorTest, IterateNonObject2) {
  std::string const value("true");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ASSERT_VELOCYPACK_EXCEPTION(ObjectIterator(s), Exception::InvalidValueType);
}

TEST(IteratorTest, IterateNonObject3) {
  std::string const value("1");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ASSERT_VELOCYPACK_EXCEPTION(ObjectIterator(s), Exception::InvalidValueType);
}

TEST(IteratorTest, IterateNonObject4) {
  std::string const value("\"abc\"");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ASSERT_VELOCYPACK_EXCEPTION(ObjectIterator(s), Exception::InvalidValueType);
}

TEST(IteratorTest, IterateNonObject5) {
  std::string const value("[]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ASSERT_VELOCYPACK_EXCEPTION(ObjectIterator(s), Exception::InvalidValueType);
}

TEST(IteratorTest, IterateObjectEmpty) {
  std::string const value("{}");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ObjectIterator it(s);
  ASSERT_FALSE(it.valid());
  
  ASSERT_VELOCYPACK_EXCEPTION(it.key(), Exception::IndexOutOfBounds);
  ASSERT_VELOCYPACK_EXCEPTION(it.value(), Exception::IndexOutOfBounds);
  
  ASSERT_FALSE(it.next());
}

TEST(IteratorTest, IterateObject) {
  Options options;
  options.sortAttributeNames = false;

  std::string const value("{\"a\":1,\"b\":2,\"c\":3,\"d\":4,\"e\":null,\"f\":true,\"g\":\"foo\",\"h\":\"bar\"}");

  Parser parser(&options);
  parser.parse(value);
  Slice s(parser.start());

  ObjectIterator it(s);

  ASSERT_TRUE(it.valid());
  Slice key = it.key();
  Slice current = it.value();
  ASSERT_EQ("a", key.copyString());
  ASSERT_TRUE(current.isNumber());  
  ASSERT_EQ(1UL, current.getUInt());

  ASSERT_TRUE(it.next());
  
  ASSERT_TRUE(it.valid());
  key = it.key();
  current = it.value();
  ASSERT_EQ("b", key.copyString());
  ASSERT_TRUE(current.isNumber());  
  ASSERT_EQ(2UL, current.getUInt());
  
  ASSERT_TRUE(it.next());
  
  ASSERT_TRUE(it.valid());
  key = it.key();
  current = it.value();
  ASSERT_EQ("c", key.copyString());
  ASSERT_TRUE(current.isNumber());  
  ASSERT_EQ(3UL, current.getUInt());
  
  ASSERT_TRUE(it.next());
  
  ASSERT_TRUE(it.valid());
  key = it.key();
  current = it.value();
  ASSERT_EQ("d", key.copyString());
  ASSERT_TRUE(current.isNumber());  
  ASSERT_EQ(4UL, current.getUInt());
  
  ASSERT_TRUE(it.next());
  
  ASSERT_TRUE(it.valid());
  key = it.key();
  current = it.value();
  ASSERT_EQ("e", key.copyString());
  ASSERT_TRUE(current.isNull());
  
  ASSERT_TRUE(it.next());
  
  ASSERT_TRUE(it.valid());
  key = it.key();
  current = it.value();
  ASSERT_EQ("f", key.copyString());
  ASSERT_TRUE(current.isBool());
  ASSERT_TRUE(current.getBool());
  
  ASSERT_TRUE(it.next());
  
  ASSERT_TRUE(it.valid());
  key = it.key();
  current = it.value();
  ASSERT_EQ("g", key.copyString());
  ASSERT_TRUE(current.isString());
  ASSERT_EQ("foo", current.copyString());
  
  ASSERT_TRUE(it.next());
  
  ASSERT_TRUE(it.valid());
  key = it.key();
  current = it.value();
  ASSERT_EQ("h", key.copyString());
  ASSERT_TRUE(current.isString());
  ASSERT_EQ("bar", current.copyString());
  
  ASSERT_FALSE(it.next());
  ASSERT_FALSE(it.valid());

  ASSERT_VELOCYPACK_EXCEPTION(it.key(), Exception::IndexOutOfBounds);
  ASSERT_VELOCYPACK_EXCEPTION(it.value(), Exception::IndexOutOfBounds);
}

TEST(IteratorTest, IterateObjectKeys) {
  std::string const value("{\"1foo\":\"bar\",\"2baz\":\"quux\",\"3number\":1,\"4boolean\":true,\"5empty\":null}");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  size_t state = 0;
  ObjectIterator it(s);

  while (it.valid()) {
    Slice key(it.key());
    Slice value(it.value());

    switch (state++) {
      case 0:
        ASSERT_EQ("1foo", key.copyString());
        ASSERT_TRUE(value.isString());
        ASSERT_EQ("bar", value.copyString());
        break;
      case 1:
        ASSERT_EQ("2baz", key.copyString());
        ASSERT_TRUE(value.isString());
        ASSERT_EQ("quux", value.copyString());
        break;
      case 2:
        ASSERT_EQ("3number", key.copyString());
        ASSERT_TRUE(value.isNumber());
        ASSERT_EQ(1ULL, value.getUInt());
        break;
      case 3:
        ASSERT_EQ("4boolean", key.copyString());
        ASSERT_TRUE(value.isBoolean());
        ASSERT_TRUE(value.getBoolean());
        break;
      case 4:
        ASSERT_EQ("5empty", key.copyString());
        ASSERT_TRUE(value.isNull());
        break;
    }
    it.next();
  }

  ASSERT_EQ(5U, state);
}

TEST(IteratorTest, IterateObjectValues) {
  std::string const value("{\"1foo\":\"bar\",\"2baz\":\"quux\",\"3number\":1,\"4boolean\":true,\"5empty\":null}");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  std::vector<std::string> seenKeys;
  ObjectIterator it(s);

  while (it.valid()) {
    seenKeys.emplace_back(it.key().copyString());
    it.next();
  };

  ASSERT_EQ(5U, seenKeys.size());
  ASSERT_EQ("1foo", seenKeys[0]);
  ASSERT_EQ("2baz", seenKeys[1]);
  ASSERT_EQ("3number", seenKeys[2]);
  ASSERT_EQ("4boolean", seenKeys[3]);
  ASSERT_EQ("5empty", seenKeys[4]);
}

TEST(IteratorTest, EmptyArrayIteratorRangeBasedFor) {
  std::string const value("[]");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  size_t seen = 0;
  for (auto it : ArrayIterator(s)) {
    ASSERT_TRUE(false);
    ASSERT_FALSE(it.isNumber()); // only in here to please the compiler
  }
  ASSERT_EQ(0UL, seen);
}

TEST(IteratorTest, ArrayIteratorRangeBasedFor) {
  std::string const value("[1,2,3,4,5]");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  size_t seen = 0;
  for (auto it : ArrayIterator(s)) {
    ASSERT_TRUE(it.isNumber());
    ASSERT_EQ(seen + 1, it.getUInt());
    ++seen;
  }
  ASSERT_EQ(5UL, seen);
}

TEST(IteratorTest, ArrayIteratorRangeBasedForConst) {
  std::string const value("[1,2,3,4,5]");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  size_t seen = 0;
  for (auto const it : ArrayIterator(s)) {
    ASSERT_TRUE(it.isNumber());
    ASSERT_EQ(seen + 1, it.getUInt());
    ++seen;
  }
  ASSERT_EQ(5UL, seen);
}

TEST(IteratorTest, ArrayIteratorRangeBasedForConstRef) {
  std::string const value("[1,2,3,4,5]");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  size_t seen = 0;
  for (auto const& it : ArrayIterator(s)) {
    ASSERT_TRUE(it.isNumber());
    ASSERT_EQ(seen + 1, it.getUInt());
    ++seen;
  }
  ASSERT_EQ(5UL, seen);
}

TEST(IteratorTest, ObjectArrayIteratorRangeBasedFor) {
  std::string const value("{}");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  size_t seen = 0;
  for (auto it : ObjectIterator(s)) {
    ASSERT_TRUE(false);
    ASSERT_FALSE(it.value.isNumber()); // only in here to please the compiler
  }
  ASSERT_EQ(0UL, seen);
}

TEST(IteratorTest, ObjectIteratorRangeBasedFor) {
  std::string const value("{\"1foo\":1,\"2bar\":2,\"3qux\":3}");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  size_t seen = 0;
  for (auto it : ObjectIterator(s)) {
    ASSERT_TRUE(it.key.isString());
    if (seen == 0) {
      ASSERT_EQ("1foo", it.key.copyString());
    }
    else if (seen == 1) {
      ASSERT_EQ("2bar", it.key.copyString());
    }
    else if (seen == 2) {
      ASSERT_EQ("3qux", it.key.copyString());
    }
    ASSERT_TRUE(it.value.isNumber());
    ASSERT_EQ(seen + 1, it.value.getUInt());
    ++seen;
  }
  ASSERT_EQ(3UL, seen);
}

TEST(IteratorTest, ObjectIteratorRangeBasedForConst) {
  std::string const value("{\"1foo\":1,\"2bar\":2,\"3qux\":3}");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  size_t seen = 0;
  for (auto const it : ObjectIterator(s)) {
    ASSERT_TRUE(it.key.isString());
    if (seen == 0) {
      ASSERT_EQ("1foo", it.key.copyString());
    }
    else if (seen == 1) {
      ASSERT_EQ("2bar", it.key.copyString());
    }
    else if (seen == 2) {
      ASSERT_EQ("3qux", it.key.copyString());
    }
    ASSERT_TRUE(it.value.isNumber());
    ASSERT_EQ(seen + 1, it.value.getUInt());
    ++seen;
  }
  ASSERT_EQ(3UL, seen);
}

TEST(IteratorTest, ObjectIteratorRangeBasedForConstRef) {
  std::string const value("{\"1foo\":1,\"2bar\":2,\"3qux\":3}");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  size_t seen = 0;
  for (auto const& it : ObjectIterator(s)) {
    ASSERT_TRUE(it.key.isString());
    if (seen == 0) {
      ASSERT_EQ("1foo", it.key.copyString());
    }
    else if (seen == 1) {
      ASSERT_EQ("2bar", it.key.copyString());
    }
    else if (seen == 2) {
      ASSERT_EQ("3qux", it.key.copyString());
    }
    ASSERT_TRUE(it.value.isNumber());
    ASSERT_EQ(seen + 1, it.value.getUInt());
    ++seen;
  }
  ASSERT_EQ(3UL, seen);
}

int main (int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

