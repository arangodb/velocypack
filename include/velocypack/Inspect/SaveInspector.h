
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
#include <variant>

#include "velocypack/Builder.h"
#include "velocypack/Inspect/InspectorAccess.h"
#include "velocypack/Slice.h"
#include "velocypack/Value.h"

namespace arangodb::velocypack {

struct SaveInspector {
  static constexpr bool isLoading = false;

  explicit SaveInspector(Builder &builder) : _builder(builder) {}

  [[nodiscard]] bool beginObject() {
    _builder.openObject();
    return true;
  }

  [[nodiscard]] bool endObject() {
    _builder.close();
    return true;
  }

  template <class T>
  [[nodiscard]] bool value(T const &v) requires inspection::IsBuiltinType<T> {
    _builder.add(VPackValue(v));
    return true;
  }

  [[nodiscard]] bool beginArray() {
    _builder.openArray();
    return true;
  }

  [[nodiscard]] bool endArray() {
    _builder.close();
    return true;
  }

  template <class T> [[nodiscard]] bool tuple(const T &data) {
    return [ this, &data ]<std::size_t... Is>(std::index_sequence<Is...>) {
      return beginArray() &&
             (inspection::save(*this, std::get<Is>(data)) && ...) && endArray();
    }
    (std::make_index_sequence<std::tuple_size<T>::value>{});
  }

  template <class T, size_t N> [[nodiscard]] bool tuple(T (&data)[N]) {
    if (!beginArray()) {
      return false;
    }
    for (size_t index = 0; index < N; ++index) {
      if (!inspection::save(*this, data[index])) {
        return false;
      }
    }
    return endArray();
  }

  template <class T> [[nodiscard]] bool list(const T &list) {
    if (!beginArray()) {
      return false;
    }
    for (auto &&val : list) {
      if (!inspection::save(*this, val)) {
        return false;
      }
    }
    return endArray();
  }

  template <class T> [[nodiscard]] bool map(const T &map) {
    if (!beginObject()) {
      return false;
    }
    for (auto &&[k, v] : map) {
      _builder.add(VPackValue(k));
      if (!inspection::save(*this, v)) {
        return false;
      }
    }
    return endObject();
  }

  template <class T> [[nodiscard]] bool apply(const T &x) {
    return inspection::save(*this, x);
  }

  struct Object {
    template <class... Args> [[nodiscard]] bool fields(Args... args) {
      return inspector.beginObject() && (args(inspector) && ...) &&
             inspector.endObject();
    }

    SaveInspector &inspector;
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
    [[nodiscard]] bool operator()(SaveInspector &f) {
      return inspection::saveField(f, this->name, *value);
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

  Builder &_builder;
};

} // namespace arangodb::velocypack
