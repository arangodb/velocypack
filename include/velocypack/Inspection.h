////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2020 ArangoDB GmbH, Cologne, Germany
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

#include "velocypack/inspection/LoadInspector.h"
#include "velocypack/inspection/SaveInspector.h"

namespace arangodb::velocypack {

template<class T>
void serialize(Builder& builder, T& value) {
  inspection::SaveInspector inspector(builder);
  if (auto res = inspector.apply(value); !res.ok()) {
    throw Exception(Exception::SerializationError,
                    res.error() + "\nPath: " + res.path());
  }
}

template<class T>
T deserialize(Slice slice, inspection::ParseOptions options = {}) {
  inspection::LoadInspector inspector(slice, options);
  T result;
  if (auto res = inspector.apply(result); !res.ok()) {
    throw Exception(Exception::ParseError,
                    res.error() + "\nPath: " + res.path());
  }
  return result;
}

template<class T>
void deserialize(Slice slice, T& result,
                 inspection::ParseOptions options = {}) {
  inspection::LoadInspector inspector(slice, options);
  if (auto res = inspector.apply(result); !res.ok()) {
    throw Exception(Exception::ParseError,
                    res.error() + "\nPath: " + res.path());
  }
}

template<class T>
T deserialize(Builder& builder) {
  return deserialize<T>(builder.slice());
}

}  // namespace arangodb::velocypack
