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
          std::vector<int64_t> temp;
          if (_size > 1024) {
            temp.reserve(_size / 32);
          }
          else {
            temp.reserve(8);
          }

          // skip over optional BOM
          if (_size >= 3 &&
              _start[0] == 0xef &&
              _start[1] == 0xbb &&
              _start[2] == 0xbf) {
            // found UTF-8 BOM. simply skip over it
            _pos += 3;
          }

          JasonLength len = 0;
          JasonLength nr = 0;
          size_t savePos;
          do {
            savePos = _pos;
            temp.clear();
            scanJson(temp, len);
            while (_pos < _size && 
                   isWhiteSpace(_start[_pos])) {
              ++_pos;
            }
            if (! multi && _pos != _size) {
              consume();  // to get error reporting right
              throw JasonParserError("expecting EOF");
            }
            _pos = savePos;
            _b.reserve(len);
            size_t tempPos = 0;
            buildJason(temp, tempPos);
            while (_pos < _size && 
                   isWhiteSpace(_start[_pos])) {
              ++_pos;
            }
            nr++;
          } 
          while (multi && _pos < _size);
          return nr;
        }

        inline bool isWhiteSpace (uint8_t i) const {
          return (i == ' ' || i == '\t' || i == '\n' || i == '\r' || i == '\f' || i == '\b');
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

        void scanNumber (JasonLength& len) {
          int i = consume();
          // We know that a character is coming, and it's a number if it
          // starts with '-' or a digit. otherwise it's invalid
          if (i == '-') {
            i = getOneOrThrow("scanNumber: incomplete number");
          }
          if (i < '0' || i > '9') {
            throw JasonParserError("value expected");
          }
          
          uint64_t value = 0;
          if (i != '0') {
            if (i < '1' || i > '9') {
              throw JasonParserError("scanNumber: incomplete number");
            }
            unconsume();
            value = scanDigits();
          }
          i = consume();
          if (i < 0) {
            len += 1 + _b.uintLength(value);
            return;
          }
          if (i != '.') {
            unconsume();
            len += 1 + _b.uintLength(value);
            return;
          }
          i = getOneOrThrow("scanNumber: incomplete number");
          if (i < '0' || i > '9') {
            throw JasonParserError("scanNumber: incomplete number");
          }
          len += 1 + sizeof(double);

          unconsume();
          scanDigits();
          i = consume();
          if (i < 0) {
            return;
          }
          if (i != 'e' && i != 'E') {
            unconsume();
            return;
          }
          i = getOneOrThrow("scanNumber: incomplete number");
          if (i == '+' || i == '-') {
            i = getOneOrThrow("scanNumber: incomplete number");
          }
          if (i < '0' || i > '9') {
            throw JasonParserError("scanNumber: incomplete number");
          }
          scanDigits();
        }

        int64_t scanString (JasonLength& len) {
          // When we get here, we have seen a " character and now want to
          // find the end of the string and count the number of bytes.
          // len is increased by the amount of bytes needed in the Jason
          // representation and the return value is the number of bytes
          // in the actual string.
          int64_t byteLen = 0;
          uint32_t highSurrogate = 0;  // non-zero if high-surrogate was seen

          while (true) {
            int i = getOneOrThrow("scanString: Unfinished string detected.");
            switch (i) {
              case '"':
                len += byteLen;
                if (byteLen < 128) {
                  len += 1;
                }
                else {
                  uint64_t x = static_cast<uint64_t>(byteLen);
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
                switch (i) {
                  case '"':
                  case '\\':
                  case '/':
                  case 'b':
                  case 'f':
                  case 'n':
                  case 'r':
                  case 't':
                    byteLen++;
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
                      byteLen++;
                      highSurrogate = 0;
                    }
                    else if (v < 0x800) {
                      byteLen += 2;
                      highSurrogate = 0;
                    }
                    else if (v >= 0xd800 && v < 0xdc00) {
                      // High surrogate:
                      highSurrogate = v;
                      byteLen += 3;  // We will correct this later with 
                                     // the low surrogate pair
                    }
                    else if (v >= 0xdc00 && v < 0xe000) {
                      // Low surrogate:
                      if (highSurrogate == 0) {
                        // no high surrogate before, so let's forget it
                        byteLen += 3;
                      }
                      else {
                        byteLen += 1;   // 3 from before, 1 here, total 4
                        highSurrogate = 0;
                      }
                    }
                    else {
                      byteLen += 3;
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
                  byteLen++;
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
                  for (int j = 0; j < follow; ++j) {
                    i = getOneOrThrow("scanString: truncated UTF-8 sequence");
                    if ((i & 0xc0) != 0x80) {
                      throw JasonParserError("scanString: invalid UTF-8 sequence");
                    }
                  }
                  byteLen += follow + 1;
                  highSurrogate = 0;
                }
                break;
            }
          }
        }

        void scanArray (std::vector<int64_t>& temp,
                        int64_t& nr, JasonLength& len) {
          nr = 0;
          int i = skipWhiteSpace("scanArray: item or ] expected");
          if (i == ']') {
            // empty array
            ++_pos;
            len = 4;
            return;
          }

          JasonLength startLen = len;

          while (true) {
            // parse array element itself
            scanJson(temp, len);
            ++nr;
            i = skipWhiteSpace("scanArray: , or ] expected");
            if (i == ']') {
              // end of array
              ++_pos;
              break;
            }
            else if (nr > 0) {
              // skip over ','
              if (i != ',') {
                throw JasonParserError("scanArray: , or ] expected");
              }
              ++_pos;
            }
          }
            
          // If we have more than 255 entries, then it must be a long array.
          // If we have less than 256, then we need to make sure that all
          // offsets are 16 bit. len-startLen is the size we need for the
          // actual objects, and then we need the header: if there are
          // <= 255 entries, we need 1 byte for the type, 1 byte for the
          // number of entries, 2 bytes for the offset to the end and
          // then nr-1 byte pairs for the offsets, thus 4 + 2*(nr-1).
          // Note that for nr==0 we actually need 4 bytes of header, but
          // then we are in the small case anyway.
          if (nr > 255 || len - startLen + 4 + 2 * (nr - 1) > 65535) {
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
              ++_pos;
              break;
            }

            // always expecting a string attribute name here
            if (i != '"') {
              throw JasonParserError("scanObject: \" or } expected");
            }
            // get past the initial '"'
            ++_pos;

            temp.emplace_back(scanString(len));
            skipWhiteSpace("scanObject: : expected");
            // always expecting the ':' here
            i = consume();
            if (i != ':') {
              throw JasonParserError("scanObject: : expected");
            }

            scanJson(temp, len);
            ++nr;
            i = skipWhiteSpace("scanObject: , or } expected");
            if (i == '}') {
              // end of object
              ++_pos;
              break;
            }
            if (i != ',') {
              throw JasonParserError("scanObject: , or } expected");
            } 
            // skip over ','
            ++_pos;
          }
            
          // If we have more than 255 entries, then it must be a long object.
          // If we have less than 256, then we need to make sure that all
          // offsets are 16 bit. len-startLen is the size we need for the
          // actual objects, and then we need the header:
          // if there are <= 255 entries, we need
          // 1 byte for the type, 1 byte for the number of entries, 2
          // bytes for the offset to the end and then nr byte
          // pairs for the offsets, thus 4 + 2*nr
          if (nr > 255 || len - startLen + 4 + 2 * nr > 65535) {
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
                       JasonLength& len) {
          skipWhiteSpace("expecting item");
          int i = consume();
          if (i < 0) {
            return; 
          }
          switch (i) {
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
                           // this consumes the closing ']' or throws
              break;
            }
            case 't':
              scanTrue(len);  // this consumes "rue" or throws
              break;
            case 'f':
              scanFalse(len);  // this consumes "alse" or throws
              break;
            case 'n':
              scanNull(len);  // this consumes "ull" or throws
              break;
            case '"': {
              temp.emplace_back(scanString(len));
              break;
            }
            default: {
              // everything else must be a number or is invalid...
              // this includes '-' and '0' to '9'. scanNumber() will
              // throw if the input is non-numeric
              unconsume();
              scanNumber(len);  // this consumes the number or throws
              break;
            }
          }
        }

        void buildNumber () {
          uint64_t integerPart = 0;
          double   fractionalPart;
          uint64_t expPart;
          // We know that a character is coming, and we know it is '-' or a
          // digit, otherwise we would not have been called.
          bool negative = false;
          int i = consume();
          if (i == '-') {
            i = consume();
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
              _b.add(Jason(-static_cast<int64_t>(integerPart)));
            }
            else {
              _b.add(Jason(integerPart));
            }
            return;
          }
          if (i != '.') {
            unconsume();
            if (negative) {
              _b.add(Jason(-static_cast<int64_t>(integerPart)));
            }
            else {
              _b.add(Jason(integerPart));
            }
            return;
          }
          fractionalPart = scanDigitsFractional();
          if (negative) {
            fractionalPart = -static_cast<double>(integerPart) - fractionalPart;
          }
          else {
            fractionalPart = static_cast<double>(integerPart) + fractionalPart;
          }
          i = consume();
          if (i < 0) {
            _b.add(Jason(fractionalPart));
            return;
          }
          if (i != 'e' && i != 'E') {
            unconsume();
            _b.add(Jason(fractionalPart));
            return;
          }
          i = getOneOrThrow("scanNumber: incomplete number");
          negative = false;
          if (i == '+' || i == '-') {
            negative = (i == '-');
          }
          else {
            unconsume();  // The first digit
          }
          // We know c is another digit here.
          expPart = scanDigits();
          if (negative) {
            fractionalPart *= pow(10, -static_cast<double>(expPart));
          }
          else {
            fractionalPart *= pow(10, static_cast<double>(expPart));
          }
          _b.add(Jason(fractionalPart));
        }

        void buildString (std::vector<int64_t> const& temp, size_t& tempPos) {
          // When we get here, we have seen a " character and now want to
          // actually build the string from what we see. All error checking
          // has already been done. And: temp[tempPos] is the length of the
          // string that will be created.

          int64_t strLen = temp[tempPos++];
          uint8_t* target;
          uint32_t highSurrogate = 0;

          if (strLen > 127) {
            target = _b.add(JasonPair(static_cast<uint8_t const*>(nullptr), 
                                      static_cast<uint64_t>(strLen),
                                      JasonType::StringLong));
          }
          else {
            target = _b.add(JasonPair(static_cast<uint8_t const*>(nullptr),
                                      static_cast<uint64_t>(strLen),
                                      JasonType::String));
          }

          while (true) {
            int i = consume();
            switch (i) {
              case '"':
                return;
              case '\\':
                i = consume();
                switch (i) {
                  case '"':
                    *target++ = '"';
                    highSurrogate = 0;
                    break;
                  case '\\':
                    *target++ = '\\';
                    highSurrogate = 0;
                    break;
                  case '/':
                    *target++ = '/';
                    highSurrogate = 0;
                    break;
                  case 'b':
                    *target++ = '\b';
                    highSurrogate = 0;
                    break;
                  case 'f':
                    *target++ = '\f';
                    highSurrogate = 0;
                    break;
                  case 'n':
                    *target++ = '\n';
                    highSurrogate = 0;
                    break;
                  case 'r':
                    *target++ = '\r';
                    highSurrogate = 0;
                    break;
                  case 't':
                    *target++ = '\t';
                    highSurrogate = 0;
                    break;
                  case 'u': {
                    uint32_t v = 0;
                    for (int j = 0; j < 4; j++) {
                      i = consume();
                      if (i >= '0' && i <= '9') {
                        v = (v << 4) + i - '0';
                      }
                      else if (i >= 'a' && i <= 'f') {
                        v = (v << 4) + i - 'a' + 10;
                      }
                      else if (i >= 'A' && i <= 'F') {
                        v = (v << 4) + i - 'A' + 10;
                      }
                    }
                    if (v < 0x80) {
                      *target++ = v;
                      highSurrogate = 0;
                      continue;
                    }
                    else if (v < 0x800) {
                      *target++ = 0xc0 + (v >> 6);
                      *target++ = 0x80 + (v & 0x3f);
                      highSurrogate = 0;
                      continue;
                    }
                    else if (v >= 0xdc00 && v < 0xe000 &&
                             highSurrogate != 0) {
                      // now put the two together:
                      v = 0x10000 + ((highSurrogate - 0xd800) << 10)
                                  + v - 0xdc00;
                      target -= 3;
                      *target++ = 0xf0 + (v >> 18);
                      *target++ = 0x80 + ((v >> 12) & 0x3f);
                      *target++ = 0x80 + ((v >> 6) & 0x3f);
                      *target++ = 0x80 + (v & 0x3f);
                      highSurrogate = 0;
                      continue;
                    }
                    if (v >= 0xd800 && v < 0xdc00) {
                      // High surrogate:
                      highSurrogate = v;
                    }
                    else {
                      highSurrogate = 0;
                    }
                    *target++ = 0xe0 + (v >> 12);
                    *target++ = 0x80 + ((v >> 6) & 0x3f);
                    *target++ = 0x80 + (v & 0x3f);
                    break;
                  }
                  default:
                    break;
                }
                break;
              default:
                *target++ = static_cast<uint8_t>(i);
                highSurrogate = 0;
                break;
            }
          }
        }

        void buildObject (std::vector<int64_t> const& temp, size_t& tempPos) {
          // Remembered from previous pass:
          int64_t nrAttrs = temp[tempPos++];
          if (nrAttrs < 0) {
            // Long Object:
            _b.add(Jason(-nrAttrs, JasonType::ObjectLong));
          }
          else {
            _b.add(Jason(nrAttrs, JasonType::Object));
          }
          int64_t nr = 0;
          while (true) {
            int i = skipWhiteSpaceNoCheck();
            if (i == '}' && nr == 0) {
              // '}' is only valid here if we haven't seen a ',' last time
              ++_pos;
              break;
            }
            // always expecting a string attribute name here
            // get past the initial '"'
            ++_pos;

            buildString(temp, tempPos);
            skipWhiteSpaceNoCheck();
            // always expecting the ':' here
            i = consume();

            buildJason(temp, tempPos);
            ++nr;
            i = skipWhiteSpaceNoCheck();
            if (i == '}') {
              // end of object
              ++_pos;
              break;
            }
            // skip over ','
            ++_pos;
          }
          _b.close();
        }
                       
        void buildArray (std::vector<int64_t> const& temp, size_t& tempPos) {
          // Remembered from previous pass:
          int64_t nrEntries = temp[tempPos++];
          if (nrEntries < 0) {
            // Long Array:
            _b.add(Jason(-nrEntries, JasonType::ArrayLong));
          }
          else {
            _b.add(Jason(nrEntries, JasonType::Array));
          }
          int64_t nr = 0;
          while (true) {
            int i = skipWhiteSpaceNoCheck();
            if (i == ']') { 
              // end of array 
              ++_pos;
              break;
            }
            else if (nr > 0) {
              // skip over ','
              if (i != ',') {
                throw JasonParserError("scanArray: , or ] expected");
              }
              ++_pos;
            }

            // parse array element itself
            buildJason(temp, tempPos);
            ++nr;
          }
          _b.close();
        }
                       
        void buildJason (std::vector<int64_t> const& temp, size_t& tempPos) {
          skipWhiteSpaceNoCheck();
          int i = consume();
          if (i < 0) {
            return; 
          }
          switch (i) {
            case '{': {
              buildObject(temp, tempPos);   // this consumes the closing '}'
              break;
            }
            case '[': {
              buildArray(temp, tempPos);  // this consumes the closing '}'
              break;
            }
            case 't':
              // consume "rue"
              _pos += 3;
              _b.add(Jason(true));
              break;
            case 'f':
              // consume "alse"
              _pos += 4;
              _b.add(Jason(false));
              break;
            case 'n':
              // consume "ull"
              _pos += 3;
              _b.add(Jason());
              break;
            case '"': {
              buildString(temp, tempPos);
              break;
            default: {
              unconsume();
              buildNumber();  // this consumes the number
              break;
            }
          }
        }
      }

    };

  }  // namespace triagens::basics
}  // namespace triagens

#endif
