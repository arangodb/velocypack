////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2024 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Business Source License 1.1 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     https://github.com/arangodb/arangodb/blob/devel/LICENSE
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Max Neunhoeffer
/// @author Jan Steemann
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <iosfwd>

#include "velocypack-common.h"

namespace arangodb::velocypack {

enum class ValueType : uint8_t {
  None,     // not yet initialized
  Illegal,  // illegal value
  Null,     // JSON null
  Bool,
  Array,
  Object,
  Double,
  UTCDate,
  External,
  MinKey,
  MaxKey,
  Int,
  UInt,
  SmallInt,
  String,
  Binary,
  BCD,
  Custom,
  Tagged
};

char const* valueTypeName(ValueType);

ValueType valueTypeGroup(ValueType type);

}  // namespace arangodb::velocypack

std::ostream& operator<<(std::ostream&, arangodb::velocypack::ValueType);

using VPackValueType = arangodb::velocypack::ValueType;
