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

#include "velocypack/inspection/InspectorAccess.h"

namespace arangodb::velocypack::inspection {

template<class Derived>
struct InspectorBase {
  Derived& self() { return static_cast<Derived&>(*this); }

  template<class T>
  [[nodiscard]] Result apply(T& x) {
    return process(self(), x);
  }

  struct Object {
    template<class... Args>
    [[nodiscard]] Result fields(Args... args) {
      if (auto res = inspector.beginObject(); !res.ok()) {
        return res;
      }

      if (auto res = inspector.applyFields(std::forward<Args>(args)...);
          !res.ok()) {
        return res;
      }

      return inspector.endObject();
    }

   private:
    friend struct InspectorBase;
    explicit Object(Derived& inspector) : inspector(inspector) {}

    Derived& inspector;
  };

  template<typename DerivedField>
  struct Field {
    std::string_view name;

    template<class Predicate>
    [[nodiscard]] auto invariant(Predicate predicate) &&;

    template<class U>
    [[nodiscard]] auto fallback(U val) &&;
  };

  template<typename T>
  struct RawField : Field<RawField<T>> {
    using value_type = T;
    T* value;
  };

  template<typename T>
  struct VirtualField : Field<VirtualField<T>> {
    using value_type = T;
  };

  template<class Field>
  struct Invariant {};

  [[nodiscard]] Object object() noexcept { return Object{self()}; }

  template<typename T>
  [[nodiscard]] RawField<T> field(std::string_view name,
                                  T& value) const noexcept {
    static_assert(!std::is_const<T>::value);
    return RawField<T>{{name}, std::addressof(value)};
  }

 private:
  template<class Arg>
  Result applyFields(Arg arg) {
    return self().applyField(arg);
  }

  template<class Arg, class... Args>
  Result applyFields(Arg arg, Args... args) {
    if (auto res = self().applyField(arg); !res.ok()) {
      return res;
    }
    return applyFields(std::forward<Args>(args)...);
  }
};

}  // namespace arangodb::velocypack::inspection
