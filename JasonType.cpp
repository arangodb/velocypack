
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
