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

#include <concepts>
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
  Result(Result&& res, std::string_view path) : _error(std::move(res._error)) {
    assert(!ok());
    if (_error->path.empty()) {
      _error->path = path;
    } else {
      _error->path = std::string(path) + "." + _error->path;
    }
  }
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
  struct Error {
    explicit Error(std::string&& msg) : message(std::move(msg)) {}
    std::string message;
    std::string path;
  };
  std::unique_ptr<Error> _error;
};

template<class T>
struct InspectorAccess;

template<class T, class Inspector>
concept HasInspectOverload = requires(Inspector f, T a) {
  { inspect(f, a) } -> std::convertible_to<Result>;
};

template<class T>
concept IsBuiltinType =
    std::same_as<T, bool> || std::integral<T> || std::floating_point<T> ||
    std::same_as<T, std::string>;  // TODO - use is-string-like

template<class T>
concept IsListLike = requires(T a) {
  a.begin() != a.end();
  ++a.begin();
  *a.begin();
  a.push_back(std::declval<typename T::value_type>());
};

template<class T>
concept IsMapLike = requires(T a) {
  a.begin() != a.end();
  ++a.begin();
  *a.begin();
  typename T::key_type;
  typename T::mapped_type;
  // TODO - check that T::key_type is string-like
  a.emplace(std::declval<std::string>(),
            std::declval<typename T::mapped_type>());
};

template<class T>
concept IsTuple = std::is_array_v<T> || requires(T a) {
  typename std::tuple_size<T>::type;
};

template<class T>
concept IsCompleteType = requires(T a) {
  sizeof(a);
};

template<class T>
concept HasInspectorAccessSpecialization = IsCompleteType<InspectorAccess<T>>;

template<class T, class Inspector>
concept IsInspectable =
    IsBuiltinType<T> || HasInspectOverload<T, Inspector> || IsListLike<T> ||
    IsMapLike<T> || IsTuple<T> || HasInspectorAccessSpecialization<T>;

template<class Inspector, class T>
[[nodiscard]] Result process(Inspector& f, T& x) {
  using TT = std::remove_cvref_t<T>;
  static_assert(IsInspectable<TT, Inspector>);
  if constexpr (HasInspectOverload<TT, Inspector>) {
    return static_cast<Result>(inspect(f, x));
  } else if constexpr (HasInspectorAccessSpecialization<TT>) {
    return InspectorAccess<T>::apply(f, x);
  } else if constexpr (IsBuiltinType<TT>) {
    return f.value(x);
  } else if constexpr (IsTuple<T>) {
    return f.tuple(x);
  } else if constexpr (IsMapLike<T>) {
    return f.map(x);
  } else if constexpr (IsListLike<T>) {
    return f.list(x);
  }
}

template<class Inspector, class T>
[[nodiscard]] Result process(Inspector& f, const T& x) {
  return process(f, const_cast<T&>(x));
}

template<class Inspector, class T>
[[nodiscard]] Result saveField(Inspector& f, std::string_view name, T& val) {
  if constexpr (HasInspectorAccessSpecialization<T>) {
    return InspectorAccess<T>::saveField(f, name, val);
  } else {
    f.builder().add(VPackValue(name));
    return f.apply(val);
  }
}

template<class Inspector, class T>
[[nodiscard]] Result loadField(Inspector& f, std::string_view name, T& val) {
  if constexpr (HasInspectorAccessSpecialization<T>) {
    return InspectorAccess<T>::loadField(f, name, val);
  } else {
    auto s = f.slice()[name];
    if (s.isNone()) {
      return {"Missing required attribute '" + std::string(name) + "'"};
    }
    Inspector ff(s);
    return ff.apply(val);
  }
}

template<class Inspector, class T, class U>
[[nodiscard]] Result loadField(Inspector& f, std::string_view name, T& val,
                               U& fallback) {
  if constexpr (HasInspectorAccessSpecialization<T>) {
    return InspectorAccess<T>::loadField(f, name, val, fallback);
  } else {
    auto s = f.slice()[name];
    if (s.isNone()) {
      val = T{std::move(fallback)};  // TODO - do we want to move?
      return {};
    }
    Inspector ff(s);
    return ff.apply(val);
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

  template<class Inspector>
  [[nodiscard]] static Result loadField(Inspector& f, std::string_view name,
                                        std::optional<T>& val) {
    auto s = f.slice()[name];
    Inspector ff(s);
    return ff.apply(val);
  }

  template<class Inspector, class U>
  [[nodiscard]] static Result loadField(Inspector& f, std::string_view name,
                                        std::optional<T>& val, U& fallback) {
    auto s = f.slice()[name];
    if (s.isNone()) {
      val = fallback;
      return {};
    }
    Inspector ff(s);
    return ff.apply(val);
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
  [[nodiscard]] static Result loadField(Inspector& f, std::string_view name,
                                        T& val) {
    auto s = f.slice()[name];
    if (s.isNone() || s.isNull()) {
      val.reset();
      return {};
    }
    val = Derived::make();  // TODO - reuse existing object?
    Inspector ff(s);
    return ff.apply(val);
  }

  template<class Inspector, class U>
  [[nodiscard]] static Result loadField(Inspector& f, std::string_view name,
                                        T& val, U& fallback) {
    auto s = f.slice()[name];
    if (s.isNone()) {
      val = fallback;
      return {};
    } else if (s.isNull()) {
      val.reset();
      return {};
    }
    val = Derived::make();  // TODO - reuse existing object?
    Inspector ff(s);
    return ff.apply(val);
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
