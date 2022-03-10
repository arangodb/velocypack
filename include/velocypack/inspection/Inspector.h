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

#include <array>
#include <functional>
#include <type_traits>
#include <variant>
#include "velocypack/Iterator.h"
#include "velocypack/inspection/InspectorAccess.h"

namespace arangodb::velocypack::inspection {

template<class Derived>
struct InspectorBase {
  Derived& self() { return static_cast<Derived&>(*this); }

  template<class T>
  [[nodiscard]] Result apply(T& x) {
    return process(self(), x);
  }

  template<class T>
  struct Object;

  template<class T, const char ErrorMsg[], class Func, class... Args>
  static Result checkInvariant(Func&& func, Args&&... args) {
    using result_t = std::invoke_result_t<Func, Args...>;
    if constexpr (std::is_same_v<result_t, bool>) {
      if (!std::invoke(std::forward<Func>(func), std::forward<Args>(args)...)) {
        return {ErrorMsg};
      }
      return {};
    } else {
      static_assert(std::is_same_v<result_t, Result>,
                    "Invariants must either return bool or "
                    "velocypack::inspection::Result");
      return std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
    }
  }

  template<class T>
  struct FieldsResult {
    template<class Invariant>
    Result invariant(Invariant&& func) {
      if constexpr (Derived::isLoading) {
        if (!result.ok()) {
          return std::move(result);
        }
        return checkInvariant<FieldsResult, FieldsResult::InvariantFailedError>(
            std::forward<Invariant>(func), object);
      } else {
        return std::move(result);
      }
    }
    operator Result() && { return std::move(result); }

    static constexpr const char InvariantFailedError[] =
        "Object invariant failed";

   private:
    template<class TT>
    friend struct Object;
    FieldsResult(Result&& res, T& object)
        : result(std::move(res)), object(object) {}
    Result result;
    T& object;
  };

  template<class T>
  struct Object {
    template<class... Args>
    [[nodiscard]] FieldsResult<T> fields(Args... args) {
      if (auto res = inspector.beginObject(); !res.ok()) {
        return {std::move(res), object};
      }

      if (auto res = inspector.applyFields(std::forward<Args>(args)...);
          !res.ok()) {
        return {std::move(res), object};
      }

      return {inspector.endObject(), object};
    }

   private:
    friend struct InspectorBase;
    explicit Object(Derived& inspector, T& o)
        : inspector(inspector), object(o) {}

    Derived& inspector;
    T& object;
  };

  template<class Field>
  struct InvariantMixin {
    template<class Predicate>
    [[nodiscard]] auto invariant(Predicate predicate) && {
      return InvariantField<Field, Predicate>(
          std::move(static_cast<Field&>(*this)), std::move(predicate));
    }
  };

  template<class Field>
  static constexpr bool HasInvariantMethod = requires(Field f) {
    f.invariant([](auto) { return true; });
  };

  template<class Inner>
  using WithInvariant =
      std::conditional_t<HasInvariantMethod<Inner>, std::monostate,
                         InvariantMixin<Inner>>;

  template<class Field>
  struct FallbackMixin {
    template<class U>
    [[nodiscard]] auto fallback(U&& val) && {
      static_assert(std::is_constructible_v<typename Field::value_type, U>);

      return FallbackField<Field, U>(std::move(static_cast<Field&>(*this)),
                                     std::forward<U>(val));
    }
  };

  template<class Field>
  static constexpr bool HasFallbackMethod = requires(Field f) {
    f.fallback(std::declval<typename Field::value_type>());
  };

  template<class Inner>
  using WithFallback = std::conditional_t<HasFallbackMethod<Inner>,
                                          std::monostate, FallbackMixin<Inner>>;

  template<class Field>
  struct TransformMixin {
    template<class T>
    [[nodiscard]] auto transformWith(T transformer) && {
      return TransformField<Field, T>(std::move(static_cast<Field&>(*this)),
                                      std::move(transformer));
    }
  };

  template<class Field>
  static constexpr bool HasTransformMethod = requires(Field f) {
    f.transform;
  };

  template<class Inner>
  using WithTransform =
      std::conditional_t<HasTransformMethod<Inner>, std::monostate,
                         TransformMixin<Inner>>;

  template<class InnerField, class Invariant>
  struct InvariantField : Derived::template InvariantContainer<Invariant>,
                          WithFallback<InvariantField<InnerField, Invariant>>,
                          WithTransform<InvariantField<InnerField, Invariant>> {
    InvariantField(InnerField inner, Invariant&& invariant)
        : Derived::template InvariantContainer<Invariant>(std::move(invariant)),
          inner(std::move(inner)) {}
    using value_type = typename InnerField::value_type;
    InnerField inner;
  };

  template<class InnerField, class U>
  struct FallbackField : Derived::template FallbackContainer<U>,
                         WithInvariant<FallbackField<InnerField, U>>,
                         WithTransform<FallbackField<InnerField, U>> {
    FallbackField(InnerField inner, U&& val)
        : Derived::template FallbackContainer<U>(std::move(val)),
          inner(std::move(inner)) {}
    using value_type = typename InnerField::value_type;
    InnerField inner;
  };

  template<class InnerField, class T>
  struct TransformField : WithInvariant<TransformField<InnerField, T>>,
                          WithFallback<TransformField<InnerField, T>> {
    TransformField(InnerField inner, T&& transformer)
        : inner(std::move(inner)), transformer(std::move(transformer)) {}
    using value_type = typename InnerField::value_type;
    InnerField inner;
    T transformer;
  };

  template<typename DerivedField>
  struct BasicField : InvariantMixin<DerivedField>,
                      FallbackMixin<DerivedField>,
                      TransformMixin<DerivedField> {
    explicit BasicField(std::string_view name) : name(name) {}
    std::string_view name;
  };

  template<typename T>
  struct RawField : BasicField<RawField<T>> {
    RawField(std::string_view name, T& value)
        : BasicField<RawField>(name), value(value) {}
    using value_type = T;
    T& value;
  };

  template<typename T>
  struct VirtualField : BasicField<VirtualField<T>> {
    using value_type = T;
  };

  template<class Field>
  struct Invariant {};

  template<class T>
  [[nodiscard]] Object<T> object(T& o) noexcept {
    return Object<T>{self(), o};
  }

  template<typename T>
  [[nodiscard]] RawField<T> field(std::string_view name,
                                  T& value) const noexcept {
    static_assert(!std::is_const<T>::value);
    return RawField<T>{{name}, value};
  }

  template<class T>
  static std::string_view getFieldName(T& field) noexcept {
    if constexpr (requires() { field.inner; }) {
      return getFieldName(field.inner);
    } else {
      return field.name;
    }
  }

  template<class T>
  static auto& getFieldValue(T& field) noexcept {
    if constexpr (requires() { field.inner; }) {
      return getFieldValue(field.inner);
    } else {
      return field.value;
    }
  }

  template<class T>
  static decltype(auto) getFallbackValue(T& field) noexcept {
    if constexpr (requires() { field.fallbackValue; }) {
      auto& result = field.fallbackValue;  // We want to return a reference!
      return result;
    } else if constexpr (requires() { field.inner; }) {
      return getFallbackValue(field.inner);
    }
  }

  template<class T>
  static decltype(auto) getTransformer(T& field) noexcept {
    if constexpr (requires() { field.transformer; }) {
      auto& result = field.transformer;  // We want to return a reference!
      return result;
    } else if constexpr (requires() { field.inner; }) {
      return getTransformer(field.inner);
    }
  }
};

}  // namespace arangodb::velocypack::inspection
