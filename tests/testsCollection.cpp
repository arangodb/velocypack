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

static auto DoNothingCallback =
    [](Slice const&, ValueLength) -> bool { return false; };
static auto FailCallback = [](Slice const&, ValueLength) -> bool {
  EXPECT_TRUE(false);
  return false;
};

TEST(CollectionTest, KeysNonObject1) {
  std::string const value("null");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ASSERT_VELOCYPACK_EXCEPTION(Collection::keys(s), Exception::InvalidValueType);
}

TEST(CollectionTest, KeysNonObject2) {
  std::string const value("null");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  std::vector<std::string> result;
  ASSERT_VELOCYPACK_EXCEPTION(Collection::keys(s, result),
                              Exception::InvalidValueType);
}

TEST(CollectionTest, KeysNonObject3) {
  std::string const value("null");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  std::unordered_set<std::string> result;
  ASSERT_VELOCYPACK_EXCEPTION(Collection::keys(s, result),
                              Exception::InvalidValueType);
}

TEST(CollectionTest, KeysNonObject4) {
  std::string const value("[]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ASSERT_VELOCYPACK_EXCEPTION(Collection::keys(s), Exception::InvalidValueType);
}

TEST(CollectionTest, KeysNonObject5) {
  std::string const value("[]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  std::vector<std::string> result;
  ASSERT_VELOCYPACK_EXCEPTION(Collection::keys(s, result),
                              Exception::InvalidValueType);
}

TEST(CollectionTest, KeysNonObject6) {
  std::string const value("[]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  std::unordered_set<std::string> result;
  ASSERT_VELOCYPACK_EXCEPTION(Collection::keys(s, result),
                              Exception::InvalidValueType);
}

TEST(CollectionTest, ObjectKeys1) {
  Options options;
  options.sortAttributeNames = false;

  std::string const value("{\"foo\":1,\"bar\":2,\"baz\":3}");
  Parser parser(&options);
  parser.parse(value);
  Slice s(parser.start());

  std::vector<std::string> keys = Collection::keys(s);
  ASSERT_EQ(3UL, keys.size());
  ASSERT_EQ("foo", keys[0]);
  ASSERT_EQ("bar", keys[1]);
  ASSERT_EQ("baz", keys[2]);
}

TEST(CollectionTest, ObjectKeys2) {
  Options options;
  options.sortAttributeNames = false;

  std::string const value("{\"foo\":1,\"bar\":2,\"baz\":3}");
  Parser parser(&options);
  parser.parse(value);
  Slice s(parser.start());

  std::vector<std::string> keys;
  Collection::keys(s, keys);
  ASSERT_EQ(3UL, keys.size());
  ASSERT_EQ("foo", keys[0]);
  ASSERT_EQ("bar", keys[1]);
  ASSERT_EQ("baz", keys[2]);
}

TEST(CollectionTest, ObjectKeys3) {
  Options options;
  options.sortAttributeNames = false;

  std::string const value("{\"foo\":1,\"bar\":2,\"baz\":3}");
  Parser parser(&options);
  parser.parse(value);
  Slice s(parser.start());

  std::unordered_set<std::string> keys;
  Collection::keys(s, keys);
  ASSERT_EQ(3UL, keys.size());
  ASSERT_TRUE(keys.find("foo") != keys.end());
  ASSERT_TRUE(keys.find("bar") != keys.end());
  ASSERT_TRUE(keys.find("baz") != keys.end());
}

TEST(CollectionTest, ObjectKeys) {
  std::string const value(
      "{\"1foo\":\"bar\",\"2baz\":\"quux\",\"3number\":1,\"4boolean\":true,"
      "\"5empty\":null}");

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

TEST(CollectionsTest, ObjectKeysRef) {
  std::string const value(
      "{\"1foo\":\"bar\",\"2baz\":\"quux\",\"3number\":1,\"4boolean\":true,"
      "\"5empty\":null}");

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

TEST(CollectionTest, ObjectKeysCompact) {
  std::string const value(
      "{\"1foo\":\"bar\",\"2baz\":\"quux\",\"3number\":1,\"4boolean\":true,"
      "\"5empty\":null}");

  Options options;
  options.buildUnindexedArrays = true;

  Parser parser(&options);
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

TEST(CollectionTest, ValuesNonObject1) {
  std::string const value("null");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ASSERT_VELOCYPACK_EXCEPTION(Collection::values(s),
                              Exception::InvalidValueType);
}

TEST(CollectionTest, ValuesNonObject2) {
  std::string const value("\"foobar\"");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ASSERT_VELOCYPACK_EXCEPTION(Collection::values(s),
                              Exception::InvalidValueType);
}

TEST(CollectionTest, ValuesNonObject3) {
  std::string const value("[]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ASSERT_VELOCYPACK_EXCEPTION(Collection::values(s),
                              Exception::InvalidValueType);
}

TEST(CollectionTest, ObjectValues) {
  std::string const value(
      "{\"1foo\":\"bar\",\"2baz\":\"quux\",\"3number\":1,\"4boolean\":true,"
      "\"5empty\":null}");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  Builder b = Collection::values(s);
  s = b.slice();
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(5U, s.length());

  ASSERT_TRUE(s.at(0).isString());
  ASSERT_EQ("bar", s.at(0).copyString());
  ASSERT_TRUE(s.at(1).isString());
  ASSERT_EQ("quux", s.at(1).copyString());
  ASSERT_TRUE(s.at(2).isNumber());
  ASSERT_EQ(1UL, s.at(2).getUInt());
  ASSERT_TRUE(s.at(3).isBoolean());
  ASSERT_TRUE(s.at(3).getBoolean());
  ASSERT_TRUE(s.at(4).isNull());
}

TEST(CollectionTest, ObjectValuesCompact) {
  std::string const value(
      "{\"1foo\":\"bar\",\"2baz\":\"quux\",\"3number\":1,\"4boolean\":true,"
      "\"5empty\":null}");

  Options options;
  options.buildUnindexedObjects = true;

  Parser parser(&options);
  parser.parse(value);
  Slice s(parser.start());

  ASSERT_EQ(0x14, s.head());

  Builder b = Collection::values(s);
  s = b.slice();
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(5U, s.length());

  ASSERT_TRUE(s.at(0).isString());
  ASSERT_EQ("bar", s.at(0).copyString());
  ASSERT_TRUE(s.at(1).isString());
  ASSERT_EQ("quux", s.at(1).copyString());
  ASSERT_TRUE(s.at(2).isNumber());
  ASSERT_EQ(1UL, s.at(2).getUInt());
  ASSERT_TRUE(s.at(3).isBoolean());
  ASSERT_TRUE(s.at(3).getBoolean());
  ASSERT_TRUE(s.at(4).isNull());
}

TEST(CollectionTest, ForEachNonArray) {
  std::string const value("null");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ASSERT_VELOCYPACK_EXCEPTION(Collection::forEach(s, DoNothingCallback),
                              Exception::InvalidValueType);
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
  Collection::forEach(s,
                      [&seen](Slice const& slice, ValueLength index) -> bool {
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

  ASSERT_EQ(5UL, seen);
}

TEST(CollectionTest, ForEachArrayAbort) {
  std::string const value("[1,2,3,\"foo\",\"bar\"]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  size_t seen = 0;
  Collection::forEach(s, [&seen](Slice const&, ValueLength index) -> bool {
    EXPECT_EQ(seen, index);

    if (seen == 3) {
      return false;
    }
    ++seen;
    return true;
  });

  ASSERT_EQ(3UL, seen);
}

TEST(CollectionTest, IterateArrayValues) {
  std::string const value("[1,2,3,4,null,true,\"foo\",\"bar\"]");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  size_t state = 0;
  Collection::forEach(s, [&state](Slice const& value, ValueLength) -> bool {
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

TEST(CollectionTest, FilterNonArray) {
  std::string const value("null");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ASSERT_VELOCYPACK_EXCEPTION(Collection::filter(s, DoNothingCallback),
                              Exception::InvalidValueType);
}

TEST(CollectionTest, FilterEmptyArray) {
  std::string const value("[]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  Builder b = Collection::filter(s, FailCallback);

  s = b.slice();
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(0UL, s.length());
}

TEST(CollectionTest, FilterAll) {
  std::string const value("[1,2,3,4,-42,19]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  Builder b = Collection::filter(s, DoNothingCallback);

  s = b.slice();
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(0UL, s.length());
}

TEST(CollectionTest, FilterArray) {
  std::string const value("[1,2,3,4,-42,19]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  size_t seen = 0;
  Builder b = Collection::filter(
      s, [&seen](Slice const& slice, ValueLength index) -> bool {
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
  ASSERT_EQ(6UL, seen);

  s = b.slice();
  ASSERT_TRUE(s.isArray());
  ASSERT_EQ(5UL, s.length());

  ASSERT_TRUE(s.at(0).isNumber());
  ASSERT_EQ(1, s.at(0).getInt());

  ASSERT_TRUE(s.at(1).isNumber());
  ASSERT_EQ(2, s.at(1).getInt());

  ASSERT_TRUE(s.at(2).isNumber());
  ASSERT_EQ(3, s.at(2).getInt());

  ASSERT_TRUE(s.at(3).isNumber());
  ASSERT_EQ(4, s.at(3).getInt());

  ASSERT_TRUE(s.at(4).isNumber());
  ASSERT_EQ(19, s.at(4).getInt());
}

TEST(CollectionTest, FindNonArray) {
  std::string const value("null");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ASSERT_VELOCYPACK_EXCEPTION(Collection::find(s, DoNothingCallback),
                              Exception::InvalidValueType);
}

TEST(CollectionTest, FindEmptyArray) {
  std::string const value("[]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  Slice found = Collection::find(s, FailCallback);
  ASSERT_TRUE(found.isNone());
}

TEST(CollectionTest, FindArrayFalse) {
  std::string const value("[1,2,3]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  Slice found = Collection::find(s, DoNothingCallback);
  ASSERT_TRUE(found.isNone());
}

TEST(CollectionTest, FindArrayFirst) {
  std::string const value("[1,2,3]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  size_t seen = 0;
  Slice found = Collection::find(s, [&seen](Slice const&, ValueLength) {
    ++seen;
    return true;
  });
  ASSERT_EQ(1UL, seen);
  ASSERT_TRUE(found.isNumber());
  ASSERT_EQ(1UL, found.getUInt());
}

TEST(CollectionTest, FindArrayLast) {
  std::string const value("[1,2,3]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  size_t seen = 0;
  Slice found = Collection::find(s, [&seen](Slice const&, ValueLength index) {
    ++seen;
    if (index == 2) {
      return true;
    }
    return false;
  });
  ASSERT_EQ(3UL, seen);
  ASSERT_TRUE(found.isNumber());
  ASSERT_EQ(3UL, found.getUInt());
}

TEST(CollectionTest, ContainsNonArray) {
  std::string const value("null");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ASSERT_VELOCYPACK_EXCEPTION(Collection::contains(s, DoNothingCallback),
                              Exception::InvalidValueType);
}

TEST(CollectionTest, ContainsEmptyArray) {
  std::string const value("[]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ASSERT_FALSE(Collection::contains(s, FailCallback));
}

TEST(CollectionTest, ContainsArrayFalse) {
  std::string const value("[1,2,3]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ASSERT_FALSE(Collection::contains(s, DoNothingCallback));
}

TEST(CollectionTest, ContainsArrayFirst) {
  std::string const value("[1,2,3]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  size_t seen = 0;
  ASSERT_TRUE(Collection::contains(s, [&seen](Slice const&, ValueLength) {
    ++seen;
    return true;
  }));
  ASSERT_EQ(1UL, seen);
}

TEST(CollectionTest, ContainsArrayLast) {
  std::string const value("[1,2,3]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  size_t seen = 0;
  ASSERT_TRUE(Collection::contains(s, [&seen](Slice const&, ValueLength index) {
    ++seen;
    if (index == 2) {
      return true;
    }
    return false;
  }));
  ASSERT_EQ(3UL, seen);
}

TEST(CollectionTest, AllNonArray) {
  std::string const value("null");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ASSERT_VELOCYPACK_EXCEPTION(Collection::all(s, DoNothingCallback),
                              Exception::InvalidValueType);
}

TEST(CollectionTest, AllEmptyArray) {
  std::string const value("[]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ASSERT_TRUE(Collection::all(s, FailCallback));
}

TEST(CollectionTest, AllArrayFalse) {
  std::string const value("[1,2,3]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ASSERT_FALSE(Collection::all(s, DoNothingCallback));
}

TEST(CollectionTest, AllArrayFirstFalse) {
  std::string const value("[1,2,3,4]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  size_t seen = 0;
  ASSERT_FALSE(
      Collection::all(s, [&seen](Slice const&, ValueLength index) -> bool {
        EXPECT_EQ(seen, index);

        ++seen;
        return false;
      }));

  ASSERT_EQ(1UL, seen);
}

TEST(CollectionTest, AllArrayLastFalse) {
  std::string const value("[1,2,3,4]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  size_t seen = 0;
  ASSERT_FALSE(
      Collection::all(s, [&seen](Slice const&, ValueLength index) -> bool {
        EXPECT_EQ(seen, index);

        ++seen;
        if (index == 2) {
          return false;
        }
        return true;
      }));

  ASSERT_EQ(3UL, seen);
}

TEST(CollectionTest, AllArrayTrue) {
  std::string const value("[1,2,3,4]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  size_t seen = 0;
  ASSERT_TRUE(
      Collection::all(s, [&seen](Slice const&, ValueLength index) -> bool {
        EXPECT_EQ(seen, index);

        ++seen;
        return true;
      }));

  ASSERT_EQ(4UL, seen);
}

TEST(CollectionTest, AnyNonArray) {
  std::string const value("null");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ASSERT_VELOCYPACK_EXCEPTION(Collection::any(s, DoNothingCallback),
                              Exception::InvalidValueType);
}

TEST(CollectionTest, AnyEmptyArray) {
  std::string const value("[]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ASSERT_FALSE(Collection::any(s, FailCallback));
}

TEST(CollectionTest, AnyArrayFalse) {
  std::string const value("[1,2,3]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ASSERT_FALSE(Collection::all(s, DoNothingCallback));
}

TEST(CollectionTest, AnyArrayLastTrue) {
  std::string const value("[1,2,3,4]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  size_t seen = 0;
  ASSERT_TRUE(
      Collection::any(s, [&seen](Slice const&, ValueLength index) -> bool {
        EXPECT_EQ(seen, index);

        ++seen;
        if (index == 3) {
          return true;
        }
        return false;
      }));

  ASSERT_EQ(4UL, seen);
}

TEST(CollectionTest, AnyArrayFirstTrue) {
  std::string const value("[1,2,3,4]");
  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  size_t seen = 0;
  ASSERT_TRUE(
      Collection::any(s, [&seen](Slice const&, ValueLength index) -> bool {
        EXPECT_EQ(seen, index);

        ++seen;
        return true;
      }));

  ASSERT_EQ(1UL, seen);
}

TEST(CollectionTest, KeepNonObject) {
  std::string const value("[]");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  std::vector<std::string> const toKeep = {"foo", "bar"};
  ASSERT_VELOCYPACK_EXCEPTION(Collection::keep(s, toKeep),
                              Exception::InvalidValueType);
}

TEST(CollectionTest, KeepEmptyObject) {
  std::string const value("{}");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  std::vector<std::string> const toKeep = {"foo", "bar"};
  Builder b = Collection::keep(s, toKeep);
  s = b.slice();
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(0U, s.length());
}

TEST(CollectionTest, KeepNoAttributes) {
  std::string const value(
      "{\"foo\":\"bar\",\"baz\":\"quux\",\"number\":1,\"boolean\":true,"
      "\"empty\":null}");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  std::vector<std::string> const toKeep = {};
  Builder b = Collection::keep(s, toKeep);
  s = b.slice();
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(0U, s.length());
}

TEST(CollectionTest, KeepSomeAttributes) {
  std::string const value(
      "{\"foo\":\"bar\",\"baz\":\"quux\",\"number\":1,\"boolean\":true,"
      "\"empty\":null}");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  std::vector<std::string> const toKeep = {"foo", "baz", "empty"};
  Builder b = Collection::keep(s, toKeep);
  s = b.slice();
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(3U, s.length());

  ASSERT_TRUE(s.hasKey("foo"));
  ASSERT_EQ("bar", s.get("foo").copyString());

  ASSERT_TRUE(s.hasKey("baz"));
  ASSERT_EQ("quux", s.get("baz").copyString());

  ASSERT_TRUE(s.hasKey("empty"));
  ASSERT_TRUE(s.get("empty").isNull());

  ASSERT_FALSE(s.hasKey("number"));
  ASSERT_FALSE(s.hasKey("boolean"));
  ASSERT_FALSE(s.hasKey("quetzalcoatl"));
}

TEST(CollectionTest, KeepSomeAttributesUsingSet) {
  std::string const value(
      "{\"foo\":\"bar\",\"baz\":\"quux\",\"number\":1,\"boolean\":true,"
      "\"empty\":null}");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  std::unordered_set<std::string> const toKeep = {"foo", "baz", "empty"};
  Builder b = Collection::keep(s, toKeep);
  s = b.slice();
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(3U, s.length());

  ASSERT_TRUE(s.hasKey("foo"));
  ASSERT_EQ("bar", s.get("foo").copyString());

  ASSERT_TRUE(s.hasKey("baz"));
  ASSERT_EQ("quux", s.get("baz").copyString());

  ASSERT_TRUE(s.hasKey("empty"));
  ASSERT_TRUE(s.get("empty").isNull());

  ASSERT_FALSE(s.hasKey("number"));
  ASSERT_FALSE(s.hasKey("boolean"));
  ASSERT_FALSE(s.hasKey("quetzalcoatl"));
}

TEST(CollectionTest, KeepNonExistingAttributes) {
  std::string const value(
      "{\"foo\":\"bar\",\"baz\":\"quux\",\"number\":1,\"boolean\":true,"
      "\"empty\":null}");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  std::vector<std::string> const toKeep = {"boo", "far", "quetzalcoatl",
                                           "empty"};
  Builder b = Collection::keep(s, toKeep);
  s = b.slice();
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(1U, s.length());

  ASSERT_TRUE(s.hasKey("empty"));
  ASSERT_TRUE(s.get("empty").isNull());

  ASSERT_FALSE(s.hasKey("foo"));
  ASSERT_FALSE(s.hasKey("baz"));
  ASSERT_FALSE(s.hasKey("number"));
  ASSERT_FALSE(s.hasKey("boolean"));
  ASSERT_FALSE(s.hasKey("quetzalcoatl"));
}

TEST(CollectionTest, KeepNonExistingAttributesUsingSet) {
  std::string const value(
      "{\"foo\":\"bar\",\"baz\":\"quux\",\"number\":1,\"boolean\":true,"
      "\"empty\":null}");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  std::unordered_set<std::string> const toKeep = {"boo", "far", "quetzalcoatl",
                                                  "empty"};
  Builder b = Collection::keep(s, toKeep);
  s = b.slice();
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(1U, s.length());

  ASSERT_TRUE(s.hasKey("empty"));
  ASSERT_TRUE(s.get("empty").isNull());

  ASSERT_FALSE(s.hasKey("foo"));
  ASSERT_FALSE(s.hasKey("baz"));
  ASSERT_FALSE(s.hasKey("number"));
  ASSERT_FALSE(s.hasKey("boolean"));
  ASSERT_FALSE(s.hasKey("quetzalcoatl"));
}

TEST(CollectionTest, KeepManyAttributes) {
  std::string value("{");
  for (size_t i = 0; i < 100; ++i) {
    if (i > 0) {
      value.push_back(',');
    }
    value.append("\"test");
    value.append(std::to_string(i));
    value.append("\":");
    value.append(std::to_string(i));
  }
  value.push_back('}');

  Builder b = Parser::fromJson(value);
  Slice s(b.start());

  std::vector<std::string> toKeep;
  for (size_t i = 0; i < 30; ++i) {
    std::string key = "test" + std::to_string(i);
    toKeep.push_back(key);
  }

  b = Collection::keep(s, toKeep);
  s = b.slice();
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(30U, s.length());

  for (size_t i = 0; i < 100; ++i) {
    std::string key = "test" + std::to_string(i);
    if (i < 30) {
      ASSERT_TRUE(s.hasKey(key));
      ASSERT_TRUE(s.get(key).isNumber());
      ASSERT_EQ(i, s.get(key).getUInt());
    } else {
      ASSERT_FALSE(s.hasKey(key));
    }
  }
}

TEST(CollectionTest, RemoveNonObject) {
  std::string const value("[]");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  std::vector<std::string> const toRemove = {"foo", "bar"};
  ASSERT_VELOCYPACK_EXCEPTION(Collection::remove(s, toRemove),
                              Exception::InvalidValueType);
}

TEST(CollectionTest, RemoveEmptyObject) {
  std::string const value("{}");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  std::vector<std::string> const toRemove = {"foo", "bar"};
  Builder b = Collection::remove(s, toRemove);
  s = b.slice();
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(0U, s.length());
}

TEST(CollectionTest, RemoveNoAttributes) {
  std::string const value(
      "{\"foo\":\"bar\",\"baz\":\"quux\",\"number\":1,\"boolean\":true,"
      "\"empty\":null}");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  std::vector<std::string> const toRemove = {};
  Builder b = Collection::remove(s, toRemove);
  s = b.slice();
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(5U, s.length());

  ASSERT_TRUE(s.hasKey("foo"));
  ASSERT_EQ("bar", s.get("foo").copyString());
  ASSERT_TRUE(s.hasKey("baz"));
  ASSERT_EQ("quux", s.get("baz").copyString());
  ASSERT_TRUE(s.hasKey("number"));
  ASSERT_EQ(1U, s.get("number").getUInt());
  ASSERT_TRUE(s.hasKey("boolean"));
  ASSERT_TRUE(s.get("boolean").getBoolean());
  ASSERT_TRUE(s.hasKey("empty"));
  ASSERT_TRUE(s.get("empty").isNull());
}

TEST(CollectionTest, RemoveSomeAttributes) {
  std::string const value(
      "{\"foo\":\"bar\",\"baz\":\"quux\",\"number\":1,\"boolean\":true,"
      "\"empty\":null}");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  std::vector<std::string> const toRemove = {"foo", "baz", "empty"};
  Builder b = Collection::remove(s, toRemove);
  s = b.slice();
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(2U, s.length());

  ASSERT_FALSE(s.hasKey("foo"));
  ASSERT_FALSE(s.hasKey("baz"));
  ASSERT_FALSE(s.hasKey("empty"));

  ASSERT_TRUE(s.hasKey("number"));
  ASSERT_EQ(1U, s.get("number").getUInt());
  ASSERT_TRUE(s.hasKey("boolean"));
  ASSERT_TRUE(s.get("boolean").getBoolean());
}

TEST(CollectionTest, RemoveSomeAttributesUsingSet) {
  std::string const value(
      "{\"foo\":\"bar\",\"baz\":\"quux\",\"number\":1,\"boolean\":true,"
      "\"empty\":null}");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  std::unordered_set<std::string> const toRemove = {"foo", "baz", "empty"};
  Builder b = Collection::remove(s, toRemove);
  s = b.slice();
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(2U, s.length());

  ASSERT_FALSE(s.hasKey("foo"));
  ASSERT_FALSE(s.hasKey("baz"));
  ASSERT_FALSE(s.hasKey("empty"));

  ASSERT_TRUE(s.hasKey("number"));
  ASSERT_EQ(1U, s.get("number").getUInt());
  ASSERT_TRUE(s.hasKey("boolean"));
  ASSERT_TRUE(s.get("boolean").getBoolean());
}

TEST(CollectionTest, RemoveManyAttributes) {
  std::string value("{");
  for (size_t i = 0; i < 100; ++i) {
    if (i > 0) {
      value.push_back(',');
    }
    value.append("\"test");
    value.append(std::to_string(i));
    value.append("\":");
    value.append(std::to_string(i));
  }
  value.push_back('}');

  Builder b = Parser::fromJson(value);
  Slice s(b.start());

  std::vector<std::string> toRemove;
  for (size_t i = 0; i < 30; ++i) {
    std::string key = "test" + std::to_string(i);
    toRemove.push_back(key);
  }

  b = Collection::remove(s, toRemove);
  s = b.slice();
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(70U, s.length());

  for (size_t i = 0; i < 100; ++i) {
    std::string key = "test" + std::to_string(i);
    if (i >= 30) {
      ASSERT_TRUE(s.hasKey(key));
      ASSERT_TRUE(s.get(key).isNumber());
      ASSERT_EQ(i, s.get(key).getUInt());
    } else {
      ASSERT_FALSE(s.hasKey(key));
    }
  }
}

TEST(CollectionTest, RemoveNonExistingAttributes) {
  std::string const value(
      "{\"foo\":\"bar\",\"baz\":\"quux\",\"number\":1,\"boolean\":true,"
      "\"empty\":null}");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  std::vector<std::string> const toRemove = {"boo", "far", "quetzalcoatl",
                                             "empty"};
  Builder b = Collection::remove(s, toRemove);
  s = b.slice();
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(4U, s.length());

  ASSERT_TRUE(s.hasKey("foo"));
  ASSERT_EQ("bar", s.get("foo").copyString());
  ASSERT_TRUE(s.hasKey("baz"));
  ASSERT_EQ("quux", s.get("baz").copyString());
  ASSERT_TRUE(s.hasKey("number"));
  ASSERT_EQ(1UL, s.get("number").getUInt());
  ASSERT_TRUE(s.hasKey("boolean"));
  ASSERT_TRUE(s.get("boolean").getBoolean());
  ASSERT_FALSE(s.hasKey("empty"));
}

TEST(CollectionTest, RemoveNonExistingAttributesUsingSet) {
  std::string const value(
      "{\"foo\":\"bar\",\"baz\":\"quux\",\"number\":1,\"boolean\":true,"
      "\"empty\":null}");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  std::unordered_set<std::string> const toRemove = {"boo", "far",
                                                    "quetzalcoatl", "empty"};
  Builder b = Collection::remove(s, toRemove);
  s = b.slice();
  ASSERT_TRUE(s.isObject());
  ASSERT_EQ(4U, s.length());

  ASSERT_TRUE(s.hasKey("foo"));
  ASSERT_EQ("bar", s.get("foo").copyString());
  ASSERT_TRUE(s.hasKey("baz"));
  ASSERT_EQ("quux", s.get("baz").copyString());
  ASSERT_TRUE(s.hasKey("number"));
  ASSERT_EQ(1UL, s.get("number").getUInt());
  ASSERT_TRUE(s.hasKey("boolean"));
  ASSERT_TRUE(s.get("boolean").getBoolean());
  ASSERT_FALSE(s.hasKey("empty"));
}

TEST(CollectionTest, MergeNonObject) {
  Builder b1;
  b1.add(Value(ValueType::Array));
  b1.close();

  Builder b2;
  b2.add(Value(ValueType::Object));
  b2.close();

  ASSERT_VELOCYPACK_EXCEPTION(Collection::merge(b1.slice(), b1.slice(), false),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(Collection::merge(b1.slice(), b2.slice(), false),
                              Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(Collection::merge(b2.slice(), b1.slice(), false),
                              Exception::InvalidValueType);
}

TEST(CollectionTest, MergeEmptyLeft) {
  std::string const l("{}");
  std::string const r("{\"bark\":1,\"qux\":2,\"bart\":3}");

  Builder p1 = Parser::fromJson(l);
  Slice s1(p1.start());

  Builder p2 = Parser::fromJson(r);
  Slice s2(p2.start());

  Builder b = Collection::merge(s1, s2, true);
  Slice s(b.start());
  ASSERT_TRUE(s.hasKey("bark"));
  ASSERT_EQ(1UL, s.get("bark").getUInt());
  ASSERT_TRUE(s.hasKey("qux"));
  ASSERT_EQ(2UL, s.get("qux").getUInt());
  ASSERT_TRUE(s.hasKey("bart"));
  ASSERT_EQ(3UL, s.get("bart").getUInt());
}

TEST(CollectionTest, MergeEmptyRight) {
  std::string const l("{\"bark\":1,\"qux\":2,\"bart\":3}");
  std::string const r("{}");

  Builder p1 = Parser::fromJson(l);
  Slice s1(p1.start());

  Builder p2 = Parser::fromJson(r);
  Slice s2(p2.start());

  Builder b = Collection::merge(s1, s2, true);
  Slice s(b.start());
  ASSERT_TRUE(s.hasKey("bark"));
  ASSERT_EQ(1UL, s.get("bark").getUInt());
  ASSERT_TRUE(s.hasKey("qux"));
  ASSERT_EQ(2UL, s.get("qux").getUInt());
  ASSERT_TRUE(s.hasKey("bart"));
  ASSERT_EQ(3UL, s.get("bart").getUInt());
}

TEST(CollectionTest, MergeDistinct) {
  std::string const l("{\"foo\":1,\"bar\":2,\"baz\":3}");
  std::string const r("{\"bark\":1,\"qux\":2,\"bart\":3}");

  Builder p1 = Parser::fromJson(l);
  Slice s1(p1.start());

  Builder p2 = Parser::fromJson(r);
  Slice s2(p2.start());

  Builder b = Collection::merge(s1, s2, true);
  Slice s(b.start());
  ASSERT_TRUE(s.hasKey("foo"));
  ASSERT_EQ(1UL, s.get("foo").getUInt());
  ASSERT_TRUE(s.hasKey("bar"));
  ASSERT_EQ(2UL, s.get("bar").getUInt());
  ASSERT_TRUE(s.hasKey("baz"));
  ASSERT_EQ(3UL, s.get("baz").getUInt());
  ASSERT_TRUE(s.hasKey("bark"));
  ASSERT_EQ(1UL, s.get("bark").getUInt());
  ASSERT_TRUE(s.hasKey("qux"));
  ASSERT_EQ(2UL, s.get("qux").getUInt());
  ASSERT_TRUE(s.hasKey("bart"));
  ASSERT_EQ(3UL, s.get("bart").getUInt());
}

TEST(CollectionTest, MergeOverlap) {
  std::string const l("{\"foo\":1,\"bar\":2,\"baz\":3}");
  std::string const r(
      "{\"baz\":19,\"bark\":1,\"qux\":2,\"bar\":42,\"test\":9,\"foo\":12}");

  Builder p1 = Parser::fromJson(l);
  Slice s1(p1.start());

  Builder p2 = Parser::fromJson(r);
  Slice s2(p2.start());

  Builder b = Collection::merge(s1, s2, true);
  Slice s(b.start());
  ASSERT_TRUE(s.hasKey("foo"));
  ASSERT_EQ(12UL, s.get("foo").getUInt());
  ASSERT_TRUE(s.hasKey("bar"));
  ASSERT_EQ(42UL, s.get("bar").getUInt());
  ASSERT_TRUE(s.hasKey("baz"));
  ASSERT_EQ(19UL, s.get("baz").getUInt());
  ASSERT_TRUE(s.hasKey("bark"));
  ASSERT_EQ(1UL, s.get("bark").getUInt());
  ASSERT_TRUE(s.hasKey("qux"));
  ASSERT_EQ(2UL, s.get("qux").getUInt());
  ASSERT_TRUE(s.hasKey("test"));
  ASSERT_EQ(9UL, s.get("test").getUInt());
}

TEST(CollectionTest, MergeSubAttributes) {
  std::string const l(
      "{\"foo\":1,\"bar\":{\"one\":1,\"two\":2,\"three\":3},\"baz\":{},"
      "\"test\":1}");
  std::string const r(
      "{\"foo\":2,\"bar\":{\"one\":23,\"two\":42,\"four\":99},\"baz\":{"
      "\"test\":1,\"bart\":2}}");

  Builder p1 = Parser::fromJson(l);
  Slice s1(p1.start());

  Builder p2 = Parser::fromJson(r);
  Slice s2(p2.start());

  Builder b = Collection::merge(s1, s2, true);
  Slice s(b.start());
  ASSERT_TRUE(s.hasKey("foo"));
  ASSERT_EQ(2UL, s.get("foo").getUInt());
  ASSERT_TRUE(s.hasKey("bar"));
  Slice sub = s.get("bar");
  ASSERT_TRUE(sub.hasKey("one"));
  ASSERT_EQ(23UL, sub.get("one").getUInt());
  ASSERT_TRUE(sub.hasKey("two"));
  ASSERT_EQ(42UL, sub.get("two").getUInt());
  ASSERT_TRUE(sub.hasKey("three"));
  ASSERT_EQ(3UL, sub.get("three").getUInt());
  ASSERT_TRUE(sub.hasKey("four"));
  ASSERT_EQ(99UL, sub.get("four").getUInt());
  ASSERT_TRUE(s.hasKey("test"));
  ASSERT_EQ(1UL, s.get("test").getUInt());
  ASSERT_TRUE(s.hasKey("baz"));
  sub = s.get("baz");
  ASSERT_EQ(2UL, sub.length());
  ASSERT_TRUE(sub.hasKey("test"));
  ASSERT_EQ(1UL, sub.get("test").getUInt());
  ASSERT_TRUE(sub.hasKey("bart"));
  ASSERT_EQ(2UL, sub.get("bart").getUInt());
}

TEST(CollectionTest, MergeOverwriteSubAttributes) {
  std::string const l(
      "{\"foo\":1,\"bar\":{\"one\":1,\"two\":2,\"three\":3},\"baz\":{\"bird\":"
      "9},\"test\":1}");
  std::string const r(
      "{\"foo\":2,\"bar\":{\"one\":23,\"two\":42,\"four\":99},\"baz\":{"
      "\"test\":1,\"bart\":2}}");

  Builder p1 = Parser::fromJson(l);
  Slice s1(p1.start());

  Builder p2 = Parser::fromJson(r);
  Slice s2(p2.start());

  Builder b = Collection::merge(s1, s2, false);
  Slice s(b.start());
  ASSERT_TRUE(s.hasKey("foo"));
  ASSERT_EQ(2UL, s.get("foo").getUInt());
  ASSERT_TRUE(s.hasKey("bar"));
  Slice sub = s.get("bar");
  ASSERT_TRUE(sub.hasKey("one"));
  ASSERT_EQ(23UL, sub.get("one").getUInt());
  ASSERT_TRUE(sub.hasKey("two"));
  ASSERT_EQ(42UL, sub.get("two").getUInt());
  ASSERT_FALSE(sub.hasKey("three"));
  ASSERT_TRUE(sub.hasKey("four"));
  ASSERT_EQ(99UL, sub.get("four").getUInt());
  ASSERT_TRUE(s.hasKey("test"));
  ASSERT_EQ(1UL, s.get("test").getUInt());
  ASSERT_TRUE(s.hasKey("baz"));
  sub = s.get("baz");
  ASSERT_EQ(2UL, sub.length());
  ASSERT_FALSE(sub.hasKey("bird"));
  ASSERT_TRUE(sub.hasKey("test"));
  ASSERT_EQ(1UL, sub.get("test").getUInt());
  ASSERT_TRUE(sub.hasKey("bart"));
  ASSERT_EQ(2UL, sub.get("bart").getUInt());
}

TEST(CollectionTest, VisitRecursiveNonCompound) {
  std::string const value("[1,null,true,\"foo\"]");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  ASSERT_VELOCYPACK_EXCEPTION(
      Collection::visitRecursive(
          s.at(0), Collection::PreOrder,
          [](Slice const&, Slice const&) -> bool { return true; }),
      Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(
      Collection::visitRecursive(
          s.at(1), Collection::PreOrder,
          [](Slice const&, Slice const&) -> bool { return true; }),
      Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(
      Collection::visitRecursive(
          s.at(2), Collection::PreOrder,
          [](Slice const&, Slice const&) -> bool { return true; }),
      Exception::InvalidValueType);
  ASSERT_VELOCYPACK_EXCEPTION(
      Collection::visitRecursive(
          s.at(3), Collection::PreOrder,
          [](Slice const&, Slice const&) -> bool { return true; }),
      Exception::InvalidValueType);
}

TEST(CollectionTest, VisitRecursiveArrayPreOrderAbort) {
  std::string const value("[true, false, 1]");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  int seen = 0;
  Collection::visitRecursive(
      s, Collection::PreOrder,
      [&seen](Slice const& key, Slice const& value) -> bool {
        EXPECT_TRUE(key.isNone());
        switch (seen) {
          case 0:
            EXPECT_TRUE(value.isTrue());
            break;
          case 1:
            EXPECT_TRUE(value.isFalse());
            return false;
          default:
            throw "invalid state";
        }
        ++seen;
        return true;
      });

  ASSERT_EQ(1, seen);
}

TEST(CollectionTest, VisitRecursiveArrayPostOrderAbort) {
  std::string const value("[true, [null], false, 1]");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  int seen = 0;
  Collection::visitRecursive(
      s, Collection::PostOrder,
      [&seen](Slice const& key, Slice const& value) -> bool {
        EXPECT_TRUE(key.isNone());
        switch (seen) {
          case 0:
            EXPECT_TRUE(value.isTrue());
            break;
          case 1:
            EXPECT_TRUE(value.isArray());
            EXPECT_EQ(1UL, value.length());
            break;
          case 2:
            EXPECT_TRUE(value.isNull());
            return false;
          default:
            throw "invalid state";
        }
        ++seen;
        return true;
      });

  ASSERT_EQ(2, seen);
}

TEST(CollectionTest, VisitRecursiveObjectPreOrderAbort) {
  std::string const value("{\"foo\":true,\"bar\":false,\"baz\":1}");

  Options options;
  options.sortAttributeNames = false;
  Parser parser(&options);
  parser.parse(value);
  Slice s(parser.start());

  int seen = 0;
  Collection::visitRecursive(
      s, Collection::PreOrder,
      [&seen](Slice const& key, Slice const& value) -> bool {
        EXPECT_FALSE(key.isNone());
        switch (seen) {
          case 0:
            EXPECT_EQ("foo", key.copyString());
            EXPECT_TRUE(value.isTrue());
            break;
          case 1:
            EXPECT_EQ("bar", key.copyString());
            EXPECT_TRUE(value.isFalse());
            return false;
          default:
            throw "invalid state";
        }
        ++seen;
        return true;
      });

  ASSERT_EQ(1, seen);
}

TEST(CollectionTest, VisitRecursiveObjectPostOrderAbort) {
  std::string const value("{\"foo\":{\"baz\":1,\"bar\":2},\"bark\":3}");

  Options options;
  options.sortAttributeNames = false;
  Parser parser(&options);
  parser.parse(value);
  Slice s(parser.start());

  int seen = 0;
  Collection::visitRecursive(
      s, Collection::PostOrder,
      [&seen](Slice const& key, Slice const& value) -> bool {
        EXPECT_FALSE(key.isNone());
        switch (seen) {
          case 0:
            EXPECT_EQ("foo", key.copyString());
            EXPECT_TRUE(value.isObject());
            break;
          case 1:
            EXPECT_EQ("baz", key.copyString());
            EXPECT_TRUE(value.isNumber());
            EXPECT_EQ(1UL, value.getUInt());
            return false;
          default:
            throw "invalid state";
        }
        ++seen;
        return true;
      });

  ASSERT_EQ(1, seen);
}

TEST(CollectionTest, VisitRecursiveArrayPreOrder) {
  std::string const value("[1,null,true,\"foo\",[23,42],false,[]]");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  int seen = 0;
  Collection::visitRecursive(
      s, Collection::PreOrder,
      [&seen](Slice const& key, Slice const& value) -> bool {
        EXPECT_TRUE(key.isNone());
        switch (seen) {
          case 0:
            EXPECT_TRUE(value.isNumber());
            EXPECT_EQ(1UL, value.getUInt());
            break;
          case 1:
            EXPECT_TRUE(value.isNull());
            break;
          case 2:
            EXPECT_TRUE(value.isTrue());
            break;
          case 3:
            EXPECT_TRUE(value.isString());
            EXPECT_EQ("foo", value.copyString());
            break;
          case 4:
            EXPECT_TRUE(value.isNumber());
            EXPECT_EQ(23UL, value.getUInt());
            break;
          case 5:
            EXPECT_TRUE(value.isNumber());
            EXPECT_EQ(42UL, value.getUInt());
            break;
          case 6:
            EXPECT_TRUE(value.isArray());
            EXPECT_EQ(2UL, value.length());
            break;
          case 7:
            EXPECT_TRUE(value.isFalse());
            break;
          case 8:
            EXPECT_TRUE(value.isArray());
            EXPECT_EQ(0UL, value.length());
            break;
        }
        ++seen;
        return true;
      });

  ASSERT_EQ(9, seen);
}

TEST(CollectionTest, VisitRecursiveArrayPostOrder) {
  std::string const value("[1,null,true,\"foo\",[23,42],false,[]]");

  Parser parser;
  parser.parse(value);
  Slice s(parser.start());

  int seen = 0;
  Collection::visitRecursive(
      s, Collection::PostOrder,
      [&seen](Slice const& key, Slice const& value) -> bool {
        EXPECT_TRUE(key.isNone());
        switch (seen) {
          case 0:
            EXPECT_TRUE(value.isNumber());
            EXPECT_EQ(1UL, value.getUInt());
            break;
          case 1:
            EXPECT_TRUE(value.isNull());
            break;
          case 2:
            EXPECT_TRUE(value.isTrue());
            break;
          case 3:
            EXPECT_TRUE(value.isString());
            EXPECT_EQ("foo", value.copyString());
            break;
          case 4:
            EXPECT_TRUE(value.isArray());
            EXPECT_EQ(2UL, value.length());
            break;
          case 5:
            EXPECT_TRUE(value.isNumber());
            EXPECT_EQ(23UL, value.getUInt());
            break;
          case 6:
            EXPECT_TRUE(value.isNumber());
            EXPECT_EQ(42UL, value.getUInt());
            break;
          case 7:
            EXPECT_TRUE(value.isFalse());
            break;
          case 8:
            EXPECT_TRUE(value.isArray());
            EXPECT_EQ(0UL, value.length());
            break;
        }
        ++seen;
        return true;
      });

  ASSERT_EQ(9, seen);
}

TEST(CollectionTest, VisitRecursiveObjectPreOrder) {
  std::string const value(
      "{\"foo\":1,\"bar\":null,\"baz\":true,\"bark\":{\"qux\":23,\"quetzal\":"
      "42},\"quux\":{}}");

  Options options;
  options.sortAttributeNames = false;
  Parser parser(&options);
  parser.parse(value);
  Slice s(parser.start());

  int seen = 0;
  Collection::visitRecursive(
      s, Collection::PreOrder,
      [&seen](Slice const& key, Slice const& value) -> bool {
        EXPECT_FALSE(key.isNone());
        switch (seen) {
          case 0:
            EXPECT_EQ("foo", key.copyString());
            EXPECT_TRUE(value.isNumber());
            EXPECT_EQ(1UL, value.getUInt());
            break;
          case 1:
            EXPECT_EQ("bar", key.copyString());
            EXPECT_TRUE(value.isNull());
            break;
          case 2:
            EXPECT_EQ("baz", key.copyString());
            EXPECT_TRUE(value.isTrue());
            break;
          case 3:
            EXPECT_EQ("qux", key.copyString());
            EXPECT_TRUE(value.isNumber());
            EXPECT_EQ(23UL, value.getUInt());
            break;
          case 4:
            EXPECT_EQ("quetzal", key.copyString());
            EXPECT_TRUE(value.isNumber());
            EXPECT_EQ(42UL, value.getUInt());
            break;
          case 5:
            EXPECT_EQ("bark", key.copyString());
            EXPECT_TRUE(value.isObject());
            EXPECT_EQ(2UL, value.length());
            break;
          case 6:
            EXPECT_EQ("quux", key.copyString());
            EXPECT_TRUE(value.isObject());
            EXPECT_EQ(0UL, value.length());
            break;
        }
        ++seen;
        return true;
      });

  ASSERT_EQ(7, seen);
}

TEST(CollectionTest, VisitRecursiveObjectPostOrder) {
  std::string const value(
      "{\"foo\":1,\"bar\":null,\"baz\":true,\"bark\":{\"qux\":23,\"quetzal\":"
      "42},\"quux\":{}}");

  Options options;
  options.sortAttributeNames = false;
  Parser parser(&options);
  parser.parse(value);
  Slice s(parser.start());

  int seen = 0;
  Collection::visitRecursive(
      s, Collection::PostOrder,
      [&seen](Slice const& key, Slice const& value) -> bool {
        EXPECT_FALSE(key.isNone());
        switch (seen) {
          case 0:
            EXPECT_EQ("foo", key.copyString());
            EXPECT_TRUE(value.isNumber());
            EXPECT_EQ(1UL, value.getUInt());
            break;
          case 1:
            EXPECT_EQ("bar", key.copyString());
            EXPECT_TRUE(value.isNull());
            break;
          case 2:
            EXPECT_EQ("baz", key.copyString());
            EXPECT_TRUE(value.isTrue());
            break;
          case 3:
            EXPECT_EQ("bark", key.copyString());
            EXPECT_TRUE(value.isObject());
            EXPECT_EQ(2UL, value.length());
            break;
          case 4:
            EXPECT_EQ("qux", key.copyString());
            EXPECT_TRUE(value.isNumber());
            EXPECT_EQ(23UL, value.getUInt());
            break;
          case 5:
            EXPECT_EQ("quetzal", key.copyString());
            EXPECT_TRUE(value.isNumber());
            EXPECT_EQ(42UL, value.getUInt());
            break;
          case 6:
            EXPECT_EQ("quux", key.copyString());
            EXPECT_TRUE(value.isObject());
            EXPECT_EQ(0UL, value.length());
            break;
        }
        ++seen;
        return true;
      });

  ASSERT_EQ(7, seen);
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
