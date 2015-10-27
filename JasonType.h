#ifndef JASON_TYPE_H
#define JASON_TYPE_H

#include <string>
#include <exception>

namespace arangodb {
  namespace jason {

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
      Custom
    };

    char const* JasonTypeName (JasonType);

  }  // namespace arangodb::jason
}  // namespace arangodb

#endif
