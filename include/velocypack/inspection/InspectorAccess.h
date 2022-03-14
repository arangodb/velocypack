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

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "velocypack/Value.h"

namespace arangodb::velocypack::inspection {

struct Result {
  Result() = default;

  Result(std::string error)
      : _error(std::make_unique<Error>(std::move(error))) {}

  bool ok() const noexcept { return _error == nullptr; }

  std::string const& error() const noexcept {
    assert(!ok());
    return _error->message;
  }

  std::string const& path() const noexcept {
    assert(!ok());
    return _error->path;
  }

 private:
  friend struct LoadInspector;
  friend struct SaveInspector;

  struct AttributeTag {};
  struct ArrayTag {};

  Result(Result&& res, std::string const& index, ArrayTag)
      : Result(std::move(res)) {
    prependPath("[" + index + "]");
  }

  Result(Result&& res, std::string_view attribute, AttributeTag)
      : Result(std::move(res)) {
    if (attribute.find('.') != std::string::npos) {
      prependPath("['" + std::string(attribute) + "']");
    } else {
      prependPath(std::string(attribute));
    }
  }

  void prependPath(std::string const& s) {
    assert(!ok());
    if (_error->path.empty()) {
      _error->path = s;
    } else {
      if (_error->path[0] != '[') {
        _error->path = "." + _error->path;
      }
      _error->path = s + _error->path;
    }
  }

  struct Error {
    explicit Error(std::string&& msg) : message(std::move(msg)) {}
    std::string message;
    std::string path;
  };
  std::unique_ptr<Error> _error;
};

template<class T>
struct InspectorAccess;

template<class T, class Inspector, class = void>
struct HasInspectOverload : std::false_type {};

template<class T, class Inspector>
struct HasInspectOverload<T, Inspector,
                          std::void_t<decltype(inspect(
                              std::declval<Inspector&>(), std::declval<T&>()))>>
    : std::conditional_t<
          std::is_convertible_v<decltype(inspect(std::declval<Inspector&>(),
                                                 std::declval<T&>())),
                                Result>,
          std::true_type, typename std::false_type> {};

template<class T>
constexpr inline bool IsBuiltinType() {
  return std::is_same_v<T, bool> || std::is_integral_v<T> ||
         std::is_floating_point_v<T> ||
         std::is_same_v<T, std::string>;  // TODO - use is-string-like?
}

template<class T, class = void>
struct IsListLike : std::false_type {};

template<class T>
struct IsListLike<T, std::void_t<decltype(std::declval<T>().begin() !=
                                          std::declval<T>().end()),
                                 decltype(++std::declval<T>().begin()),
                                 decltype(*std::declval<T>().begin()),
                                 decltype(std::declval<T>().push_back(
                                     std::declval<typename T::value_type>()))>>
    : std::true_type {};

template<class T, class = void>
struct IsMapLike : std::false_type {};

template<class T>
struct IsMapLike<T, std::void_t<decltype(std::declval<T>().begin() !=
                                         std::declval<T>().end()),
                                decltype(++std::declval<T>().begin()),
                                decltype(*std::declval<T>().begin()),
                                typename T::key_type, typename T::mapped_type,
                                decltype(std::declval<T>().emplace(
                                    std::declval<std::string>(),
                                    std::declval<typename T::mapped_type>()))>>
    : std::true_type {};

template<class T, class = void>
struct IsTuple
    : std::conditional_t<std::is_array_v<T>, std::true_type, std::false_type> {
};

template<class T>
struct IsTuple<T, std::void_t<typename std::tuple_size<T>::type>>
    : std::true_type {};

template<class T, std::size_t = sizeof(T)>
std::true_type IsCompleteTypeImpl(T*);

std::false_type IsCompleteTypeImpl(...);

template<class T>
constexpr inline bool IsCompleteType() {
  return decltype(IsCompleteTypeImpl(std::declval<T*>()))::value;
}

template<class T>
constexpr inline bool HasInspectorAccessSpecialization() {
  return IsCompleteType<InspectorAccess<T>>();
}

template<class T, class Inspector>
constexpr inline bool IsInspectable() {
  return IsBuiltinType<T>() || HasInspectOverload<T, Inspector>::value ||
         IsListLike<T>::value || IsMapLike<T>::value || IsTuple<T>::value ||
         HasInspectorAccessSpecialization<T>();
}

template<class Inspector, class T>
[[nodiscard]] Result process(Inspector& f, T& x) {
  using TT = std::remove_cvref_t<T>;
  static_assert(IsInspectable<TT, Inspector>());
  if constexpr (HasInspectOverload<TT, Inspector>::value) {
    return static_cast<Result>(inspect(f, x));
  } else if constexpr (HasInspectorAccessSpecialization<TT>()) {
    return InspectorAccess<T>::apply(f, x);
  } else if constexpr (IsBuiltinType<TT>()) {
    return f.value(x);
  } else if constexpr (IsTuple<TT>::value) {
    return f.tuple(x);
  } else if constexpr (IsMapLike<TT>::value) {
    return f.map(x);
  } else if constexpr (IsListLike<TT>::value) {
    return f.list(x);
  }
}

template<class Inspector, class T>
[[nodiscard]] Result process(Inspector& f, T const& x) {
  static_assert(!Inspector::isLoading);
  return process(f, const_cast<T&>(x));
}

template<class Inspector, class T>
[[nodiscard]] Result saveField(Inspector& f, std::string_view name, T& val) {
  if constexpr (HasInspectorAccessSpecialization<T>()) {
    return InspectorAccess<T>::saveField(f, name, val);
  } else {
    f.builder().add(VPackValue(name));
    return f.apply(val);
  }
}

template<class Inspector, class T, class Transformer>
[[nodiscard]] Result saveTransformedField(Inspector& f, std::string_view name,
                                          T& val, Transformer& transformer) {
  if constexpr (HasInspectorAccessSpecialization<T>()) {
    return InspectorAccess<T>::saveTransformedField(f, name, val, transformer);
  } else {
    typename Transformer::SerializedType v;
    if (auto res = transformer.toSerialized(val, v); !res.ok()) {
      return res;
    }
    f.builder().add(VPackValue(name));
    return f.apply(v);
  }
}

template<class Inspector, class T>
[[nodiscard]] Result loadField(Inspector& f, std::string_view name, T& val) {
  if constexpr (HasInspectorAccessSpecialization<T>()) {
    return InspectorAccess<T>::loadField(f, name, val);
  } else {
    auto s = f.slice();
    if (s.isNone()) {
      return {"Missing required attribute '" + std::string(name) + "'"};
    }
    return f.apply(val);
  }
}

template<class Inspector, class Value, class Fallback>
[[nodiscard]] Result loadField(Inspector& f,
                               [[maybe_unused]] std::string_view name,
                               Value& val, Fallback&& fallback) {
  if constexpr (HasInspectorAccessSpecialization<Value>()) {
    return InspectorAccess<Value>::loadField(f, name, val, fallback);
  } else {
    auto s = f.slice();
    if (s.isNone()) {
      if constexpr (std::is_assignable_v<Value, Fallback>) {
        val = std::forward<Fallback>(fallback);
      } else {
        val = Value{std::forward<Fallback>(fallback)};
      }
      return {};
    }
    return f.apply(val);
  }
}

template<class Inspector, class Value, class Transformer>
[[nodiscard]] Result loadTransformedField(Inspector& f, std::string_view name,
                                          Value& val,
                                          Transformer& transformer) {
  if constexpr (HasInspectorAccessSpecialization<Value>()) {
    return InspectorAccess<Value>::loadTransformedField(f, name, val,
                                                        transformer);
  } else {
    auto s = f.slice();
    if (s.isNone()) {
      return {"Missing required attribute '" + std::string(name) + "'"};
    }
    typename Transformer::SerializedType v;
    if (auto res = f.apply(v); !res.ok()) {
      return res;
    }
    return transformer.fromSerialized(v, val);
  }
}

template<class Inspector, class Value, class Fallback, class Transformer>
[[nodiscard]] Result loadTransformedField(
    Inspector& f, [[maybe_unused]] std::string_view name, Value& val,
    Fallback&& fallback, Transformer& transformer) {
  if constexpr (HasInspectorAccessSpecialization<Value>()) {
    return InspectorAccess<Value>::loadTransformedField(f, name, val, fallback,
                                                        transformer);
  } else {
    auto s = f.slice();
    if (s.isNone()) {
      if constexpr (std::is_assignable_v<Value, Fallback>) {
        val = std::forward<Fallback>(fallback);
      } else {
        val = Value{std::forward<Fallback>(fallback)};
      }
      return {};
    }
    typename Transformer::SerializedType v;
    if (auto res = f.apply(v); !res.ok()) {
      return res;
    }
    return transformer.fromSerialized(v, val);
  }
}
template<class T>
struct InspectorAccess<std::optional<T>> {
  template<class Inspector>
  [[nodiscard]] static Result apply(Inspector& f, std::optional<T>& val) {
    if constexpr (Inspector::isLoading) {
      if (f.slice().isNone() || f.slice().isNull()) {
        val.reset();
        return {};
      } else {
        T v;
        auto res = f.apply(v);
        if (res.ok()) {
          val = std::move(v);
          return {};
        }
        return res;
      }

    } else {
      if (val.has_value()) {
        return f.apply(val.value());
      }
      f.builder().add(VPackValue(ValueType::Null));
      return {};
    }
  }

  template<class Inspector>
  [[nodiscard]] static Result saveField(Inspector& f, std::string_view name,
                                        std::optional<T>& val) {
    if (!val.has_value()) {
      return {};
    }
    return inspection::saveField(f, name, val.value());
  }

  template<class Inspector, class Transformer>
  [[nodiscard]] static Result saveTransformedField(Inspector& f,
                                                   std::string_view name,
                                                   std::optional<T>& val,
                                                   Transformer& transformer) {
    if (!val.has_value()) {
      return {};
    }
    typename Transformer::SerializedType v;
    if (auto res = transformer.toSerialized(*val, v); !res.ok()) {
      return res;
    }
    return inspection::saveField(f, name, v);
  }

  template<class Inspector>
  [[nodiscard]] static Result loadField(Inspector& f,
                                        [[maybe_unused]] std::string_view name,
                                        std::optional<T>& val) {
    return f.apply(val);
  }

  template<class Inspector, class U>
  [[nodiscard]] static Result loadField(Inspector& f,
                                        [[maybe_unused]] std::string_view name,
                                        std::optional<T>& val, U& fallback) {
    auto s = f.slice();
    if (s.isNone()) {
      val = fallback;
      return {};
    }
    return f.apply(val);
  }

  template<class Inspector, class Transformer>
  [[nodiscard]] static Result loadTransformedField(
      Inspector& f, [[maybe_unused]] std::string_view name,
      std::optional<T>& val, Transformer& transformer) {
    std::optional<typename Transformer::SerializedType> v;
    if (auto res = f.apply(v); !res.ok()) {
      return res;
    }
    if (!v.has_value()) {
      val.reset();
      return {};
    }
    T vv;
    auto res = transformer.fromSerialized(*v, vv);
    val = vv;
    return res;
  }

  template<class Inspector, class Fallback, class Transformer>
  [[nodiscard]] static Result loadTransformedField(
      Inspector& f, [[maybe_unused]] std::string_view name,
      std::optional<T>& val, Fallback&& fallback, Transformer& transformer) {
    auto s = f.slice();
    if (s.isNone()) {
      if constexpr (std::is_assignable_v<std::optional<T>, Fallback>) {
        val = std::forward<Fallback>(fallback);
      } else {
        val = std::optional<T>{std::forward<Fallback>(fallback)};
      }
      return {};
    }
    return loadTransformedField(f, name, val, transformer);
  }
};

template<class Derived, class T>
struct PointerInspectorAccess {
  template<class Inspector>
  [[nodiscard]] static Result apply(Inspector& f, T& val) {
    if constexpr (Inspector::isLoading) {
      if (f.slice().isNone() || f.slice().isNull()) {
        val.reset();
        return {};
      }
      val = Derived::make();  // TODO - reuse existing object?
      return f.apply(*val);
    } else {
      if (val != nullptr) {
        return f.apply(*val);
      }
      f.builder().add(VPackValue(ValueType::Null));
      return {};
    }
  }

  template<class Inspector>
  [[nodiscard]] static Result saveField(Inspector& f, std::string_view name,
                                        T& val) {
    if (val == nullptr) {
      return {};
    }
    return inspection::saveField(f, name, *val);
  }

  template<class Inspector>
  [[nodiscard]] static Result loadField(Inspector& f,
                                        [[maybe_unused]] std::string_view name,
                                        T& val) {
    auto s = f.slice();
    if (s.isNone() || s.isNull()) {
      val.reset();
      return {};
    }
    val = Derived::make();  // TODO - reuse existing object?
    return f.apply(val);
  }

  template<class Inspector, class U>
  [[nodiscard]] static Result loadField(Inspector& f,
                                        [[maybe_unused]] std::string_view name,
                                        T& val, U& fallback) {
    auto s = f.slice();
    if (s.isNone()) {
      val = fallback;
      return {};
    } else if (s.isNull()) {
      val.reset();
      return {};
    }
    val = Derived::make();  // TODO - reuse existing object?
    return f.apply(val);
  }
};

template<class T, class Deleter>
struct InspectorAccess<std::unique_ptr<T, Deleter>>
    : PointerInspectorAccess<InspectorAccess<std::unique_ptr<T, Deleter>>,
                             std::unique_ptr<T, Deleter>> {
  static auto make() { return std::make_unique<T>(); }
};

template<class T>
struct InspectorAccess<std::shared_ptr<T>>
    : PointerInspectorAccess<InspectorAccess<std::shared_ptr<T>>,
                             std::shared_ptr<T>> {
  static auto make() { return std::make_shared<T>(); }
};

}  // namespace arangodb::velocypack::inspection
