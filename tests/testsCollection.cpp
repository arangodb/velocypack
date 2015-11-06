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
#include <unordered_set>
#include <vector>

#include "tests-common.h"
  
static auto DoNothingCallback = [] (Slice const&, ValueLength) -> bool { return false; };
static auto FailCallback = [] (Slice const&, ValueLength) -> bool { EXPECT_TRUE(false); return false; };

TEST(CollectionTest, KeysNonObject1) {
  std::string const value("null");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  EXPECT_VELOCYPACK_EXCEPTION(Collection::keys(s), Exception::InvalidValueType);
}

TEST(CollectionTest, KeysNonObject2) {
  std::string const value("null");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  std::vector<std::string> result;
  EXPECT_VELOCYPACK_EXCEPTION(Collection::keys(s, result), Exception::InvalidValueType);
}

TEST(CollectionTest, KeysNonObject3) {
  std::string const value("null");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  std::unordered_set<std::string> result;
  EXPECT_VELOCYPACK_EXCEPTION(Collection::keys(s, result), Exception::InvalidValueType);
}
  
TEST(CollectionTest, ObjectKeys1) {
  std::string const value("{\"foo\":1,\"bar\":2,\"baz\":3}");
  Parser parser;
  parser.options.sortAttributeNames = false;
  parser.parse(value);
  Slice s(parser.start());
 
  std::vector<std::string> keys = Collection::keys(s);
  EXPECT_EQ(3UL, keys.size());
  EXPECT_EQ("foo", keys[0]);
  EXPECT_EQ("bar", keys[1]);
  EXPECT_EQ("baz", keys[2]);
}

TEST(CollectionTest, ObjectKeys2) {
  std::string const value("{\"foo\":1,\"bar\":2,\"baz\":3}");
  Parser parser;
  parser.options.sortAttributeNames = false;
  parser.parse(value);
  Slice s(parser.start());
 
  std::vector<std::string> keys;
  Collection::keys(s, keys);
  EXPECT_EQ(3UL, keys.size());
  EXPECT_EQ("foo", keys[0]);
  EXPECT_EQ("bar", keys[1]);
  EXPECT_EQ("baz", keys[2]);
}

TEST(CollectionTest, ObjectKeys3) {
  std::string const value("{\"foo\":1,\"bar\":2,\"baz\":3}");
  Parser parser;
  parser.options.sortAttributeNames = false;
  parser.parse(value);
  Slice s(parser.start());
 
  std::unordered_set<std::string> keys;
  Collection::keys(s, keys);
  EXPECT_EQ(3UL, keys.size());
  EXPECT_TRUE(keys.find("foo") != keys.end());
  EXPECT_TRUE(keys.find("bar") != keys.end());
  EXPECT_TRUE(keys.find("baz") != keys.end());
}

TEST(CollectionTest, ObjectKeys) {
  std::string const value("{\"1foo\":\"bar\",\"2baz\":\"quux\",\"3number\":1,\"4boolean\":true,\"5empty\":null}");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  std::vector<std::string> keys = Collection::keys(s);
  ASSERT_EQ(5U, keys.size());
  ASSERT_EQ("1foo", keys[0]);
  ASSERT_EQ("2baz", keys[1]);
  ASSERT_EQ("3number", keys[2]);
  ASSERT_EQ("4boolean", keys[3]);
  ASSERT_EQ("5empty", keys[4]);
}

TEST(SliceTest, ObjectKeysRef) {
  std::string const value("{\"1foo\":\"bar\",\"2baz\":\"quux\",\"3number\":1,\"4boolean\":true,\"5empty\":null}");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  std::vector<std::string> keys;
  Collection::keys(s, keys);
  ASSERT_EQ(5U, keys.size());
  ASSERT_EQ("1foo", keys[0]);
  ASSERT_EQ("2baz", keys[1]);
  ASSERT_EQ("3number", keys[2]);
  ASSERT_EQ("4boolean", keys[3]);
  ASSERT_EQ("5empty", keys[4]);
}

TEST(CollectionTest, ForEachNonArray) {
  std::string const value("null");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
  
  EXPECT_VELOCYPACK_EXCEPTION(Collection::forEach(s, DoNothingCallback), Exception::InvalidValueType);
}

TEST(CollectionTest, ForEachEmptyArray) {
  std::string const value("[]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
 
  Collection::forEach(s, FailCallback);
}

TEST(CollectionTest, ForEachArray) {
  std::string const value("[1,2,3,\"foo\",\"bar\"]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
 
  size_t seen = 0;
  Collection::forEach(s, [&seen] (Slice const& slice, ValueLength index) -> bool {
    EXPECT_EQ(seen, index);

    switch (seen) {
      case 0:
      case 1:
      case 2:
        EXPECT_TRUE(slice.isNumber());
        break;
      case 3:
      case 4:
        EXPECT_TRUE(slice.isString());
    }

    ++seen;
    return true;
  });

  EXPECT_EQ(5UL, seen);
}

TEST(CollectionTest, ForEachArrayAbort) {
  std::string const value("[1,2,3,\"foo\",\"bar\"]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
 
  size_t seen = 0;
  Collection::forEach(s, [&seen] (Slice const&, ValueLength index) -> bool {
    EXPECT_EQ(seen, index);

    if (seen == 3) {
      return false;
    }
    ++seen;
    return true;
  });

  EXPECT_EQ(3UL, seen);
}

TEST(CollectionTest, FilterNonArray) {
  std::string const value("null");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
 
  EXPECT_VELOCYPACK_EXCEPTION(Collection::filter(s, DoNothingCallback), Exception::InvalidValueType);
}

TEST(CollectionTest, FilterEmptyArray) {
  std::string const value("[]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
 
  Builder b = Collection::filter(s, FailCallback);

  s = b.slice();
  EXPECT_TRUE(s.isArray());
  EXPECT_EQ(0UL, s.length());
}

TEST(CollectionTest, FilterAll) {
  std::string const value("[1,2,3,4,-42,19]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
 
  Builder b = Collection::filter(s, DoNothingCallback);

  s = b.slice();
  EXPECT_TRUE(s.isArray());
  EXPECT_EQ(0UL, s.length());
}

TEST(CollectionTest, FilterArray) {
  std::string const value("[1,2,3,4,-42,19]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
 
  size_t seen = 0;
  Builder b = Collection::filter(s, [&seen] (Slice const& slice, ValueLength index) -> bool {
    EXPECT_EQ(seen, index);
    EXPECT_TRUE(slice.isNumber());

    switch (seen) {
      case 0:
        EXPECT_EQ(1, slice.getInt());
        break;
      case 1:
        EXPECT_EQ(2, slice.getInt());
        break;
      case 2:
        EXPECT_EQ(3, slice.getInt());
        break;
      case 3:
        EXPECT_EQ(4, slice.getInt());
        break;
      case 4:
        EXPECT_EQ(-42, slice.getInt());
        break;
      case 5:
        EXPECT_EQ(19, slice.getInt());
        break;
    }
    ++seen;
    return (index != 4);
  });
  EXPECT_EQ(6UL, seen);

  s = b.slice();
  EXPECT_TRUE(s.isArray());
  EXPECT_EQ(5UL, s.length());

  EXPECT_TRUE(s.at(0).isNumber());
  EXPECT_EQ(1, s.at(0).getInt());

  EXPECT_TRUE(s.at(1).isNumber());
  EXPECT_EQ(2, s.at(1).getInt());

  EXPECT_TRUE(s.at(2).isNumber());
  EXPECT_EQ(3, s.at(2).getInt());

  EXPECT_TRUE(s.at(3).isNumber());
  EXPECT_EQ(4, s.at(3).getInt());

  EXPECT_TRUE(s.at(4).isNumber());
  EXPECT_EQ(19, s.at(4).getInt());
}

TEST(CollectionTest, MapNonArray) {
  std::string const value("null");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
 
  EXPECT_VELOCYPACK_EXCEPTION(Collection::map(s, [] (Slice const&, ValueLength) -> Value { return Value(ValueType::None); }), Exception::InvalidValueType);
}

TEST(CollectionTest, MapEmptyArray) {
  std::string const value("[]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
 
  Builder b = Collection::map(s, [] (Slice const&, ValueLength) -> Value {
    EXPECT_TRUE(false);
    return Value(ValueType::None);
  });

  s = b.slice();
  EXPECT_TRUE(s.isArray());
  EXPECT_EQ(0UL, s.length());
}

TEST(CollectionTest, MapArray) {
  std::string const value("[1,2,3,4,-42,19]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
 
  std::vector<std::string> mapped = { "foo", "bar", "baz", "qux", "quetzalcoatl", "" };
  size_t seen = 0;
  Builder b = Collection::map(s, [&seen, &mapped] (Slice const& slice, ValueLength index) -> Value {
    EXPECT_EQ(seen, index);
    EXPECT_TRUE(slice.isNumber());

    ++seen;
    return Value(mapped[index]);
  });
  EXPECT_EQ(6UL, seen);

  s = b.slice();
  EXPECT_TRUE(s.isArray());
  EXPECT_EQ(6UL, s.length());

  EXPECT_TRUE(s.at(0).isString());
  EXPECT_EQ("foo", s.at(0).copyString());
  
  EXPECT_TRUE(s.at(1).isString());
  EXPECT_EQ("bar", s.at(1).copyString());
  
  EXPECT_TRUE(s.at(2).isString());
  EXPECT_EQ("baz", s.at(2).copyString());
  
  EXPECT_TRUE(s.at(3).isString());
  EXPECT_EQ("qux", s.at(3).copyString());
  
  EXPECT_TRUE(s.at(4).isString());
  EXPECT_EQ("quetzalcoatl", s.at(4).copyString());
  
  EXPECT_TRUE(s.at(5).isString());
  EXPECT_EQ("", s.at(5).copyString());
}

TEST(CollectionTest, FindNonArray) {
  std::string const value("null");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
  
  EXPECT_VELOCYPACK_EXCEPTION(Collection::find(s, DoNothingCallback), Exception::InvalidValueType);
}

TEST(CollectionTest, FindEmptyArray) {
  std::string const value("[]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
 
  Slice found = Collection::find(s, FailCallback);
  EXPECT_TRUE(found.isNone());
}

TEST(CollectionTest, FindArrayFalse) {
  std::string const value("[1,2,3]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
 
  Slice found = Collection::find(s, DoNothingCallback);
  EXPECT_TRUE(found.isNone());
}

TEST(CollectionTest, FindArrayFirst) {
  std::string const value("[1,2,3]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
 
  size_t seen = 0;
  Slice found = Collection::find(s, [&seen] (Slice const&, ValueLength) {
    ++seen;
    return true;
  });
  EXPECT_EQ(1UL, seen);
  EXPECT_TRUE(found.isNumber());
  EXPECT_EQ(1UL, found.getUInt());
}

TEST(CollectionTest, FindArrayLast) {
  std::string const value("[1,2,3]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
 
  size_t seen = 0;
  Slice found = Collection::find(s, [&seen] (Slice const&, ValueLength index) {
    ++seen;
    if (index == 2) {
      return true;
    }
    return false;
  });
  EXPECT_EQ(3UL, seen);
  EXPECT_TRUE(found.isNumber());
  EXPECT_EQ(3UL, found.getUInt());
}

TEST(CollectionTest, ContainsNonArray) {
  std::string const value("null");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
  
  EXPECT_VELOCYPACK_EXCEPTION(Collection::contains(s, DoNothingCallback), Exception::InvalidValueType);
}

TEST(CollectionTest, ContainsEmptyArray) {
  std::string const value("[]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
 
  EXPECT_FALSE(Collection::contains(s, FailCallback));
}

TEST(CollectionTest, ContainsArrayFalse) {
  std::string const value("[1,2,3]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
 
  EXPECT_FALSE(Collection::contains(s, DoNothingCallback));
}

TEST(CollectionTest, ContainsArrayFirst) {
  std::string const value("[1,2,3]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
 
  size_t seen = 0;
  EXPECT_TRUE(Collection::contains(s, [&seen] (Slice const&, ValueLength) {
    ++seen;
    return true;
  }));
  EXPECT_EQ(1UL, seen);
}

TEST(CollectionTest, ContainsArrayLast) {
  std::string const value("[1,2,3]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
 
  size_t seen = 0;
  EXPECT_TRUE(Collection::contains(s, [&seen] (Slice const&, ValueLength index) {
    ++seen;
    if (index == 2) {
      return true;
    }
    return false;
  }));
  EXPECT_EQ(3UL, seen);
}

TEST(CollectionTest, AllNonArray) {
  std::string const value("null");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
  
  EXPECT_VELOCYPACK_EXCEPTION(Collection::all(s, DoNothingCallback), Exception::InvalidValueType);
}

TEST(CollectionTest, AllEmptyArray) {
  std::string const value("[]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
 
  EXPECT_TRUE(Collection::all(s, FailCallback));
}

TEST(CollectionTest, AllArrayFalse) {
  std::string const value("[1,2,3]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
 
  EXPECT_FALSE(Collection::all(s, DoNothingCallback));
}

TEST(CollectionTest, AllArrayFirstFalse) {
  std::string const value("[1,2,3,4]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
 
  size_t seen = 0;
  EXPECT_FALSE(Collection::all(s, [&seen] (Slice const&, ValueLength index) -> bool {
    EXPECT_EQ(seen, index);

    ++seen;
    return false;
  }));

  EXPECT_EQ(1UL, seen);
}

TEST(CollectionTest, AllArrayLastFalse) {
  std::string const value("[1,2,3,4]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
 
  size_t seen = 0;
  EXPECT_FALSE(Collection::all(s, [&seen] (Slice const&, ValueLength index) -> bool {
    EXPECT_EQ(seen, index);

    ++seen;
    if (index == 2) {
      return false;
    }
    return true;
  }));

  EXPECT_EQ(3UL, seen);
}

TEST(CollectionTest, AllArrayTrue) {
  std::string const value("[1,2,3,4]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
 
  size_t seen = 0;
  EXPECT_TRUE(Collection::all(s, [&seen] (Slice const&, ValueLength index) -> bool {
    EXPECT_EQ(seen, index);

    ++seen;
    return true;
  }));

  EXPECT_EQ(4UL, seen);
}

TEST(CollectionTest, AnyNonArray) {
  std::string const value("null");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
  
  EXPECT_VELOCYPACK_EXCEPTION(Collection::any(s, DoNothingCallback), Exception::InvalidValueType);
}

TEST(CollectionTest, AnyEmptyArray) {
  std::string const value("[]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
 
  EXPECT_FALSE(Collection::any(s, FailCallback));
}

TEST(CollectionTest, AnyArrayFalse) {
  std::string const value("[1,2,3]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
 
  EXPECT_FALSE(Collection::all(s, DoNothingCallback));
}

TEST(CollectionTest, AnyArrayLastTrue) {
  std::string const value("[1,2,3,4]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
 
  size_t seen = 0;
  EXPECT_TRUE(Collection::any(s, [&seen] (Slice const&, ValueLength index) -> bool {
    EXPECT_EQ(seen, index);

    ++seen;
    if (index == 3) {
      return true;
    }
    return false;
  }));

  EXPECT_EQ(4UL, seen);
}

TEST(CollectionTest, AnyArrayFirstTrue) {
  std::string const value("[1,2,3,4]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());
 
  size_t seen = 0;
  EXPECT_TRUE(Collection::any(s, [&seen] (Slice const&, ValueLength index) -> bool {
    EXPECT_EQ(seen, index);

    ++seen;
    return true;
  }));

  EXPECT_EQ(1UL, seen);
}

int main (int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

