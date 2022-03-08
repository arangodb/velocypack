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
#include "velocypack/inspection/Inspector.h"
#include "velocypack/Iterator.h"
#include "velocypack/Slice.h"
#include "velocypack/Value.h"

namespace arangodb::velocypack::inspection {

struct LoadInspector : InspectorBase<LoadInspector> {
  static constexpr bool isLoading = true;

  explicit LoadInspector(Builder& builder) : _slice(builder.slice()) {}
  explicit LoadInspector(Slice slice) : _slice(slice) {}

  template<class T>
  requires std::is_integral_v<T>
  [[nodiscard]] Result value(T& v) {
    try {
      v = _slice.getNumber<T>();
      return {};
    } catch (Exception& e) {
      return {e.what()};
    }
  }

  [[nodiscard]] Result value(double& v) {
    try {
      v = _slice.getNumber<double>();
      return {};
    } catch (Exception& e) {
      return {e.what()};
    }
  }

  [[nodiscard]] Result value(std::string& v) {
    if (!_slice.isString()) {
      return {"Expecting type String"};
    }
    v = _slice.copyString();
    return {};
  }

  [[nodiscard]] Result value(bool& v) {
    if (!_slice.isBool()) {
      return {"Expecting type Bool"};
    }
    v = _slice.isTrue();
    return {};
  }

  [[nodiscard]] Result beginObject() {
    if (!_slice.isObject()) {
      return {"Expecting type Object"};
    }
    return {};
  }

  [[nodiscard]] Result endObject() { return {}; }

  [[nodiscard]] Result beginArray() {
    if (!_slice.isArray()) {
      return {"Expecting type Array"};
    }
    return {};
  }

  [[nodiscard]] Result endArray() { return {}; }

  template<class T>
  [[nodiscard]] Result list(T& list) {
    if (auto res = beginArray(); !res.ok()) {
      return res;
    }
    std::size_t idx = 0;
    for (auto&& s : VPackArrayIterator(_slice)) {
      LoadInspector ff(s);
      typename T::value_type val;
      if (auto res = process(ff, val); !res.ok()) {
        return {std::move(res), "[" + std::to_string(idx) + "]"};
      }
      list.push_back(std::move(val));
      ++idx;
    }
    return endArray();
  }

  template<class T>
  [[nodiscard]] Result map(T& map) {
    if (auto res = beginObject(); !res.ok()) {
      return res;
    }
    for (auto&& pair : VPackObjectIterator(_slice)) {
      LoadInspector ff(pair.value);
      typename T::mapped_type val;
      if (auto res = process(ff, val); !res.ok()) {
        return {std::move(res), "['" + pair.key.copyString() + "']"};
      }
      map.emplace(pair.key.copyString(), std::move(val));
    }
    return endObject();
  }

  template<class T>
  [[nodiscard]] Result tuple(T& data) {
    constexpr auto arrayLength = std::tuple_size_v<T>;
    if (auto res = beginArray(); !res.ok()) {
      return res;
    }
    if (_slice.length() != arrayLength) {
      return {"Expected array of length " + std::to_string(arrayLength)};
    }

    if (auto res = processTuple<0, arrayLength>(data); !res.ok()) {
      return res;
    }

    return endArray();
  }

  template<class T, size_t N>
  [[nodiscard]] Result tuple(T (&data)[N]) {
    if (auto res = beginArray(); !res.ok()) {
      return res;
    }
    if (_slice.length() != N) {
      return {"Expected array of length " + std::to_string(N)};
    }
    std::size_t index = 0;
    for (auto&& v : VPackArrayIterator(_slice)) {
      LoadInspector ff(v);
      if (auto res = process(ff, data[index]); !res.ok()) {
        return {std::move(res), "[" + std::to_string(index) + "]"};
        return res;
      }
      ++index;
    }
    return endArray();
  }

  template<class T>
  [[nodiscard]] Result applyField(T& field) {
    auto res = [&]() {
      if constexpr (!std::is_void_v<decltype(getFallbackValue(field))>) {
        return loadField(*this, getFieldName(field), getFieldValue(field),
                         getFallbackValue(field));
      } else {
        return loadField(*this, getFieldName(field), getFieldValue(field));
      }
    }();
    if (res.ok()) {
      res = checkInvariant(field);
    }

    if (!res.ok()) {
      return {std::move(res), getFieldName(field)};
    }
    return res;
  }

  Slice slice() noexcept { return _slice; }

  template<class U>
  struct FallbackContainer {
    explicit FallbackContainer(U&& val) : fallbackValue(std::move(val)) {}
    U fallbackValue;
  };

  template<class Invariant>
  struct InvariantContainer {
    explicit InvariantContainer(Invariant&& invariant)
        : invariantFunc(std::move(invariant)) {}
    Invariant invariantFunc;

    static constexpr const char InvariantFailedError[] =
        "Field invariant failed";
  };

 private:
  template<class T>
  struct HasFallback : std::false_type {};

  template<class T, class U>
  struct HasFallback<FallbackField<T, U>> : std::true_type {};

  template<class T>
  Result checkInvariant(T& field) {
    if constexpr (requires() { field.invariantFunc; }) {
      return InspectorBase::checkInvariant<T, T::InvariantFailedError>(
          field.invariantFunc, getFieldValue(field));
    } else if constexpr (requires() { field.inner; }) {
      return checkInvariant(field.inner);
    } else {
      return {};
    }
  }

  template<std::size_t Idx, std::size_t End, class T>
  [[nodiscard]] Result processTuple(T& data) {
    if constexpr (Idx < End) {
      LoadInspector ff{_slice[Idx]};
      if (auto res = process(ff, std::get<Idx>(data)); !res.ok()) {
        return {std::move(res), "[" + std::to_string(Idx) + "]"};
      }
      return processTuple<Idx + 1, End>(data);
    } else {
      return {};
    }
  }

  Slice _slice;
};

}  // namespace arangodb::velocypack::inspection
