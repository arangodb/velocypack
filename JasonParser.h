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
      //     size_t nr = p.parse(json);
      //   }
      //   catch (std::bad_alloc e) {
      //     std::cout << "Out of memory!" << std::endl;
      //   }
      //   catch (JasonParserError e) {
      //     std::cout << "Parse error: " << e.what() << std::endl;
      //   }
      //   JasonBuilder b = p.steal();
      //
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

        JasonLength parse (std::string const& json) {
          _start = reinterpret_cast<uint8_t const*>(json.c_str());
          _size  = json.size();
          _pos   = 0;
          _b.clear();
          return parseInternal();
        }

        JasonLength parse (uint8_t const* start, size_t size) {
          _start = start;
          _size = size;
          _pos = 0;
          _b.clear();
          return parseInternal();
        }

        JasonLength parse (char const* start, size_t size) {
          _start = reinterpret_cast<uint8_t const*>(start);
          _size = size;
          _pos = 0;
          _b.clear();
          return parseInternal();
        }

        // We probably want a parse from stream at some stage...
        
        JasonBuilder&& steal () {
          return std::move(_b);
        }

      private:

        int consume () {
          if (_pos >= _size) {
            return -1;
          }
          return static_cast<int>(_start[_pos++]);
        }

        void unconsume () {
          _pos--;
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

        JasonLength parseInternal () {
          std::vector<int64_t> temp;
          if (_size > 1024) {
            temp.reserve(_size / 32);
          }
          else {
            temp.reserve(8);
          }
          int64_t nr = 0;
          JasonLength len = 0;
          scanJson(temp, nr, len);
          _pos = 0;
          _b.reserve(len);
          buildJason(temp);
          assert(nr >= 0);
          return static_cast<JasonLength>(nr);
        }

        static inline bool isWhiteSpace (int i) {
          return i == ' ' || i == '\t' || i == '\n' || i == '\r' || i == '\f';
        }

        inline int skipWhiteSpace (char const* err) {
          int i;
          do {
            i = consume();
            if (i < 0) {
              throw JasonParserError(err);
            }
          } while (isWhiteSpace(i));
          return i;
        }

        void scanNumber (JasonLength& len) {
          // ...
          len += 9;   // this is an upper bound
        }

        void scanString (JasonLength& len) {
          // ...
        }

        void scanArray(std::vector<int64_t>& temp,
                       int64_t& nr, JasonLength& len) {
          nr = 0;
          int i;
          uint8_t c;
          
          JasonLength startLen = len;

          while (true) {
            i = skipWhiteSpace("scanArray: item or ] expected");
            if (i == ']') {
              break;
            }
            int64_t nr1 = 0;
            scanJson(temp, nr1, len);
            if (nr1 != 1) {
              throw JasonParserError("scanArray: exactly one item expected");
            }
            nr++;
            i = skipWhiteSpace("scanArray: , or ] expected");
            if (i == ']') {
              break;
            }
            if (i != ',') {
              throw JasonParserError("scanArray: , or ] expected");
            }
          }
            
          if (nr > 255 || len - startLen + 514 > 65535) {
            // ArrayLong
            len += (nr > 1) ? 8 * (nr + 1) : 16;
            nr = -nr;
          }
          else {
            // Array
            len += (nr > 1) ? 2 * (nr + 1) : 4;
          }
        }
                       
        void scanObject(std::vector<int64_t>& temp,
                        int64_t& nr, JasonLength& len) {
          nr = 0;
          int i;
          uint8_t c;
          
          JasonLength startLen = len;

          while (true) {
            i = skipWhiteSpace("scanObject: \" or } expected");
            if (i == '}') {
              break;
            }
            if (i != '"') {
              throw JasonParserError("scanObject: \" or } expected");
            }
            scanString(len);
            i = skipWhiteSpace("scanObject: : expected");
            if (i != ':') {
              throw JasonParserError("scanObject: : expected");
            }

            int64_t nr1 = 0;
            scanJson(temp, nr1, len);
            if (nr1 != 1) {
              throw JasonParserError("scanObject: exactly one item expected");
            }
            nr++;
            i = skipWhiteSpace("scanObject: , or } expected");
            if (i == '}') {
              break;
            }
            if (i != ',') {
              throw JasonParserError("scanObject: , or } expected");
            }
          }
            
          if (nr > 255 || len - startLen + 516 > 65535) {
            // ObjectLong
            len += 8 * (nr + 2);
            nr = -nr;
          }
          else {
            // Object
            len += 2 * (nr + 2);
          }
        }
                       
        void scanJson (std::vector<int64_t>& temp,
                       int64_t& nr, JasonLength& len) {
          nr = 0;
          int i;
          uint8_t c;
          while (true) {
            i = consume();
            if (i < 0) {
              break;  // OK to stop here
            }
            c = static_cast<uint8_t>(i);
            switch (c) {
              case ',':   // OK to stop here, do not consume the comma
              case ']':
              case '}':
                unconsume();
                break;
              case ' ':   // WHITESPACE is ignored here
              case '\n':
              case '\r':
              case '\t':
              case '\f':
                continue;
              case '{': {
                size_t tempPos = temp.size();
                temp.push_back(0);      // Here we will put the size        
                scanObject(temp, temp[tempPos], len);
                             // this consumes the closing '}' or throws
                nr++;
                break;
              }
              case '[': {
                size_t tempPos = temp.size();
                temp.push_back(0);      // Here we will put the size
                scanArray(temp, temp[tempPos], len);  
                             // this consumes the closing '}' or throws
                nr++;
                break;
              }
              case 't':
                scanTrue(len);  // this consumes "rue" or throws
                nr++;
                break;
              case 'f':
                scanFalse(len);  // this consumes "alse" or throws
                nr++;
                break;
              case 'n':
                scanNull(len);  // this consumes "ull" or throws
                nr++;
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
                scanNumber(len);  // this consumes the number or throws
                // Maybe we should do better here and detect integers?
                nr++;
                break;
              case '"':
                scanString(len);
                nr++;
                break;
            }
          }
        }

        void buildJason (std::vector<int64_t>& temp) {
#if 0
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
#endif
        }

        void scanTrue (JasonLength& len) {
          // Called, when main mode has just seen a 't', need to see "rue" next
          if (consume() != 'r' || consume() != 'u' || consume() != 'e') {
            throw JasonParserError("true expected");
          }
          len++;
        }

        void scanFalse (JasonLength& len) {
          // Called, when main mode has just seen a 'f', need to see "alse" next
          if (consume() != 'a' || consume() != 'l' || consume() != 's' ||
              consume() != 'e') {
            throw JasonParserError("false expected");
          }
          len++;
        }

        void scanNull (JasonLength& len) {
          // Called, when main mode has just seen a 'n', need to see "ull" next
          if (consume() != 'u' || consume() != 'l' || consume() != 'l') {
            throw JasonParserError("null expected");
          }
          len++;
        }

    };

  }  // namespace triagens::basics
}  // namespace triagens

#endif
