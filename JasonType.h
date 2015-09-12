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
      None,           // not yet initialised
      Null,           // JSON null
      Bool,
      Double,
      String,
      Array,
      ArrayLong,
      Object,
      ObjectLong,
      External,
      ID,
      ArangoDB_id,
      UTCDate,
      Int,
      UInt,
      Binary
    };

    char const* JasonTypeName (JasonType);

  }  // namespace triagens::basics
}  // namespace triagens

#endif
