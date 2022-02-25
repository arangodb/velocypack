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
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

#include "velocypack/Value.h"

namespace arangodb::velocypack::inspection {

struct AccessType {
  struct Builtin {};
  struct Inspect {};
  struct Specialization {};
  struct Tuple {};
  struct List {};
  struct Map {};
};

template<class T>
struct InspectorAccess;

template<class T, class Inspector>
concept HasInspectOverload = requires(Inspector f, T a) {
  { inspect(f, a) } -> std::same_as<bool>;
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
constexpr auto inspectAccessType() {
  using TT = std::remove_cvref_t<T>;
  static_assert(IsInspectable<TT, Inspector>);
  if constexpr (HasInspectOverload<TT, Inspector>) {
    return AccessType::Inspect{};
  } else if constexpr (HasInspectorAccessSpecialization<TT>) {
    return AccessType::Specialization{};
  } else if constexpr (IsBuiltinType<TT>) {
    return AccessType::Builtin{};
  } else if constexpr (IsTuple<T>) {
    return AccessType::Tuple{};
  } else if constexpr (IsMapLike<T>) {
    return AccessType::Map{};
  } else if constexpr (IsListLike<T>) {
    return AccessType::List{};
  }
}

// TODO - use concepts instead of tag type overloads

template<class Inspector, class T>
[[nodiscard]] bool save(Inspector& f, T& x, AccessType::Builtin) {
  return f.value(x);
}

template<class Inspector, class T>
[[nodiscard]] bool save(Inspector& f, T& x, AccessType::Inspect) {
  return inspect(f, x);
}

template<class Inspector, class T>
[[nodiscard]] bool save(Inspector& f, T& x, AccessType::Tuple) {
  return f.tuple(x);
}

template<class Inspector, class T>
[[nodiscard]] bool save(Inspector& f, T& x, AccessType::List) {
  return f.list(x);
}

template<class Inspector, class T>
bool save(Inspector& f, T& x, AccessType::Map) {
  return f.map(x);
}

template<class Inspector, class T>
[[nodiscard]] bool save(Inspector& f, T& x, AccessType::Specialization) {
  return InspectorAccess<T>::apply(f, x);
}

template<class Inspector, class T>
[[nodiscard]] bool save(Inspector& f, T& x) {
  return save(f, x, inspectAccessType<Inspector, T>());
}

template<class Inspector, class T>
[[nodiscard]] bool save(Inspector& f, const T& x) {
  return save(f, const_cast<T&>(x), inspectAccessType<Inspector, T>());
}

template<class Inspector, class T>
[[nodiscard]] bool saveField(Inspector& f, std::string_view name, T& val) {
  if constexpr (HasInspectorAccessSpecialization<T>) {
    return InspectorAccess<T>::saveField(f, name, val);
  } else {
    f._builder.add(VPackValue(name));
    return f.apply(val);
  }
}

template<class Inspector, class T>
[[nodiscard]] bool load(Inspector& f, T& x, AccessType::Inspect) {
  return inspect(f, x);
}

template<class Inspector, class T>
[[nodiscard]] bool load(Inspector& f, T& x, AccessType::Builtin) {
  return f.value(x);
}

template<class Inspector, class T>
[[nodiscard]] bool load(Inspector& f, T& x, AccessType::List) {
  return f.list(x);
}

template<class Inspector, class T>
[[nodiscard]] bool load(Inspector& f, T& x, AccessType::Map) {
  return f.map(x);
}

template<class Inspector, class T>
[[nodiscard]] bool load(Inspector& f, T& x, AccessType::Tuple) {
  return f.tuple(x);
}

template<class Inspector, class T>
[[nodiscard]] bool load(Inspector& f, T& x, AccessType::Specialization) {
  return InspectorAccess<T>::apply(f, x);
}

template<class Inspector, class T>
[[nodiscard]] bool load(Inspector const& f, T& x) {
  return load(const_cast<Inspector&>(f), x, inspectAccessType<Inspector, T>());
}

template<class Inspector, class T>
[[nodiscard]] bool loadField(Inspector& f, std::string_view name, T& val) {
  auto s = f._slice[name];
  Inspector ff(s);
  return ff.apply(val);
}

template<class T>
struct InspectorAccess<std::optional<T>> {
  template<class Inspector>
  [[nodiscard]] static bool apply(Inspector& f, std::optional<T>& val) {
    if constexpr (Inspector::isLoading) {
      if (f._slice.isNone() || f._slice.isNull()) {
        val.reset();
        return true;
      } else {
        T v;
        if (f.apply(v)) {
          val = std::move(v);
          return true;
        }
        return false;
      }

    } else {
      if (val.has_value()) {
        return f.apply(val.value());
      }
      f._builder.add(VPackValue(ValueType::Null));
      return true;
    }
  }

  template<class Inspector>
  [[nodiscard]] static bool saveField(Inspector& f, std::string_view name,
                                      std::optional<T>& val) {
    return !val.has_value() || inspection::saveField(f, name, val.value());
  }
};

template<class T>
struct PointerInspectorAccess {
  template<class Inspector>
  [[nodiscard]] static bool apply(Inspector& f, T& val) {
    if (val != nullptr) {
      return f.apply(*val);
    }
    f._builder.add(VPackValue(ValueType::Null));
    return true;
  }

  template<class Inspector>
  [[nodiscard]] static bool saveField(Inspector& f, std::string_view name,
                                      T& val) {
    return (val == nullptr) || inspection::saveField(f, name, *val);
  }
};

template<class T, class Deleter>
struct InspectorAccess<std::unique_ptr<T, Deleter>>
    : PointerInspectorAccess<std::unique_ptr<T, Deleter>> {};

template<class T>
struct InspectorAccess<std::shared_ptr<T>>
    : PointerInspectorAccess<std::shared_ptr<T>> {};

}  // namespace arangodb::velocypack::inspection