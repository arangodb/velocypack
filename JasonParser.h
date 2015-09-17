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

        JasonParser (JasonParser const&) = delete;
        JasonParser& operator= (JasonParser const&) = delete;

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

        int peek () const {
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
          while (_pos < _size && isWhiteSpace(_start[_pos])) {
            ++_pos;
          }
          if (_pos != _size) {
            throw JasonParserError("expecting EOF");
          }
          _pos = 0;
          _b.reserve(len);
          buildJason(temp);
          assert(nr >= 0);
          return static_cast<JasonLength>(nr);
        }

        static inline bool isWhiteSpace (int i) {
          return i == ' ' || i == '\t' || i == '\n' || i == '\r' || i == '\f' || i == '\b';
        }

        // skips over all following whitespace tokens but does not consume the
        // byte following the whitespace
        inline int skipWhiteSpace (char const* err) {
          while (true) {
            int i = peek();
            if (i < 0) {
              throw JasonParserError(err);
            }
            if (! isWhiteSpace(i)) { 
              return i;
            }
            consume();
          } 
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

        uint64_t scanDigits () {
          uint64_t x = 0;
          while (true) {
            int i = consume();
            if (i < 0) {
              return x;
            }
            uint8_t c = static_cast<uint8_t>(i);
            if (c < '0' || c > '9') {
              unconsume();
              return x;
            }
            x = x * 10 + c - '0';
          }
        }

        inline uint8_t getOneOrThrow (char const* msg) {
          int i = consume();
          if (i < 0) {
            throw JasonParserError(msg);
          }
          return static_cast<uint8_t>(i);
        }

        void scanNumber (JasonLength& len) {
          int i;
          uint8_t c = static_cast<uint8_t>(consume());
          // We know that a character is coming, and we know it is '-' or a
          // digit, otherwise we would not have been called.
          if (c == '-') {
            c = getOneOrThrow("scanNumber: incomplete number");
          }
          if (c != '0') {
            if (c < '1' || c > '9') {
              throw JasonParserError("scanNumber: incomplete number");
            }
            unconsume();
            scanDigits();
          }
          i = consume();
          if (i < 0) {
            len += 9;   // FIXME: make this more accurate?
            return;
          }
          c = static_cast<uint8_t>(i);
          if (c != '.') {
            unconsume();
            len += 9;   // FIXME: make this more accurate?
            return;
          }
          c = getOneOrThrow("scanNumber: incomplete number");
          if (c < '0' || c > '9') {
            throw JasonParserError("scanNumber: incomplete number");
          }
          scanDigits();
          i = consume();
          if (i < 0) {
            len += 9;   // FIXME: make this more accurate
            return;
          }
          c = static_cast<uint8_t>(i);
          if (c != 'e' && c != 'E') {
            unconsume();
            len += 9;   // FIXME: make this more accurate
            return;
          }
          c = getOneOrThrow("scanNumber: incomplete number");
          if (c == '+' || c == '-') {
            c = getOneOrThrow("scanNumber: incomplete number");
          }
          if (c < '0' || c > '9') {
            throw JasonParserError("scanNumber: incomplete number");
          }
          scanDigits();
          len += 9;   // FIXME: make this more accurate
        }

        int64_t scanString (JasonLength& len) {
          // When we get here, we have seen a " character and now want to
          // find the end of the string and count the number of bytes.
          // len is increased by the amount of bytes needed in the Jason
          // representation and the return value is the number of bytes
          // in the actual string.
          int i;
          int64_t byteLen = 0;

          while (true) {
            uint8_t c = getOneOrThrow("scanString: Unfinished string detected.");
            // note: control chars in strings are actually valid 
            // if (c < 32) {
            //   throw JasonParserError("scanString: Control character detected.");
            // }
            switch (c) {
              case '"':
                len += byteLen;
                if (byteLen < 128) {
                  len += 1;
                }
                else {
                  uint64_t x = static_cast<uint32_t>(byteLen);
                  len += 1;
                  while (x != 0) {
                    len++;
                    x >>= 8;
                  }
                }
                return byteLen;
              case '\\':
                // Handle cases or throw error
                i = consume();
                if (i < 0) {
                  throw JasonParserError("scanString: Unfinished string detected.");
                }
                c = static_cast<uint8_t>(i);
                switch (c) {
                  case '"':
                  case '\\':
                  case '/':
                  case 'b':
                  case 'f':
                  case 'n':
                  case 'r':
                  case 't':
                    byteLen++;
                    break;
                  case 'u': {
                    uint32_t v = 0;
                    for (int j = 0; j < 4; j++) {
                      i = consume();
                      if (i < 0) {
                        throw JasonParserError("scanString: Unfinished \\uXXXX.");
                      }
                      c = static_cast<uint8_t>(i);
                      if (c >= '0' && c <= '9') {
                        v = (v << 8) + c - '0';
                      }
                      else if (c >= 'a' && c <= 'f') {
                        v = (v << 8) + c - 'a' + 10;
                      }
                      else if (c >= 'A' && c <= 'F') {
                        v = (v << 8) + c - 'A' + 10;
                      }
                      else {
                        throw JasonParserError("scanString: Illegal hash digit.");
                      }
                    }
                    if (v >= 0x4000) {
                      byteLen += 3;
                    }
                    else if (v >= 0x80) {
                      byteLen += 2;
                    }
                    else {
                      byteLen++;
                    }
                    break;
                  }
                  default:
                    throw JasonParserError("scanString: Illegal \\ sequence.");
                }
                break;
              default:
                byteLen++;
                break;
            }
          }
        }

        void scanArray (std::vector<int64_t>& temp,
                        int64_t& nr, JasonLength& len) {
          nr = 0;
          
          JasonLength startLen = len;

          while (true) {
            int i = skipWhiteSpace("scanArray: item or ] expected");
            if (i == ']' && nr == 0) { 
              // ']' is only valid here if we haven't seen a ',' last time
              consume();
              break;
            }
            int64_t nr1 = 0;
            // parse array element itself
            scanJson(temp, nr1, len);
            if (nr1 != 1) {
              throw JasonParserError("scanArray: exactly one item expected");
            }
            nr++;
            i = skipWhiteSpace("scanArray: , or ] expected");
            if (i == ']') {
              // end of array
              consume();
              break;
            }
            if (i != ',') {
              throw JasonParserError("scanArray: , or ] expected");
            }
            // skip over ','
            consume();
          }
            
          // TODO: what is the meaning of magic number 514? 2 * 256 + 2, but why??
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
                       
        void scanObject (std::vector<int64_t>& temp,
                         int64_t& nr, JasonLength& len) {
          nr = 0;
          
          JasonLength startLen = len;

          while (true) {
            int i = skipWhiteSpace("scanObject: \" or } expected");
            if (i == '}' && nr == 0) {
              // '}' is only valid here if we haven't seen a ',' last time
              consume();
              break;
            }
            // always expecting a string attribute name here
            if (i != '"') {
              throw JasonParserError("scanObject: \" or } expected");
            }
            scanString(len);
            skipWhiteSpace("scanObject: : expected");
            // always expecting the ':' here
            i = consume();
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
              // end of object
              consume();
              break;
            }
            if (i != ',') {
              throw JasonParserError("scanObject: , or } expected");
            } 
            // skip over ','
            consume();
          }
            
          // TODO: what is the meaning of magic number 516? 2 * 256 + 4, but why??
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
          skipWhiteSpace("expecting item");
          int i = consume();
          if (i < 0) {
            return; 
          }
          uint8_t c = static_cast<uint8_t>(i);
          switch (c) {
            case '{': {
              size_t tempPos = temp.size();
              temp.push_back(0);      // Here we will put the size       
              int64_t tempNr; 
              // pass local variable tempNr to scanObject because passing
              // temp[tempPos] is unsafe if the vector gets resized. then
              // &temp[tempPos] may point into the void
              scanObject(temp, tempNr, len);
              temp[tempPos] = tempNr;
                           // this consumes the closing '}' or throws
              nr++;
              break;
            }
            case '[': {
              size_t tempPos = temp.size();
              temp.push_back(0);      // Here we will put the size
              int64_t tempNr;
              // pass local variable tempNr to scanArray because passing
              // temp[tempPos] is unsafe if the vector gets resized. then
              // &temp[tempPos] may point into the void
              scanArray(temp, tempNr, len);  
              temp[tempPos] = tempNr;
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
              unconsume();
              scanNumber(len);  // this consumes the number or throws
              // Maybe we should do better here and detect integers?
              nr++;
              break;
            case '"': {
              temp.push_back(scanString(len));
              nr++;
              break;
            }
            default: {
              throw JasonParserError("value expected");
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
              case '\b':
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

    };

  }  // namespace triagens::basics
}  // namespace triagens

#endif
