#ifndef JASON_TYPE_H
#define JASON_TYPE_H

#include <string>

namespace triagens {
  namespace basics {

    struct JasonTypeError : std::exception {
      private:
        std::string _msg;
      public:
        JasonTypeError (std::string const& msg) : _msg(msg) {
        }
        char const* what() const noexcept {
          return _msg.c_str();
        }
    };
    
    enum JasonType {
      None,           // not yet initialized
      Null,           // JSON null
      Bool,
      Double,
      String,
      Array,
      Object,
      External,
      ID,
      ArangoDB_id,
      UTCDate,
      Int,
      UInt,
      SmallInt,
      BCD,
      Binary
    };

    static inline char const* JasonTypeName (JasonType type) {
      switch (type) {
        case JasonType::None:        return "none";
        case JasonType::Null:        return "null";
        case JasonType::Bool:        return "bool";
        case JasonType::Double:      return "double";
        case JasonType::String:      return "string";
        case JasonType::Array:       return "array";
        case JasonType::Object:      return "object";
        case JasonType::External:    return "external";
        case JasonType::ID:          return "id";
        case JasonType::ArangoDB_id: return "arangodb_id";
        case JasonType::UTCDate:     return "utc-date";
        case JasonType::Int:         return "int";
        case JasonType::UInt:        return "uint";
        case JasonType::SmallInt:    return "smallint";
        case JasonType::Binary:      return "binary";
        case JasonType::BCD:         return "bcd";
      }

      return "unknown";
    }

  }  // namespace triagens::basics
}  // namespace triagens

#endif
