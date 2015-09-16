#ifndef JASON_PARSER_H
#define JASON_PARSER_H 1

#include "JasonType.h"
#include "Jason.h"
#include "JasonBuilder.h"

namespace triagens {
  namespace basics {

    class JasonParser {

      // This class can parse JSON very rapidly, but only from contiguous
      // blocks of memory. It builds the result using the JasonBuilder.
      //
      // Use as follows:
      //   JasonParser p;
      //   std::string json = "{\"a\":12}";
      //   try {
      //     p.parse(json);
      //   }
      //   catch (std::bad_alloc e) {
      //     std::cout << "Out of memory!" << std::endl;
      //   }
      //   catch (JasonParserError e) {
      //     std::cout << "Parse error: " << e.what() << std::endl;
      //   }
      //   JasonBuilder b(p);
      //   // p is now empty again and ready to parse more.

        JasonBuilder   _b;
        uint8_t const* _start;
        size_t         _size;
        size_t         _pos;

      public:

        struct JasonParserError : std::exception {
          private:
            std::string _msg;
          public:
            JasonParserError (std::string const& msg) : _msg(msg) {
            }
            char const* what() const noexcept {
              return _msg.c_str();
            }
        };

        JasonParser () : _start(nullptr), _size(0), _pos(0) {
        }

        void parse (std::string const& json) {
          _start = reinterpret_cast<uint8_t const*>(json.c_str());
          _size  = json.size();
          _pos   = 0;
          _b.clear();
          _b.reserve(_size);
          parseInternal();
        }

        void parse (uint8_t const* start, size_t size) {
          _start = start;
          _size = size;
          _pos = 0;
          _b.clear();
          _b.reserve(_size);
          parseInternal();
        }

        void parse (char const* start, size_t size) {
          _start = reinterpret_cast<uint8_t const*>(start);
          _size = size;
          _pos = 0;
          _b.clear();
          _b.reserve(_size);
          parseInternal();
        }

        // We probably want a parse from stream at some stage...
        
        JasonBuilder&& get () {
          return std::move(_b);
        }

      private:

        int peek () {
          if (_pos >= _size) {
            return -1;
          }
          return static_cast<int>(_start[_pos]);
        }

        int consume () {
          if (_pos >= _size) {
            return -1;
          }
          return static_cast<int>(_start[_pos++]);
        }

        void reset () {
          _pos = 0;
        }

        // The following function does the actual parse. It gets bytes
        // via peek, consume and reset appends the result to the JasonBuilder
        // in _b. Errors are reported via an exception.
        // Behind the scenes it runs two parses, one to collect sizes and
        // check for parse errors (scan phase) and then one to actually
        // build the result (build phase).

        void parseInternal () {
          JasonBuilder temp;
          if (_size > 1024) {
            temp.reserve(_size / 32);
          }
          scanJson(temp);
          buildJason(temp);
        }

        void scanJson (JasonBuilder& temp) {
          uint64_t count = 0;
          int i;
          uint8_t c;

          while (true) {
            i = consume();
            if (i < 0) {
              break;  // OK to stop here
            }
            c = static_cast<uint8_t>(i);
            switch (c) {
              case ' ':   // WHITESPACE is ignored here
              case '\n':
              case '\r':
              case '\t':
              case '\f':
                continue;
              case '{':
                _b.set(Jason(10,
                             // NEED SIZE HERE
                             JasonType::Object)); 
                             // NEED TO KNOW IF LONG HERE
                parseObject();  // this consumes the closing '}' or throws
                break;
              case '[':
                _b.set(Jason(10,
                             // NEED SIZE HERE
                             JasonType::Array)); 
                             // NEED TO KNOW IF LONG HERE
                scanObject();  // this consumes the closing '}' or throws
                break;
              case 't':
                scanTrue();  // this consumes "rue" or throws
                _b.set(Jason(true));
                count++;
                break;
              case 'f':
                scanFalse();  // this consumes "alse" or throws
                _b.set(Jason(false));
                count++;
                break;
              case 'n':
                scanNull();  // this consumes "ull" or throws
                _b.set(Jason());
                count++;
                break;
              case '-':
              case '0':
              case '1':
              case '2':
              case '3':
              case '4':
              case '5':
              case '6':
              case '7':
              case '8':
              case '9':
                scanNumber();  // this consumes the number or throws
                // Maybe we should do better here and detect integers?
                _b.set(Jason(n, JasonType::Double));
                count++;
                break;
              case '"':
                scanString();  // consumes the string, determines the length
                count++;
                break;
            }
          }
        }

        void buildJason (JasonBuilder temp) {

        }

        void scanTrue () {
          // Called, when main mode has just seen a 't', need to see "rue" next
          if (consume() != 'r' || consume() != 'u' || consume() != 'e') {
            throw JasonParserError("true expected");
          }
        }

        void scanFalse () {
          // Called, when main mode has just seen a 'f', need to see "alse" next
          if (consume() != 'a' || consume() != 'l' || consume() != 's' ||
              consume() != 'e') {
            throw JasonParserError("false expected");
          }
        }

        void scanNull () {
          // Called, when main mode has just seen a 'n', need to see "ull" next
          if (consume() != 'u' || consume() != 'l' || consume() != 'l') {
            throw JasonParserError("null expected");
          }
        }

    };

  }  // namespace triagens::basics
}  // namespace triagens

#endif
