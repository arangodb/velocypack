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
/// @author Manuel Pöter
/// @author Copyright 2021, ArangoDB GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include <gtest/gtest.h>

#include <iostream>
#include <list>
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

#include "tests-common.h"
#include "velocypack/Inspect/LoadInspector.h"
#include "velocypack/Inspect/SaveInspector.h"
#include "velocypack/Value.h"
#include "velocypack/ValueType.h"

namespace {

struct Dummy {
  int i;
  double d;
  bool b;
  std::string s;
};

template<class Inspector>
bool inspect(Inspector& f, Dummy& x) {
  return f.object().fields(f.field("i", x.i), f.field("d", x.d),
                           f.field("b", x.b), f.field("s", x.s));
}

struct Nested {
  Dummy dummy;
};

template<class Inspector>
bool inspect(Inspector& f, Nested& x) {
  return f.object().fields(f.field("dummy", x.dummy));
}

struct TypedInt {
  int value;
};

struct Container {
  TypedInt i;
};

template<class Inspector>
bool inspect(Inspector& f, TypedInt& x) {
  return f.apply(x.value);
}

template<class Inspector>
bool inspect(Inspector& f, Container& x) {
  return f.object().fields(f.field("i", x.i));
}
struct List {
  std::vector<int> vec;
  std::list<int> list;
};

template<class Inspector>
bool inspect(Inspector& f, List& x) {
  return f.object().fields(f.field("vec", x.vec), f.field("list", x.list));
}

struct Map {
  std::map<std::string, int> map;
  std::unordered_map<std::string, int> unordered;
};

template<class Inspector>
bool inspect(Inspector& f, Map& x) {
  return f.object().fields(f.field("map", x.map),
                           f.field("unordered", x.unordered));
}

struct Tuple {
  std::tuple<std::string, int, double> tuple;
  std::pair<int, std::string> pair;
  std::string array1[2];
  std::array<int, 3> array2;
};

template<class Inspector>
bool inspect(Inspector& f, Tuple& x) {
  return f.object().fields(f.field("tuple", x.tuple), f.field("pair", x.pair),
                           f.field("array1", x.array1),
                           f.field("array2", x.array2));
}

struct Optional {
  std::optional<int> x;
  std::optional<std::string> y;
  std::vector<std::optional<int>> vec;
  std::map<std::string, std::optional<int>> map;
};

template<class Inspector>
bool inspect(Inspector& f, Optional& x) {
  return f.object().fields(f.field("x", x.x), f.field("y", x.y),
                           f.field("vec", x.vec), f.field("map", x.map));
}

struct Pointer {
  std::shared_ptr<int> a;
  std::shared_ptr<int> b;
  std::unique_ptr<int> c;
  std::unique_ptr<int> d;
};

template<class Inspector>
bool inspect(Inspector& f, Pointer& x) {
  return f.object().fields(f.field("a", x.a), f.field("b", x.b),
                           f.field("c", x.c), f.field("d", x.d));
}

using namespace arangodb::velocypack;

struct SaveInspectorTest : public ::testing::Test {
  Builder builder;
  SaveInspector inspector{builder};
};

TEST_F(SaveInspectorTest, store_int) {
  int x = 42;
  auto success = inspector.apply(x);
  EXPECT_TRUE(success);
  EXPECT_EQ(x, builder.slice().getInt());
}

TEST_F(SaveInspectorTest, store_double) {
  double x = 123.456;
  auto success = inspector.apply(x);
  EXPECT_TRUE(success);
  EXPECT_EQ(x, builder.slice().getDouble());
}

TEST_F(SaveInspectorTest, store_bool) {
  bool x = true;
  auto success = inspector.apply(x);
  EXPECT_TRUE(success);
  EXPECT_EQ(x, builder.slice().getBool());
}

TEST_F(SaveInspectorTest, store_string) {
  std::string x = "foobar";
  auto success = inspector.apply(x);
  EXPECT_TRUE(success);
  EXPECT_EQ(x, builder.slice().copyString());
}

TEST_F(SaveInspectorTest, store_object) {
  static_assert(inspection::HasInspectOverload<Dummy, SaveInspector>);

  Dummy f{.i = 42, .d = 123.456, .b = true, .s = "foobar"};
  auto success = inspector.apply(f);
  ASSERT_TRUE(success);

  Slice slice = builder.slice();
  ASSERT_TRUE(slice.isObject());
  EXPECT_EQ(f.i, slice["i"].getInt());
  EXPECT_EQ(f.d, slice["d"].getDouble());
  EXPECT_EQ(f.b, slice["b"].getBool());
  EXPECT_EQ(f.s, slice["s"].copyString());
}

TEST_F(SaveInspectorTest, store_nested_object) {
  static_assert(inspection::HasInspectOverload<Nested, SaveInspector>);

  Nested b{.dummy = {.i = 42, .d = 123.456, .b = true, .s = "foobar"}};
  auto success = inspector.apply(b);
  ASSERT_TRUE(success);

  Slice slice = builder.slice();
  ASSERT_TRUE(slice.isObject());
  auto d = slice["dummy"];
  ASSERT_TRUE(d.isObject());
  EXPECT_EQ(b.dummy.i, d["i"].getInt());
  EXPECT_EQ(b.dummy.d, d["d"].getDouble());
  EXPECT_EQ(b.dummy.b, d["b"].getBool());
  EXPECT_EQ(b.dummy.s, d["s"].copyString());
}

TEST_F(SaveInspectorTest, store_nested_object_without_nesting) {
  static_assert(inspection::HasInspectOverload<Container, SaveInspector>);

  Container c{.i = {.value = 42}};
  auto success = inspector.apply(c);
  ASSERT_TRUE(success);

  Slice slice = builder.slice();
  ASSERT_TRUE(slice.isObject());
  EXPECT_EQ(c.i.value, slice["i"].getInt());
}

TEST_F(SaveInspectorTest, store_list) {
  static_assert(inspection::HasInspectOverload<List, SaveInspector>);

  List l{.vec = {1, 2, 3}, .list = {4, 5}};
  auto success = inspector.apply(l);
  ASSERT_TRUE(success);

  Slice slice = builder.slice();
  ASSERT_TRUE(slice.isObject());
  auto list = slice["vec"];
  ASSERT_TRUE(list.isArray());
  ASSERT_EQ(3, list.length());
  EXPECT_EQ(l.vec[0], list[0].getInt());
  EXPECT_EQ(l.vec[1], list[1].getInt());
  EXPECT_EQ(l.vec[2], list[2].getInt());

  list = slice["list"];
  ASSERT_TRUE(list.isArray());
  ASSERT_EQ(2, list.length());
  auto it = l.list.begin();
  EXPECT_EQ(*it++, list[0].getInt());
  EXPECT_EQ(*it++, list[1].getInt());
}

TEST_F(SaveInspectorTest, store_map) {
  static_assert(inspection::HasInspectOverload<Map, SaveInspector>);

  Map m{.map = {{"1", 1}, {"2", 2}, {"3", 3}},
        .unordered = {{"4", 4}, {"5", 5}}};
  auto success = inspector.apply(m);
  ASSERT_TRUE(success);

  Slice slice = builder.slice();
  ASSERT_TRUE(slice.isObject());
  auto obj = slice["map"];
  ASSERT_TRUE(obj.isObject());
  ASSERT_EQ(3, obj.length());
  EXPECT_EQ(m.map["1"], obj["1"].getInt());
  EXPECT_EQ(m.map["2"], obj["2"].getInt());
  EXPECT_EQ(m.map["3"], obj["3"].getInt());

  obj = slice["unordered"];
  ASSERT_TRUE(obj.isObject());
  ASSERT_EQ(2, obj.length());
  EXPECT_EQ(m.unordered["4"], obj["4"].getInt());
  EXPECT_EQ(m.unordered["5"], obj["5"].getInt());
}

TEST_F(SaveInspectorTest, store_tuples) {
  static_assert(inspection::HasInspectOverload<Tuple, SaveInspector>);

  Tuple t{.tuple = {"foo", 42, 12.34},
          .pair = {987, "bar"},
          .array1 = {"a", "b"},
          .array2 = {1, 2, 3}};
  auto success = inspector.apply(t);
  ASSERT_TRUE(success);

  Slice slice = builder.slice();
  ASSERT_TRUE(slice.isObject());
  auto list = slice["tuple"];
  ASSERT_EQ(3, list.length());
  EXPECT_EQ(std::get<0>(t.tuple), list[0].copyString());
  EXPECT_EQ(std::get<1>(t.tuple), list[1].getInt());
  EXPECT_EQ(std::get<2>(t.tuple), list[2].getDouble());

  list = slice["pair"];
  ASSERT_EQ(2, list.length());
  EXPECT_EQ(std::get<0>(t.pair), list[0].getInt());
  EXPECT_EQ(std::get<1>(t.pair), list[1].copyString());

  list = slice["array1"];
  ASSERT_EQ(2, list.length());
  EXPECT_EQ(t.array1[0], list[0].copyString());
  EXPECT_EQ(t.array1[1], list[1].copyString());

  list = slice["array2"];
  ASSERT_EQ(3, list.length());
  EXPECT_EQ(t.array2[0], list[0].getInt());
  EXPECT_EQ(t.array2[1], list[1].getInt());
  EXPECT_EQ(t.array2[2], list[2].getInt());
}

TEST_F(SaveInspectorTest, store_optional) {
  static_assert(inspection::HasInspectOverload<Optional, SaveInspector>);

  Optional o{.x = std::nullopt,
             .y = "blubb",
             .vec = {1, std::nullopt, 3},
             .map = {{"1", 1}, {"2", std::nullopt}, {"3", 3}}};
  auto success = inspector.apply(o);
  ASSERT_TRUE(success);

  Slice slice = builder.slice();
  ASSERT_TRUE(slice.isObject());
  EXPECT_EQ(3, slice.length());
  EXPECT_EQ("blubb", slice["y"].copyString());

  auto vec = slice["vec"];
  ASSERT_TRUE(vec.isArray());
  ASSERT_EQ(3, vec.length());
  EXPECT_EQ(1, vec[0].getInt());
  EXPECT_TRUE(vec[1].isNull());
  EXPECT_EQ(3, vec[2].getInt());

  auto map = slice["map"];
  ASSERT_TRUE(map.isObject());
  ASSERT_EQ(3, map.length());
  EXPECT_EQ(1, map["1"].getInt());
  EXPECT_TRUE(map["2"].isNull());
  EXPECT_EQ(3, map["3"].getInt());
}

TEST_F(SaveInspectorTest, store_optional_pointer) {
  static_assert(inspection::HasInspectOverload<Pointer, SaveInspector>);

  Pointer p{.a = nullptr,
            .b = std::make_shared<int>(42),
            .c = nullptr,
            .d = std::make_unique<int>(43)};
  auto success = inspector.apply(p);
  ASSERT_TRUE(success);

  Slice slice = builder.slice();
  ASSERT_TRUE(slice.isObject());
  EXPECT_EQ(2, slice.length());
  EXPECT_EQ(42, slice["b"].getInt());
  EXPECT_EQ(43, slice["d"].getInt());
}

struct LoadInspectorTest : public ::testing::Test {
  Builder builder;
};

TEST_F(LoadInspectorTest, load_int) {
  builder.add(VPackValue(42));
  LoadInspector inspector{builder};

  int x = 0;
  auto success = inspector.apply(x);
  EXPECT_TRUE(success);
  EXPECT_EQ(42, x);
}

TEST_F(LoadInspectorTest, load_double) {
  builder.add(VPackValue(123.456));
  LoadInspector inspector{builder};

  double x = 0;
  auto success = inspector.apply(x);
  EXPECT_TRUE(success);
  EXPECT_EQ(123.456, x);
}

TEST_F(LoadInspectorTest, load_bool) {
  builder.add(VPackValue(true));
  LoadInspector inspector{builder};

  bool x = false;
  auto success = inspector.apply(x);
  EXPECT_TRUE(success);
  EXPECT_EQ(true, x);
}

TEST_F(LoadInspectorTest, store_string) {
  builder.add(VPackValue("foobar"));
  LoadInspector inspector{builder};

  std::string x;
  auto success = inspector.apply(x);
  EXPECT_TRUE(success);
  EXPECT_EQ("foobar", x);
}

TEST_F(LoadInspectorTest, load_object) {
  builder.openObject();
  builder.add("i", VPackValue(42));
  builder.add("d", VPackValue(123.456));
  builder.add("b", VPackValue(true));
  builder.add("s", VPackValue("foobar"));
  builder.close();
  LoadInspector inspector{builder};

  Dummy d;
  auto success = inspector.apply(d);
  ASSERT_TRUE(success);
  EXPECT_EQ(42, d.i);
  EXPECT_EQ(123.456, d.d);
  EXPECT_EQ(true, d.b);
  EXPECT_EQ("foobar", d.s);
}

TEST_F(LoadInspectorTest, load_nested_object) {
  builder.openObject();
  builder.add(VPackValue("dummy"));
  builder.openObject();
  builder.add("i", VPackValue(42));
  builder.add("d", VPackValue(123.456));
  builder.add("b", VPackValue(true));
  builder.add("s", VPackValue("foobar"));
  builder.close();
  builder.close();
  LoadInspector inspector{builder};

  Nested n;
  auto success = inspector.apply(n);
  ASSERT_TRUE(success);
  EXPECT_EQ(42, n.dummy.i);
  EXPECT_EQ(123.456, n.dummy.d);
  EXPECT_EQ(true, n.dummy.b);
  EXPECT_EQ("foobar", n.dummy.s);
}

TEST_F(LoadInspectorTest, load_nested_object_without_nesting) {
  builder.openObject();
  builder.add("i", VPackValue(42));
  builder.close();
  LoadInspector inspector{builder};

  Container c;
  auto success = inspector.apply(c);
  ASSERT_TRUE(success);
  EXPECT_EQ(42, c.i.value);
}

TEST_F(LoadInspectorTest, load_list) {
  builder.openObject();
  builder.add(VPackValue("vec"));
  builder.openArray();
  builder.add(VPackValue(1));
  builder.add(VPackValue(2));
  builder.add(VPackValue(3));
  builder.close();
  builder.add(VPackValue("list"));
  builder.openArray();
  builder.add(VPackValue(4));
  builder.add(VPackValue(5));
  builder.close();
  builder.close();
  LoadInspector inspector{builder};

  List l;
  auto success = inspector.apply(l);
  ASSERT_TRUE(success);

  EXPECT_EQ((std::vector<int>{1, 2, 3}), l.vec);
  EXPECT_EQ((std::list<int>{4, 5}), l.list);
}

TEST_F(LoadInspectorTest, load_map) {
  builder.openObject();
  builder.add(VPackValue("map"));
  builder.openObject();
  builder.add("1", VPackValue(1));
  builder.add("2", VPackValue(2));
  builder.add("3", VPackValue(3));
  builder.close();
  builder.add(VPackValue("unordered"));
  builder.openObject();
  builder.add("4", VPackValue(4));
  builder.add("5", VPackValue(5));
  builder.close();
  builder.close();
  LoadInspector inspector{builder};

  Map m;
  auto success = inspector.apply(m);
  ASSERT_TRUE(success);

  EXPECT_EQ((std::map<std::string, int>{{"1", 1}, {"2", 2}, {"3", 3}}), m.map);
  EXPECT_EQ((std::unordered_map<std::string, int>{{"4", 4}, {"5", 5}}),
            m.unordered);
}

TEST_F(LoadInspectorTest, load_tuples) {
  builder.openObject();

  builder.add(VPackValue("tuple"));
  builder.openArray();
  builder.add(VPackValue("foo"));
  builder.add(VPackValue(42));
  builder.add(VPackValue(12.34));
  builder.close();

  builder.add(VPackValue("pair"));
  builder.openArray();
  builder.add(VPackValue(987));
  builder.add(VPackValue("bar"));
  builder.close();

  builder.add(VPackValue("array1"));
  builder.openArray();
  builder.add(VPackValue("a"));
  builder.add(VPackValue("b"));
  builder.close();

  builder.add(VPackValue("array2"));
  builder.openArray();
  builder.add(VPackValue(1));
  builder.add(VPackValue(2));
  builder.add(VPackValue(3));
  builder.close();

  builder.close();
  LoadInspector inspector{builder};

  Tuple t;
  auto success = inspector.apply(t);
  ASSERT_TRUE(success);

  Tuple expected{.tuple = {"foo", 42, 12.34},
                 .pair = {987, "bar"},
                 .array1 = {"a", "b"},
                 .array2 = {1, 2, 3}};
  EXPECT_EQ(expected.tuple, t.tuple);
  EXPECT_EQ(expected.pair, t.pair);
  EXPECT_EQ(expected.array1[0], t.array1[0]);
  EXPECT_EQ(expected.array1[1], t.array1[1]);
  EXPECT_EQ(expected.array2, t.array2);
}

TEST_F(LoadInspectorTest, load_optional) {
  builder.openObject();
  builder.add("y", VPackValue("blubb"));

  builder.add(VPackValue("vec"));
  builder.openArray();
  builder.add(VPackValue(1));
  builder.add(VPackValue(ValueType::Null));
  builder.add(VPackValue(3));
  builder.close();

  builder.add(VPackValue("map"));
  builder.openObject();
  builder.add("1", VPackValue(1));
  builder.add("2", VPackValue(ValueType::Null));
  builder.add("3", VPackValue(3));
  builder.close();

  builder.close();
  LoadInspector inspector{builder};

  Optional o;
  auto success = inspector.apply(o);
  ASSERT_TRUE(success);

  Optional expected{.x = std::nullopt,
                    .y = "blubb",
                    .vec = {1, std::nullopt, 3},
                    .map = {{"1", 1}, {"2", std::nullopt}, {"3", 3}}};
  EXPECT_EQ(expected.x, o.x);
  EXPECT_EQ(expected.y, o.y);
  ASSERT_EQ(expected.vec, o.vec);
  EXPECT_EQ(expected.map, o.map);
}

}  // namespace

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}