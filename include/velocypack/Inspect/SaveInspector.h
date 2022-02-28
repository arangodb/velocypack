
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
#include "velocypack/Inspect/Inspector.h"
#include "velocypack/Inspect/InspectorAccess.h"
#include "velocypack/Slice.h"
#include "velocypack/Value.h"

namespace arangodb::velocypack {

struct SaveInspector {
  static constexpr bool isLoading = false;

  explicit SaveInspector(Builder &builder) : _builder(builder) {}

  [[nodiscard]] inspection::Result beginObject() {
    _builder.openObject();
    return {};
  }

  [[nodiscard]] inspection::Result endObject() {
    _builder.close();
    return {};
  }

  template <class T>
  [[nodiscard]] inspection::Result
  value(T const &v) requires inspection::IsBuiltinType<T> {
    _builder.add(VPackValue(v));
    return {};
  }

  [[nodiscard]] inspection::Result beginArray() {
    _builder.openArray();
    return {};
  }

  [[nodiscard]] inspection::Result endArray() {
    _builder.close();
    return {};
  }

  template <class T> [[nodiscard]] inspection::Result tuple(const T &data) {
    auto res =  beginArray();
    assert(res.ok());

    if (auto res = processTuple<0, std::tuple_size_v<T>>(data); !res.ok()) {
      return res;
    }

    return endArray();
  }

  template <std::size_t Idx, std::size_t End, class T> [[nodiscard]] inspection::Result processTuple(const T &data) {
    if constexpr(Idx < End) {
      if (auto res = inspection::save(*this, std::get<Idx>(data)); !res.ok()) {
        return res;
      }
      return processTuple<Idx + 1, End>(data);
    } else {
      return {};
    }
  }

  template <class T, size_t N>
  [[nodiscard]] inspection::Result tuple(T (&data)[N]) {
    auto res = beginArray();
    assert(res.ok());
    for (size_t index = 0; index < N; ++index) {
      if (auto res = inspection::save(*this, data[index]); !res.ok()) {
        return res;
      }
    }
    return endArray();
  }

  template <class T> [[nodiscard]] inspection::Result list(const T &list) {
    auto res = beginArray();
    assert(res.ok());
    for (auto &&val : list) {
      if (auto res = inspection::save(*this, val); !res.ok()) {
        return res;
      }
    }
    return endArray();
  }

  template <class T> [[nodiscard]] inspection::Result map(const T &map) {
    auto res = beginObject();
    assert(res.ok());
    for (auto &&[k, v] : map) {
      _builder.add(VPackValue(k));
      if (auto res = inspection::save(*this, v); !res.ok()) {
        return res;
      }
    }
    return endObject();
  }

  template <class T> [[nodiscard]] inspection::Result apply(const T &x) {
    return inspection::save(*this, x);
  }

  struct Object {
    template <class... Args>
    [[nodiscard]] inspection::Result fields(Args... args) {
      auto res = inspector.beginObject();
      assert(res.ok());
      
      if (auto res = applyFields(std::forward<Args>(args)...); !res.ok()) {
        return res;
      }

      return inspector.endObject();
    }

   private:
    friend struct SaveInspector;
    explicit Object(SaveInspector& inspector) : inspector(inspector) {}

    template <class Arg>
    inspection::Result applyFields(Arg arg) {
      return arg(inspector);
    }

    template <class Arg, class... Args>
    inspection::Result applyFields(Arg arg, Args... args) {
      if (auto res = arg(inspector); !res.ok()) {
        return res;
      }
      return applyFields(std::forward<Args>(args)...);
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
    [[nodiscard]] inspection::Result operator()(SaveInspector &f) {
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
