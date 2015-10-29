////////////////////////////////////////////////////////////////////////////////
/// @brief Library to build up Jason documents.
///
/// @file JasonBuilder.h
///
/// DISCLAIMER
///
/// Copyright 2015 ArangoDB GmbH, Cologne, Germany
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
/// @author Max Neunhoeffer
/// @author Jan Steemann
/// @author Copyright 2015, ArangoDB GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "JasonType.h"

using JasonType = arangodb::jason::JasonType;

char const* arangodb::jason::JasonTypeName (JasonType type) {
  switch (type) {
    case JasonType::None:        return "none";
    case JasonType::Null:        return "null";
    case JasonType::Bool:        return "bool";
    case JasonType::Array:       return "array";
    case JasonType::Object:      return "object";
    case JasonType::Double:      return "double";
    case JasonType::UTCDate:     return "utc-date";
    case JasonType::External:    return "external";
    case JasonType::MinKey:      return "min-key";
    case JasonType::MaxKey:      return "max-key";
    case JasonType::Int:         return "int";
    case JasonType::UInt:        return "uint";
    case JasonType::SmallInt:    return "smallint";
    case JasonType::String:      return "string";
    case JasonType::Binary:      return "binary";
    case JasonType::BCD:         return "bcd";
    case JasonType::Custom:      return "custom";
  }

  return "unknown";
}
