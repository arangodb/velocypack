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
#include "velocypack/inspection/LoadInspector.h"
#include "velocypack/inspection/SaveInspector.h"
#include "velocypack/Value.h"
#include "velocypack/ValueType.h"
#include "velocypack/velocypack-memory.h"

namespace {

struct Dummy {
  int i;
  double d;
  bool b;
  std::string s;
};

template<class Inspector>
auto inspect(Inspector& f, Dummy& x) {
  return f.object(x).fields(f.field("i", x.i), f.field("d", x.d),
                            f.field("b", x.b), f.field("s", x.s));
}

struct Nested {
  Dummy dummy;
};

template<class Inspector>
auto inspect(Inspector& f, Nested& x) {
  return f.object(x).fields(f.field("dummy", x.dummy));
}

struct TypedInt {
  int value;
};

struct Container {
  TypedInt i;
};

template<class Inspector>
auto inspect(Inspector& f, TypedInt& x) {
  return f.apply(x.value);
}

template<class Inspector>
auto inspect(Inspector& f, Container& x) {
  return f.object(x).fields(f.field("i", x.i));
}
struct List {
  std::vector<int> vec;
  std::list<int> list;
};

template<class Inspector>
auto inspect(Inspector& f, List& x) {
  return f.object(x).fields(f.field("vec", x.vec), f.field("list", x.list));
}

struct Map {
  std::map<std::string, int> map;
  std::unordered_map<std::string, int> unordered;
};

template<class Inspector>
auto inspect(Inspector& f, Map& x) {
  return f.object(x).fields(f.field("map", x.map),
                            f.field("unordered", x.unordered));
}

struct Tuple {
  std::tuple<std::string, int, double> tuple;
  std::pair<int, std::string> pair;
  std::string array1[2];
  std::array<int, 3> array2;
};

template<class Inspector>
auto inspect(Inspector& f, Tuple& x) {
  return f.object(x).fields(f.field("tuple", x.tuple), f.field("pair", x.pair),
                            f.field("array1", x.array1),
                            f.field("array2", x.array2));
}

struct Optional {
  std::optional<int> a;
  std::optional<int> b;
  std::optional<int> x;
  std::optional<std::string> y;
  std::vector<std::optional<int>> vec;
  std::map<std::string, std::optional<int>> map;
};

template<class Inspector>
auto inspect(Inspector& f, Optional& x) {
  return f.object(x).fields(f.field("a", x.a).fallback(123),
                            f.field("b", x.b).fallback(456), f.field("x", x.x),
                            f.field("y", x.y), f.field("vec", x.vec),
                            f.field("map", x.map));
}

struct Pointer {
  std::shared_ptr<int> a;
  std::shared_ptr<int> b;
  std::unique_ptr<int> c;
  std::unique_ptr<Container> d;
  std::vector<std::unique_ptr<int>> vec;
  std::shared_ptr<int> x;
  std::shared_ptr<int> y;
};

template<class Inspector>
auto inspect(Inspector& f, Pointer& x) {
  return f.object(x).fields(
      f.field("a", x.a), f.field("b", x.b), f.field("c", x.c),
      f.field("d", x.d), f.field("vec", x.vec),
      f.field("x", x.x).fallback(std::make_shared<int>(123)),
      f.field("y", x.y).fallback(std::make_shared<int>(456)));
}

struct Fallback {
  int i;
  std::string s;
};

template<class Inspector>
auto inspect(Inspector& f, Fallback& x) {
  return f.object(x).fields(f.field("i", x.i).fallback(42),
                            f.field("s", x.s).fallback("foobar"));
}

struct Invariant {
  int i;
  std::string s;
};

template<class Inspector>
auto inspect(Inspector& f, Invariant& x) {
  return f.object(x).fields(
      f.field("i", x.i).invariant([](int v) { return v != 0; }),
      f.field("s", x.s).invariant(
          [](std::string const& v) { return !v.empty(); }));
}

struct InvariantAndFallback {
  int i;
  std::string s;
};

template<class Inspector>
auto inspect(Inspector& f, InvariantAndFallback& x) {
  return f.object(x).fields(
      f.field("i", x.i).fallback(42).invariant([](int v) { return v != 0; }),
      f.field("s", x.s)
          .invariant([](std::string const& v) { return !v.empty(); })
          .fallback("foobar"));
}

struct ObjectInvariant {
  int i;
  std::string s;
};

template<class Inspector>
auto inspect(Inspector& f, ObjectInvariant& x) {
  return f.object(x)
      .fields(f.field("i", x.i), f.field("s", x.s))
      .invariant([](ObjectInvariant& o) { return o.i != 0 && !o.s.empty(); });
}

struct Specialization {
  int i;
  std::string s;
};
}  // namespace

namespace arangodb::velocypack::inspection {
template<>
struct InspectorAccess<Specialization> {
  template<class Inspector>
  [[nodiscard]] static Result apply(Inspector& f, Specialization& val) {
    f.beginArray();
    f.apply(val.i);
    f.apply(val.s);
    f.endArray();
    return {};
  }
};
}  // namespace arangodb::velocypack::inspection

namespace {
using namespace arangodb::velocypack;
using LoadInspector = inspection::LoadInspector;
using SaveInspector = inspection::SaveInspector;

struct SaveInspectorTest : public ::testing::Test {
  Builder builder;
  SaveInspector inspector{builder};
};

TEST_F(SaveInspectorTest, store_int) {
  int x = 42;
  auto result = inspector.apply(x);
  EXPECT_TRUE(result.ok());
  EXPECT_EQ(x, builder.slice().getInt());
}

TEST_F(SaveInspectorTest, store_double) {
  double x = 123.456;
  auto result = inspector.apply(x);
  EXPECT_TRUE(result.ok());
  EXPECT_EQ(x, builder.slice().getDouble());
}

TEST_F(SaveInspectorTest, store_bool) {
  bool x = true;
  auto result = inspector.apply(x);
  EXPECT_TRUE(result.ok());
  EXPECT_EQ(x, builder.slice().getBool());
}

TEST_F(SaveInspectorTest, store_string) {
  std::string x = "foobar";
  auto result = inspector.apply(x);
  EXPECT_TRUE(result.ok());
  EXPECT_EQ(x, builder.slice().copyString());
}

TEST_F(SaveInspectorTest, store_object) {
  static_assert(inspection::HasInspectOverload<Dummy, SaveInspector>);

  Dummy f{.i = 42, .d = 123.456, .b = true, .s = "foobar"};
  auto result = inspector.apply(f);
  ASSERT_TRUE(result.ok());

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
  auto result = inspector.apply(b);
  ASSERT_TRUE(result.ok());

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
  auto result = inspector.apply(c);
  ASSERT_TRUE(result.ok());

  Slice slice = builder.slice();
  ASSERT_TRUE(slice.isObject());
  EXPECT_EQ(c.i.value, slice["i"].getInt());
}

TEST_F(SaveInspectorTest, store_list) {
  static_assert(inspection::HasInspectOverload<List, SaveInspector>);

  List l{.vec = {1, 2, 3}, .list = {4, 5}};
  auto result = inspector.apply(l);
  ASSERT_TRUE(result.ok());

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
  auto result = inspector.apply(m);
  ASSERT_TRUE(result.ok());

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
  auto result = inspector.apply(t);
  ASSERT_TRUE(result.ok());

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

  Optional o{.a = std::nullopt,
             .b = std::nullopt,
             .x = std::nullopt,
             .y = "blubb",
             .vec = {1, std::nullopt, 3},
             .map = {{"1", 1}, {"2", std::nullopt}, {"3", 3}}};
  auto result = inspector.apply(o);
  ASSERT_TRUE(result.ok());

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
            .d = std::make_unique<Container>(Container{.i = {.value = 43}}),
            .vec = {}};
  p.vec.push_back(std::make_unique<int>(1));
  p.vec.push_back(nullptr);
  p.vec.push_back(std::make_unique<int>(2));
  auto result = inspector.apply(p);
  ASSERT_TRUE(result.ok());

  Slice slice = builder.slice();
  ASSERT_TRUE(slice.isObject());
  EXPECT_EQ(3, slice.length());
  EXPECT_EQ(42, slice["b"].getInt());
  EXPECT_EQ(43, slice["d"]["i"].getInt());
  auto vec = slice["vec"];
  EXPECT_TRUE(vec.isArray());
  EXPECT_EQ(3, vec.length());
  EXPECT_EQ(1, vec[0].getInt());
  EXPECT_TRUE(vec[1].isNull());
  EXPECT_EQ(2, vec[2].getInt());
}

TEST_F(SaveInspectorTest, save_object_with_fallbacks) {
  Fallback f;
  auto result = inspector.apply(f);
  ASSERT_TRUE(result.ok());

  static_assert(sizeof(inspector.field("i", f.i)) ==
                sizeof(inspector.field("i", f.i).fallback(42)));
}

TEST_F(SaveInspectorTest, save_object_with_invariant) {
  Invariant i;
  auto result = inspector.apply(i);
  ASSERT_TRUE(result.ok());

  auto invariant = [](auto) { return true; };
  static_assert(sizeof(inspector.field("i", i.i)) ==
                sizeof(inspector.field("i", i.i).invariant(invariant)));
}

TEST_F(SaveInspectorTest, save_object_with_invariant_and_fallback) {
  InvariantAndFallback i;
  auto result = inspector.apply(i);
  ASSERT_TRUE(result.ok());

  auto invariant = [](auto) { return true; };
  static_assert(
      sizeof(inspector.field("i", i.i)) ==
      sizeof(inspector.field("i", i.i).invariant(invariant).fallback(42)));
  static_assert(
      sizeof(inspector.field("i", i.i)) ==
      sizeof(inspector.field("i", i.i).fallback(42).invariant(invariant)));
}

struct LoadInspectorTest : public ::testing::Test {
  Builder builder;
};

TEST_F(LoadInspectorTest, load_int) {
  builder.add(VPackValue(42));
  LoadInspector inspector{builder};

  int x = 0;
  auto result = inspector.apply(x);
  EXPECT_TRUE(result.ok());
  EXPECT_EQ(42, x);
}

TEST_F(LoadInspectorTest, load_double) {
  builder.add(VPackValue(123.456));
  LoadInspector inspector{builder};

  double x = 0;
  auto result = inspector.apply(x);
  EXPECT_TRUE(result.ok());
  EXPECT_EQ(123.456, x);
}

TEST_F(LoadInspectorTest, load_bool) {
  builder.add(VPackValue(true));
  LoadInspector inspector{builder};

  bool x = false;
  auto result = inspector.apply(x);
  EXPECT_TRUE(result.ok());
  EXPECT_EQ(true, x);
}

TEST_F(LoadInspectorTest, store_string) {
  builder.add(VPackValue("foobar"));
  LoadInspector inspector{builder};

  std::string x;
  auto result = inspector.apply(x);
  EXPECT_TRUE(result.ok());
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
  auto result = inspector.apply(d);
  ASSERT_TRUE(result.ok());
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
  builder.add("d", VPackValue(123));
  builder.add("b", VPackValue(true));
  builder.add("s", VPackValue("foobar"));
  builder.close();
  builder.close();
  LoadInspector inspector{builder};

  Nested n;
  auto result = inspector.apply(n);
  ASSERT_TRUE(result.ok());
  EXPECT_EQ(42, n.dummy.i);
  EXPECT_EQ(123, n.dummy.d);
  EXPECT_EQ(true, n.dummy.b);
  EXPECT_EQ("foobar", n.dummy.s);
}

TEST_F(LoadInspectorTest, load_nested_object_without_nesting) {
  builder.openObject();
  builder.add("i", VPackValue(42));
  builder.close();
  LoadInspector inspector{builder};

  Container c;
  auto result = inspector.apply(c);
  ASSERT_TRUE(result.ok());
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
  auto result = inspector.apply(l);
  ASSERT_TRUE(result.ok());

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
  auto result = inspector.apply(m);
  ASSERT_TRUE(result.ok());

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
  auto result = inspector.apply(t);
  ASSERT_TRUE(result.ok());

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

  builder.add("a", VPackValue(ValueType::Null));
  builder.close();
  LoadInspector inspector{builder};

  Optional o{.a = 1, .b = 2, .x = 42};
  auto result = inspector.apply(o);
  ASSERT_TRUE(result.ok());

  Optional expected{.a = std::nullopt,
                    .b = 456,
                    .x = std::nullopt,
                    .y = "blubb",
                    .vec = {1, std::nullopt, 3},
                    .map = {{"1", 1}, {"2", std::nullopt}, {"3", 3}}};
  EXPECT_EQ(expected.a, o.a);
  EXPECT_EQ(expected.b, o.b);
  EXPECT_EQ(expected.x, o.x);
  EXPECT_EQ(expected.y, o.y);
  ASSERT_EQ(expected.vec, o.vec);
  EXPECT_EQ(expected.map, o.map);
}

TEST_F(LoadInspectorTest, load_optional_pointer) {
  builder.openObject();
  builder.add(VPackValue("vec"));
  builder.openArray();
  builder.add(VPackValue(1));
  builder.add(VPackValue(VPackValueType::Null));
  builder.add(VPackValue(2));
  builder.close();

  builder.add("a", VPackValue(VPackValueType::Null));

  builder.add("b", VPackValue(42));

  builder.add(VPackValue("d"));
  builder.openObject();
  builder.add("i", VPackValue(43));
  builder.close();

  builder.add("x", VPackValue(VPackValueType::Null));

  builder.close();
  LoadInspector inspector{builder};

  Pointer p{
      .a = std::make_shared<int>(0),
      .b = std::make_shared<int>(0),
      .c = std::make_unique<int>(0),
      .d = std::make_unique<Container>(Container{.i = {.value = 0}}),
      .x = std::make_shared<int>(0),
      .y = std::make_shared<int>(0),
  };
  auto result = inspector.apply(p);
  ASSERT_TRUE(result.ok()) << result.error() << "; " << result.path();

  EXPECT_EQ(nullptr, p.a);
  ASSERT_NE(nullptr, p.b);
  EXPECT_EQ(42, *p.b);
  EXPECT_EQ(nullptr, p.c);
  ASSERT_NE(nullptr, p.d);
  EXPECT_EQ(43, p.d->i.value);

  ASSERT_EQ(3, p.vec.size());
  ASSERT_NE(nullptr, p.vec[0]);
  EXPECT_EQ(1, *p.vec[0]);
  EXPECT_EQ(nullptr, p.vec[1]);
  ASSERT_NE(nullptr, p.vec[2]);
  EXPECT_EQ(2, *p.vec[2]);

  EXPECT_EQ(nullptr, p.x);
  ASSERT_NE(nullptr, p.y);
  EXPECT_EQ(456, *p.y);
}

TEST_F(LoadInspectorTest, error_expecting_int) {
  builder.add(VPackValue("foo"));
  LoadInspector inspector{builder};

  int i;
  auto result = inspector.apply(i);
  ASSERT_FALSE(result.ok());
  EXPECT_EQ("Expecting type Int", result.error());
}

TEST_F(LoadInspectorTest, error_expecting_int16) {
  builder.add(VPackValue(123456789));
  LoadInspector inspector{builder};

  std::int16_t i;
  auto result = inspector.apply(i);
  ASSERT_FALSE(result.ok());
  EXPECT_EQ("Number out of range", result.error());
}

TEST_F(LoadInspectorTest, error_expecting_double) {
  builder.add(VPackValue("foo"));
  LoadInspector inspector{builder};

  double d;
  auto result = inspector.apply(d);
  ASSERT_FALSE(result.ok());
  EXPECT_EQ("Expecting numeric type", result.error());
}

TEST_F(LoadInspectorTest, error_expecting_bool) {
  builder.add(VPackValue(42));
  LoadInspector inspector{builder};

  bool b;
  auto result = inspector.apply(b);
  ASSERT_FALSE(result.ok());
  EXPECT_EQ("Expecting type Bool", result.error());
}

TEST_F(LoadInspectorTest, error_expecting_string) {
  builder.add(VPackValue(42));
  LoadInspector inspector{builder};

  std::string s;
  auto result = inspector.apply(s);
  ASSERT_FALSE(result.ok());
  EXPECT_EQ("Expecting type String", result.error());
}

TEST_F(LoadInspectorTest, error_expecting_array) {
  builder.add(VPackValue(42));
  LoadInspector inspector{builder};

  std::vector<int> v;
  auto result = inspector.apply(v);
  ASSERT_FALSE(result.ok());
  EXPECT_EQ("Expecting type Array", result.error());
}

TEST_F(LoadInspectorTest, error_expecting_object) {
  builder.add(VPackValue(42));
  LoadInspector inspector{builder};

  Dummy d;
  auto result = inspector.apply(d);
  ASSERT_FALSE(result.ok());
  EXPECT_EQ("Expecting type Object", result.error());
}

TEST_F(LoadInspectorTest, error_tuple_array_too_short) {
  builder.openArray();
  builder.add(VPackValue("foo"));
  builder.add(VPackValue(42));
  builder.close();
  LoadInspector inspector{builder};

  std::tuple<std::string, int, double> t;
  auto result = inspector.apply(t);
  ASSERT_FALSE(result.ok());
  EXPECT_EQ("Expected array of length 3", result.error());
}

TEST_F(LoadInspectorTest, error_tuple_array_too_large) {
  builder.openArray();
  builder.add(VPackValue("foo"));
  builder.add(VPackValue(42));
  builder.add(VPackValue(123.456));
  builder.close();
  LoadInspector inspector{builder};

  std::tuple<std::string, int> t;
  auto result = inspector.apply(t);
  ASSERT_FALSE(result.ok());
  EXPECT_EQ("Expected array of length 2", result.error());
}

TEST_F(LoadInspectorTest, error_c_style_array_too_short) {
  builder.openArray();
  builder.add(VPackValue(1));
  builder.add(VPackValue(2));
  builder.close();
  LoadInspector inspector{builder};

  int a[4];
  auto result = inspector.apply(a);
  ASSERT_FALSE(result.ok());
  EXPECT_EQ("Expected array of length 4", result.error());
}

TEST_F(LoadInspectorTest, error_c_style_array_too_long) {
  builder.openArray();
  builder.add(VPackValue(1));
  builder.add(VPackValue(2));
  builder.add(VPackValue(3));
  builder.add(VPackValue(4));
  builder.close();
  LoadInspector inspector{builder};

  int a[3];
  auto result = inspector.apply(a);
  ASSERT_FALSE(result.ok());
  EXPECT_EQ("Expected array of length 3", result.error());
}

TEST_F(LoadInspectorTest, error_expecting_type_on_path) {
  builder.openObject();
  builder.add(VPackValue("dummy"));
  builder.openObject();
  builder.add("i", VPackValue("foo"));
  builder.close();
  builder.close();
  LoadInspector inspector{builder};

  Nested n;
  auto result = inspector.apply(n);
  ASSERT_FALSE(result.ok());
  EXPECT_EQ("dummy.i", result.path());
}

TEST_F(LoadInspectorTest, error_missing_field) {
  builder.openObject();
  builder.add(VPackValue("dummy"));
  builder.openObject();
  builder.add("s", VPackValue("foo"));
  builder.close();
  builder.close();
  LoadInspector inspector{builder};

  Nested n;
  auto result = inspector.apply(n);
  ASSERT_FALSE(result.ok());
  EXPECT_EQ("Missing required attribute 'i'", result.error());
  EXPECT_EQ("dummy.i", result.path());
}

TEST_F(LoadInspectorTest, load_object_with_fallbacks) {
  builder.openObject();
  builder.close();
  LoadInspector inspector{builder};

  Fallback f;
  auto result = inspector.apply(f);
  ASSERT_TRUE(result.ok());
  EXPECT_EQ(42, f.i);
  EXPECT_EQ("foobar", f.s);
}

TEST_F(LoadInspectorTest, load_object_with_invariant_fulfilled) {
  builder.openObject();
  builder.add("i", VPackValue(42));
  builder.add("s", VPackValue("foobar"));
  builder.close();
  LoadInspector inspector{builder};

  Invariant i;
  auto result = inspector.apply(i);
  ASSERT_TRUE(result.ok());
  EXPECT_EQ(42, i.i);
  EXPECT_EQ("foobar", i.s);
}

TEST_F(LoadInspectorTest, load_object_with_invariant_not_fulfilled) {
  {
    builder.openObject();
    builder.add("i", VPackValue(0));
    builder.add("s", VPackValue("foobar"));
    builder.close();
    LoadInspector inspector{builder};

    Invariant i;
    auto result = inspector.apply(i);
    ASSERT_FALSE(result.ok());
    EXPECT_EQ("Field invariant failed", result.error());
    EXPECT_EQ("i", result.path());
  }

  {
    builder.clear();
    builder.openObject();
    builder.add("i", VPackValue(42));
    builder.add("s", VPackValue(""));
    builder.close();
    LoadInspector inspector{builder};

    Invariant i;
    auto result = inspector.apply(i);
    ASSERT_FALSE(result.ok());
    EXPECT_EQ("Field invariant failed", result.error());
    EXPECT_EQ("s", result.path());
  }
}

TEST_F(LoadInspectorTest, load_object_with_invariant_and_fallback) {
  builder.openObject();
  builder.close();
  LoadInspector inspector{builder};

  InvariantAndFallback i;
  auto result = inspector.apply(i);
  ASSERT_TRUE(result.ok());
  EXPECT_EQ(42, i.i);
  EXPECT_EQ("foobar", i.s);
}

TEST_F(LoadInspectorTest, load_object_with_object_invariant) {
  builder.openObject();
  builder.add("i", VPackValue(42));
  builder.add("s", VPackValue(""));
  builder.close();
  LoadInspector inspector{builder};

  ObjectInvariant o;
  auto result = inspector.apply(o);
  ASSERT_FALSE(result.ok());
  EXPECT_EQ("Object invariant failed", result.error());
}

}  // namespace

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
