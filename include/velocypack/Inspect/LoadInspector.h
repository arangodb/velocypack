////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2022 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
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
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <optional>
#include <string_view>
#include <tuple>
#include <type_traits>

#include "velocypack/Builder.h"
#include "velocypack/Inspect/InspectorAccess.h"
#include "velocypack/Iterator.h"
#include "velocypack/Slice.h"
#include "velocypack/Value.h"

namespace arangodb::velocypack {

struct LoadInspector {
  static constexpr bool isLoading = true;

  explicit LoadInspector(Builder &builder) : _slice(builder.slice()) {}
  explicit LoadInspector(Slice slice) : _slice(slice) {}

  template <class T> [[nodiscard]] bool apply(T &x) {
    return inspection::load(*this, x);
  }

  template <class T>
  requires std::is_integral_v<T>
  [[nodiscard]] bool value(T &v) {
    v = _slice.getNumber<T>();
    return true;
  }

  [[nodiscard]] bool value(double &v) {
    v = _slice.getDouble();
    return true;
  }

  [[nodiscard]] bool value(std::string &v) {
    v = _slice.copyString();
    return true;
  }

  [[nodiscard]] bool value(bool &v) {
    v = _slice.getBool();
    return true;
  }

  [[nodiscard]] bool beginObject() {
    assert(_slice.isObject());
    return true;
  }

  [[nodiscard]] bool endObject() { return true; }

  [[nodiscard]] bool beginArray() {
    assert(_slice.isArray());
    return true;
  }

  [[nodiscard]] bool endArray() { return true; }

  template <class T> [[nodiscard]] bool list(T &list) {
    if (!beginArray()) {
      return false;
    }
    for (auto &&s : VPackArrayIterator(_slice)) {
      LoadInspector ff(s);
      typename T::value_type val;
      if (!inspection::load(ff, val)) {
        return false;
      }
      list.push_back(std::move(val));
    }
    return endArray();
  }

  template <class T> [[nodiscard]] bool map(T &map) {
    if (!beginObject()) {
      return false;
    }
    for (auto &&pair : VPackObjectIterator(_slice)) {
      LoadInspector ff(pair.value);
      typename T::mapped_type val;
      if (!inspection::load(ff, val)) {
        return false;
      }
      map.emplace(pair.key.copyString(), std::move(val));
    }
    return endObject();
  }

  template <class T> [[nodiscard]] bool tuple(T &data) {
    assert(_slice.isArray());
    assert(_slice.length() == std::tuple_size_v<T>);
    return [ this, &data ]<std::size_t... Is>(std::index_sequence<Is...>) {
      return beginArray() &&
             (inspection::load(LoadInspector{_slice[Is]}, std::get<Is>(data)) &&
              ...) &&
             endArray();
    }
    (std::make_index_sequence<std::tuple_size<T>::value>{});
  }

  template <class T, size_t N> [[nodiscard]] bool tuple(T (&data)[N]) {
    if (!beginArray()) {
      return false;
    }
    assert(_slice.length() == N);
    std::size_t index = 0;
    for (auto &&v : VPackArrayIterator(_slice)) {
      LoadInspector ff(v);
      if (!inspection::load(ff, data[index])) {
        return false;
      }
      ++index;
    }
    return endArray();
  }

  struct Object {
    template <class... Args> [[nodiscard]] bool fields(Args... args) {
      return inspector.beginObject() && (args(inspector) && ...) &&
             inspector.endObject();
    }

    LoadInspector &inspector;
  };

  template <typename Derived> struct Field {
    std::string_view name;

    template <class Predicate>
    [[nodiscard]] auto invariant(Predicate predicate) &&;

    template <class U> [[nodiscard]] auto fallback(U val) &&;
  };

  template <typename T> struct RawField : Field<RawField<T>> {
    using value_type = T;
    T *value;
    [[nodiscard]] bool operator()(LoadInspector &f) {
      return inspection::loadField(f, this->name, *value);
    }
  };

  template <typename T> struct VirtualField : Field<VirtualField<T>> {
    using value_type = T;
  };

  template <class Field> struct Invariant {};

  [[nodiscard]] Object object() noexcept { return Object{*this}; }

  template <typename T>
  [[nodiscard]] RawField<T> field(std::string_view name,
                                  T &value) const noexcept {
    static_assert(!std::is_const<T>::value);
    return RawField<T>{{name}, std::addressof(value)};
  }

  Slice _slice;
};

} // namespace arangodb::velocypack
