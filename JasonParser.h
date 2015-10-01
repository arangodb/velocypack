#ifndef JASON_PARSER_H
#define JASON_PARSER_H 1

#include <math.h>

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
      //     std::cout << "Position of error: " << p.errorPos() << std::endl;
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

        JasonOptions options;        

        JasonParser (JasonParser const&) = delete;
        JasonParser& operator= (JasonParser const&) = delete;

        JasonParser () : _start(nullptr), _size(0), _pos(0) {
        }

        JasonLength parse (std::string const& json, bool multi = false) {
          _start = reinterpret_cast<uint8_t const*>(json.c_str());
          _size  = json.size();
          _pos   = 0;
          _b.clear();
          return parseInternal(multi);
        }

        JasonLength parse (uint8_t const* start, size_t size,
                           bool multi = false) {
          _start = start;
          _size = size;
          _pos = 0;
          _b.clear();
          return parseInternal(multi);
        }

        JasonLength parse (char const* start, size_t size,
                           bool multi = false) {
          _start = reinterpret_cast<uint8_t const*>(start);
          _size = size;
          _pos = 0;
          _b.clear();
          return parseInternal(multi);
        }

        // We probably want a parse from stream at some stage...
        // Not with this high-performance two-pass approach. :-(
        
        JasonBuilder&& steal () {
          return std::move(_b);
        }

        // Beware, only valid as long as you do not parse more, use steal
        // to move the data out!
        uint8_t const* jason () {
          return _b.start();
        }

        // Returns the position at the time when the just reported error
        // occurred, only use when handling an exception.
        size_t errorPos () const {
          return _pos > 0 ? _pos - 1 : _pos;
        }

        void clear () {
          _b.clear();
        }

      private:

        inline int peek () const {
          if (_pos >= _size) {
            return -1;
          }
          return static_cast<int>(_start[_pos]);
        }

        inline int consume () {
          if (_pos >= _size) {
            return -1;
          }
          return static_cast<int>(_start[_pos++]);
        }

        inline void unconsume () {
          --_pos;
        }

        inline void reset () {
          _pos = 0;
        }

        // The following function does the actual parse. It gets bytes
        // via peek, consume and reset appends the result to the JasonBuilder
        // in _b. Errors are reported via an exception.
        // Behind the scenes it runs two parses, one to collect sizes and
        // check for parse errors (scan phase) and then one to actually
        // build the result (build phase).

        JasonLength parseInternal (bool multi) {
          _b.options = options; // copy over options

          // skip over optional BOM
          if (_size >= 3 &&
              _start[0] == 0xef &&
              _start[1] == 0xbb &&
              _start[2] == 0xbf) {
            // found UTF-8 BOM. simply skip over it
            _pos += 3;
          }

          JasonLength nr = 0;
          do {
            parseJson();
            nr++;
            while (_pos < _size && 
                   isWhiteSpace(_start[_pos])) {
              ++_pos;
            }
            if (! multi && _pos != _size) {
              consume();  // to get error reporting right
              throw JasonParserError("expecting EOF");
            }
          } 
          while (multi && _pos < _size);
          return nr;
        }

        // FIXME: implement a white space table?
        inline bool isWhiteSpace (uint8_t i) const {
          return (i == ' ' || i == '\t' || i == '\n' || i == '\r');
        }

        // skips over all following whitespace tokens but does not consume the
        // byte following the whitespace
        inline int skipWhiteSpace (char const* err) {
          while (_pos < _size) {
            if (! isWhiteSpace(_start[_pos])) { 
              return static_cast<int>(_start[_pos]);;
            }
            ++_pos;
          } 
          throw JasonParserError(err);
        }

        // The fast non-checking variant:
        inline int skipWhiteSpaceNoCheck () {
          while (_pos < _size) {
            if (! isWhiteSpace(_start[_pos])) {
              return static_cast<int>(_start[_pos]);
            }
            ++_pos;
          } 
          return -1;
        }

        void parseTrue () {
          // Called, when main mode has just seen a 't', need to see "rue" next
          if (consume() != 'r' || consume() != 'u' || consume() != 'e') {
            throw JasonParserError("true expected");
          }
          _b.addTrue();
        }

        void parseFalse () {
          // Called, when main mode has just seen a 'f', need to see "alse" next
          if (consume() != 'a' || consume() != 'l' || consume() != 's' ||
              consume() != 'e') {
            throw JasonParserError("false expected");
          }
          _b.addFalse();
        }

        void parseNull () {
          // Called, when main mode has just seen a 'n', need to see "ull" next
          if (consume() != 'u' || consume() != 'l' || consume() != 'l') {
            throw JasonParserError("null expected");
          }
          _b.addNull();
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

        double scanDigitsFractional () {
          double pot = 0.1;
          double x = 0.0;
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
            x = x + pot * (c - '0');
            pot /= 10.0;
          }
        }

        inline int getOneOrThrow (char const* msg) {
          int i = consume();
          if (i < 0) {
            throw JasonParserError(msg);
          }
          return i;
        }

        void parseNumber () {
          uint64_t integerPart = 0;
          double   fractionalPart;
          uint64_t expPart;
          bool negative = false;
          int i = consume();
          // We know that a character is coming, and it's a number if it
          // starts with '-' or a digit. otherwise it's invalid
          if (i == '-') {
            i = getOneOrThrow("scanNumber: incomplete number");
            negative = true;
          }
          if (i < '0' || i > '9') {
            throw JasonParserError("value expected");
          }
          
          if (i != '0') {
            unconsume();
            integerPart = scanDigits();
          }
          i = consume();
          if (i < 0) {
            if (negative) {
              _b.addNegInt(integerPart);
            }
            else {
              _b.addUInt(integerPart);
            }
            return;
          }
          if (i != '.') {
            unconsume();
            if (negative) {
              _b.addNegInt(integerPart);
            }
            else {
              _b.addUInt(integerPart);
            }
            return;
          }
          i = getOneOrThrow("scanNumber: incomplete number");
          if (i < '0' || i > '9') {
            throw JasonParserError("scanNumber: incomplete number");
          }
          unconsume();
          fractionalPart = scanDigitsFractional();
          if (negative) {
            fractionalPart = -static_cast<double>(integerPart) - fractionalPart;
          }
          else {
            fractionalPart = static_cast<double>(integerPart) + fractionalPart;
          }
          i = consume();
          if (i < 0) {
            _b.addDouble(fractionalPart);
            return;
          }
          if (i != 'e' && i != 'E') {
            unconsume();
            _b.addDouble(fractionalPart);
            return;
          }
          i = getOneOrThrow("scanNumber: incomplete number");
          negative = false;
          if (i == '+' || i == '-') {
            negative = (i == '-');
            i = getOneOrThrow("scanNumber: incomplete number");
          }
          if (i < '0' || i > '9') {
            throw JasonParserError("scanNumber: incomplete number");
          }
          unconsume();
          expPart = scanDigits();
          if (negative) {
            fractionalPart *= pow(10, -static_cast<double>(expPart));
          }
          else {
            fractionalPart *= pow(10, static_cast<double>(expPart));
          }
          _b.addDouble(fractionalPart);
        }

        void parseString () {
          // When we get here, we have seen a " character and now want to
          // find the end of the string and parse the string value to its
          // Jason representation. We assume that the string is short and
          // insert 8 bytes for the length as soon as we reach 127 bytes
          // in the Jason representation.

          JasonLength base = _b._pos;
          _b.reserveSpace(1);
          _b._start[_b._pos++] = 0x40;   // correct this later

          bool large = false;          // set to true when we reach 128 bytes
          uint32_t highSurrogate = 0;  // non-zero if high-surrogate was seen

          while (true) {
            int i = getOneOrThrow("scanString: Unfinished string detected.");
            switch (i) {
              case '"':
                JasonLength len;
                if (! large) {
                  len = _b._pos - (base + 1);
                  _b._start[base] = 0x40 + static_cast<uint8_t>(len);
                  // String is ready
                }
                else {
                  len = _b._pos - (base + 9);
                  _b._start[base] = 0x0c;
                  for (JasonLength i = 1; i <= 8; i++) {
                    _b._start[base + i] = len & 0xff;
                    len >>= 8;
                  }
                }
                return;
              case '\\':
                // Handle cases or throw error
                i = consume();
                if (i < 0) {
                  throw JasonParserError("scanString: Unfinished string detected.");
                }
                switch (i) {
                  case '"':
                    _b.reserveSpace(1);
                    _b._start[_b._pos++] = '"';
                    highSurrogate = 0;
                    break;
                  case '\\':
                    _b.reserveSpace(1);
                    _b._start[_b._pos++] = '\\';
                    highSurrogate = 0;
                    break;
                  case '/':
                    _b.reserveSpace(1);
                    _b._start[_b._pos++] = '/';
                    highSurrogate = 0;
                    break;
                  case 'b':
                    _b.reserveSpace(1);
                    _b._start[_b._pos++] = '\b';
                    highSurrogate = 0;
                    break;
                  case 'f':
                    _b.reserveSpace(1);
                    _b._start[_b._pos++] = '\f';
                    highSurrogate = 0;
                    break;
                  case 'n':
                    _b.reserveSpace(1);
                    _b._start[_b._pos++] = '\n';
                    highSurrogate = 0;
                    break;
                  case 'r':
                    _b.reserveSpace(1);
                    _b._start[_b._pos++] = '\r';
                    highSurrogate = 0;
                    break;
                  case 't':
                    _b.reserveSpace(1);
                    _b._start[_b._pos++] = '\t';
                    highSurrogate = 0;
                    break;
                  case 'u': {
                    uint32_t v = 0;
                    for (int j = 0; j < 4; j++) {
                      i = consume();
                      if (i < 0) {
                        throw JasonParserError("scanString: Unfinished \\uXXXX.");
                      }
                      if (i >= '0' && i <= '9') {
                        v = (v << 4) + i - '0';
                      }
                      else if (i >= 'a' && i <= 'f') {
                        v = (v << 4) + i - 'a' + 10;
                      }
                      else if (i >= 'A' && i <= 'F') {
                        v = (v << 4) + i - 'A' + 10;
                      }
                      else {
                        throw JasonParserError("scanString: Illegal hash digit.");
                      }
                    }
                    if (v < 0x80) {
                      _b.reserveSpace(1);
                      _b._start[_b._pos++] = static_cast<uint8_t>(v);
                      highSurrogate = 0;
                    }
                    else if (v < 0x800) {
                      _b.reserveSpace(2);
                      _b._start[_b._pos++] = 0xc0 + (v >> 6);
                      _b._start[_b._pos++] = 0x80 + (v & 0x3f);
                      highSurrogate = 0;
                    }
                    else if (v >= 0xdc00 && v < 0xe000 &&
                             highSurrogate != 0) {
                      // Low surrogate, put the two together:
                      v = 0x10000 + ((highSurrogate - 0xd800) << 10)
                                  + v - 0xdc00;
                      _b._pos -= 3;
                      _b.reserveSpace(4);
                      _b._start[_b._pos++] = 0xf0 + (v >> 18);
                      _b._start[_b._pos++] = 0x80 + ((v >> 12) & 0x3f);
                      _b._start[_b._pos++] = 0x80 + ((v >> 6) & 0x3f);
                      _b._start[_b._pos++] = 0x80 + (v & 0x3f);
                      highSurrogate = 0;
                    }
                    else {
                      if (v >= 0xd800 && v < 0xdc00) {
                        // High surrogate:
                        highSurrogate = v;
                      }
                      else {
                        highSurrogate = 0;
                      }
                      _b.reserveSpace(3);
                      _b._start[_b._pos++] = 0xe0 + (v >> 12);
                      _b._start[_b._pos++] = 0x80 + ((v >> 6) & 0x3f);
                      _b._start[_b._pos++] = 0x80 + (v & 0x3f);
                    }
                    break;
                  }
                  default:
                    throw JasonParserError("scanString: Illegal \\ sequence.");
                }
                break;
              default:
                if ((i & 0x80) == 0) {
                  // non-UTF-8 sequence
                  if (i < 0x20) {
                    // control character
                    throw JasonParserError("scanString: Found control character.");
                  }
                  highSurrogate = 0;
                  _b.reserveSpace(1);
                  _b._start[_b._pos++] = static_cast<uint8_t>(i);
                }
                else {
                  // multi-byte UTF-8 sequence!
                  int follow = 0;
                  if ((i & 0xe0) == 0x80) {
                    throw JasonParserError("scanString: Illegal UTF-8 byte.");
                  }
                  else if ((i & 0xe0) == 0xc0) {
                    // two-byte sequence
                    follow = 1;
                  }
                  else if ((i & 0xf0) == 0xe0) {
                    // three-byte sequence
                    follow = 2;
                  }
                  else if ((i & 0xf8) == 0xf0) {
                    // four-byte sequence
                    follow = 3;
                  }
                  else {
                    throw JasonParserError("scanString: Illegal 5- or 6-byte sequence found in UTF-8 string.");
                  }

                  // validate follow up characters
                  _b.reserveSpace(1 + follow);
                  _b._start[_b._pos++] = static_cast<uint8_t>(i);
                  for (int j = 0; j < follow; ++j) {
                    i = getOneOrThrow("scanString: truncated UTF-8 sequence");
                    if ((i & 0xc0) != 0x80) {
                      throw JasonParserError("scanString: invalid UTF-8 sequence");
                    }
                    _b._start[_b._pos++] = static_cast<uint8_t>(i);
                  }
                  highSurrogate = 0;
                }
                break;
            }
            if (! large && _b._pos - (base + 1) > 127) {
              large = true;
              _b.reserveSpace(8);
              memmove(_b._start + base + 9, _b._start + base + 1,
                      _b._pos - (base + 1));
              _b._pos += 8;
            }
          }
        }

        void parseArray () {
          JasonLength base = _b._pos;
          _b.addArray();

          int i = skipWhiteSpace("scanArray: item or ] expected");
          if (i == ']') {
            // empty array
            consume();   // the closing ']'
            _b.close();
            return;
          }

          while (true) {
            // parse array element itself
            _b.reportAdd(base);
            parseJson();
            i = skipWhiteSpace("scanArray: , or ] expected");
            if (i == ']') {
              // end of array
              consume();
              _b.close();
              return;
            }
            // skip over ','
            if (i != ',') {
              throw JasonParserError("scanArray: , or ] expected");
            }
            consume();
          }
        }
                       
        void parseObject () {
          JasonLength base = _b._pos;
          _b.addObject();

          int i = skipWhiteSpace("scanObject: item or } expected");
          if (i == '}') {
            // empty array
            consume();   // the closing ']'
            _b.close();
            return;
          }

          while (true) {

            // always expecting a string attribute name here
            if (i != '"') {
              throw JasonParserError("scanObject: \" or } expected");
            }
            // get past the initial '"'
            consume();

            _b.reportAdd(base);
            parseString();
            i = skipWhiteSpace("scanObject: : expected");
            // always expecting the ':' here
            if (i != ':') {
              throw JasonParserError("scanObject: : expected");
            }
            consume();

            parseJson();
            i = skipWhiteSpace("scanObject: , or } expected");
            if (i == '}') {
              // end of object
              consume();
              _b.close();
              return;
            }
            if (i != ',') {
              throw JasonParserError("scanObject: , or } expected");
            } 
            // skip over ','
            consume();
            i = skipWhiteSpace("scanObject: \" or } expected");
          }
        }
                       
        void parseJson () {
          skipWhiteSpace("expecting item");
          int i = consume();
          if (i < 0) {
            return; 
          }
          switch (i) {
            case '{': {
              parseObject();  // this consumes the closing '}' or throws
              break;
            }
            case '[': {
              parseArray();   // this consumes the closing ']' or throws
              break;
            }
            case 't':
              parseTrue();    // this consumes "rue" or throws
              break;
            case 'f':
              parseFalse();   // this consumes "alse" or throws
              break;
            case 'n':
              parseNull();    // this consumes "ull" or throws
              break;
            case '"': {
              parseString();
              break;
            }
            default: {
              // everything else must be a number or is invalid...
              // this includes '-' and '0' to '9'. scanNumber() will
              // throw if the input is non-numeric
              unconsume();
              parseNumber();  // this consumes the number or throws
              break;
            }
          }
        }

    };

  }  // namespace triagens::basics
}  // namespace triagens

#endif
