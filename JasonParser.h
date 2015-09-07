#ifndef JASON_PARSER_H
#define JASON_PARSER_H 1

#include "JasonBuilder.h"

namespace triagens {
  namespace basics {

    class JasonParser {

      // Use as follows:
      //   JasonParser p;
      //   try {
      //     std::string error = p.parse(json);
      //     if (error.empty()) {
      //       size_t len;
      //       uint8_t* jason = p.get(len);
      //       // copy away the Jason at jason with length len
      //     }
      //   }
      //   catch (...) {
      //     // out of memory here
      //   }
      //
      // Improve performance
        JasonBuilder _b;

      public:

        JasonParser () {
        }

        std::string parse (std::string const& json) {
          // returns an empty string if OK or an error message
          // throws OUT_OF_MEMORY exception if memory runs out
          // The resulting Jason can be queried, if successful,
          // using get. Attention: the destructor invalidates the
          // memory, so make a copy.
          // Parse can be called multiple times on the same object.
          return std::string("");
        }

        // We probably want a parse from stream at some stage...
        
        uint8_t* get (size_t& len) {
          return nullptr;
        }
    };

  }  // namespace triagens::basics
}  // namespace triagens

#endif
