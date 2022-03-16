
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
#include <type_traits>
#include <variant>

#include "velocypack/Builder.h"
#include "velocypack/inspection/Inspector.h"
#include "velocypack/Slice.h"
#include "velocypack/Value.h"

namespace arangodb::velocypack::inspection {

struct SaveInspector : InspectorBase<SaveInspector> {
  static constexpr bool isLoading = false;

  explicit SaveInspector(Builder& builder) : _builder(builder) {}

  [[nodiscard]] Result::Success beginObject() {
    _builder.openObject();
    return {};
  }

  [[nodiscard]] Result::Success endObject() {
    _builder.close();
    return {};
  }

  template<class T, class = std::enable_if_t<IsBuiltinType<T>()>>
  [[nodiscard]] Result::Success value(T const& v) {
    static_assert(IsBuiltinType<T>());
    _builder.add(VPackValue(v));
    return {};
  }

  [[nodiscard]] Result::Success beginArray() {
    _builder.openArray();
    return {};
  }

  [[nodiscard]] Result::Success endArray() {
    _builder.close();
    return {};
  }

  template<class T>
  [[nodiscard]] Result tuple(T const& data) {
    return beginArray()                                                   //
         | [&]() { return processTuple<0, std::tuple_size_v<T>>(data); }  //
         | [&]() { return endArray(); };                                  //
  }

  template<class T, size_t N>
  [[nodiscard]] Result tuple(T (&data)[N]) {
    return beginArray()                                                     //
         | [&]() { return processList(std::begin(data), std::end(data)); }  //
         | [&]() { return endArray(); };                                    //
  }

  template<class T>
  [[nodiscard]] Result list(T const& list) {
    return beginArray()                                                     //
         | [&]() { return processList(std::begin(list), std::end(list)); }  //
         | [&]() { return endArray(); };                                    //
  }

  template<class T>
  [[nodiscard]] Result map(T const& map) {
    return beginObject()                      //
         | [&]() { return processMap(map); }  //
         | [&]() { return endObject(); };     //
  }

  template<class Arg, class... Args>
  Result applyFields(Arg&& arg, Args&&... args) {
    auto res = self().applyField(std::forward<Arg>(arg));
    if constexpr (sizeof...(args) == 0) {
      return res;
    } else {
      return std::move(res)                                               //
           | [&]() { return applyFields(std::forward<Args>(args)...); };  //
    }
  }

  Builder& builder() noexcept { return _builder; }

  template<class U>
  struct FallbackContainer {
    explicit FallbackContainer(U&&) {}
  };
  template<class T>
  struct InvariantContainer {
    explicit InvariantContainer(T&&) {}
  };

 private:
  template<class T>
  [[nodiscard]] Result applyField(T const& field) {
    auto name = getFieldName(field);
    auto& value = getFieldValue(field);
    auto res = [&]() {
      if constexpr (!std::is_void_v<decltype(getTransformer(field))>) {
        return saveTransformedField(*this, name, value, getTransformer(field));
      } else {
        return saveField(*this, name, value);
      }
    }();
    if (!res.ok()) {
      return {std::move(res), name, Result::AttributeTag{}};
    }
    return res;
  }

  template<std::size_t Idx, std::size_t End, class T>
  [[nodiscard]] auto processTuple(T const& data) {
    if constexpr (Idx < End) {
      return process(*this, std::get<Idx>(data))                  //
           | [&]() { return processTuple<Idx + 1, End>(data); };  //
    } else {
      return Result::Success{};
    }
  }

  template<class Iterator>
  [[nodiscard]] Result processList(Iterator begin, Iterator end) {
    for (auto it = begin; it != end; ++it) {
      if (auto res = process(*this, *it); !res.ok()) {
        return res;
      }
    }
    return {};
  }
  template<class T>
  [[nodiscard]] Result processMap(T const& map) {
    for (auto&& [k, v] : map) {
      _builder.add(VPackValue(k));
      if (auto res = process(*this, v); !res.ok()) {
        return res;
      }
    }
    return {};
  }

  Builder& _builder;
};

}  // namespace arangodb::velocypack::inspection
