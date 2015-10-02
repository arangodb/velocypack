#ifndef JASON_DUMPER_H
#define JASON_DUMPER_H 1

#include <string>

#include "JasonBuffer.h"
#include "JasonSlice.h"
#include "JasonType.h"
#include "Jason.h"
#include "fpconv.h"

namespace triagens {
  namespace basics {

    enum UnsupportedTypeStrategy {
      STRATEGY_SUPPRESS,
      STRATEGY_FAIL
    };

    // Dumps Jason into a JSON output string
    template<typename T, bool PrettyPrint = false>
    class JasonDumper {

        struct JasonDumperError : std::exception {
          private:
            std::string _msg;
          public:
            JasonDumperError (std::string const& msg) : _msg(msg) {
            }
            char const* what() const noexcept {
              return _msg.c_str();
            }
        };

      public:

        JasonDumper (JasonDumper const&) = delete;
        JasonDumper& operator= (JasonDumper const&) = delete;

        JasonDumper (T& buffer, UnsupportedTypeStrategy strategy = STRATEGY_FAIL) 
          : _buffer(&buffer), _strategy(strategy), _indentation(0) {
        }

        ~JasonDumper () {
        }

        void dump (JasonSlice const& slice) {
          _indentation = 0;
          internalDump(slice);
        }

        static void Dump (JasonSlice const& slice, T& buffer, UnsupportedTypeStrategy strategy = STRATEGY_FAIL) {
          JasonDumper dumper(buffer, strategy);
          dumper.internalDump(slice);
        }

        static void Dump (JasonSlice const* slice, T& buffer, UnsupportedTypeStrategy strategy = STRATEGY_FAIL) {
          JasonDumper dumper(buffer, strategy);
          dumper.internalDump(*slice);
        }
        
        static T Dump (JasonSlice const& slice, UnsupportedTypeStrategy strategy = STRATEGY_FAIL) {
          T buffer;
          JasonDumper dumper(buffer, strategy);
          dumper.internalDump(slice);
          return buffer;
        }

        static T Dump (JasonSlice const* slice, UnsupportedTypeStrategy strategy = STRATEGY_FAIL) {
          T buffer;
          JasonDumper dumper(buffer, strategy);
          dumper.internalDump(*slice);
          return buffer;
        }

        void reset () {
          _buffer.reset();
        }

      private:

        void indent () {
          size_t n = _indentation;
          _buffer->reserve(n);
          for (size_t i = 0; i < n; ++i) {
            _buffer->append("  ", 2);
          }
        }

        void internalDump (JasonSlice const& slice) {
          switch (slice.type()) {
            case JasonType::None: {
              handleUnsupportedType(slice);
              break; 
            }
            case JasonType::Null: {
              _buffer->append("null", 4);
              break;
            }
            case JasonType::Bool: {
              if (slice.getBool()) {
                _buffer->append("true", 4);
              } 
              else {
                _buffer->append("false", 5);
              }
              break;
            }
            case JasonType::Double: {
              char temp[24];
              int len = fpconv_dtoa(slice.getDouble(), &temp[0]);
              _buffer->append(&temp[0], static_cast<JasonLength>(len));
              break; 
            }
            case JasonType::Array: {
              JasonLength const n = slice.length();
              if (PrettyPrint) {
                _buffer->push_back('[');
                _buffer->push_back('\n');
                ++_indentation;
                for (JasonLength i = 0; i < n; ++i) {
                  indent();
                  internalDump(slice.at(i));
                  if (i + 1 !=  n) {
                    _buffer->push_back(',');
                  }
                  _buffer->push_back('\n');
                }
                --_indentation;
                indent();
                _buffer->push_back(']');
              }
              else {
                _buffer->push_back('[');
                for (JasonLength i = 0; i < n; ++i) {
                  if (i > 0) {
                    _buffer->push_back(',');
                  }
                  internalDump(slice.at(i));
                }
                _buffer->push_back(']');
              }
              break;
            }
            case JasonType::Object: {
              JasonLength const n = slice.length();
              if (PrettyPrint) {
                _buffer->push_back('{');
                _buffer->push_back('\n');
                ++_indentation;
                for (JasonLength i = 0; i < n; ++i) {
                  indent();
                  internalDump(slice.keyAt(i));
                  _buffer->append(" : ", 3);
                  internalDump(slice.valueAt(i));
                  if (i + 1 !=  n) {
                    _buffer->push_back(',');
                  }
                  _buffer->push_back('\n');
                }
                --_indentation;
                indent();
                _buffer->push_back('}');
              }
              else {
                _buffer->push_back('{');
                for (JasonLength i = 0; i < n; ++i) {
                  if (i > 0) {
                    _buffer->push_back(',');
                  }
                  internalDump(slice.keyAt(i));
                  _buffer->push_back(':');
                  internalDump(slice.valueAt(i));
                }
                _buffer->push_back('}');
              }
              break;
            }
            case JasonType::External: {
              char const* external = slice.getExternal();
              internalDump(JasonSlice(external));
              break;
            }
            case JasonType::ID: {
              handleUnsupportedType(slice);
              // TODO
              break;
            }
            case JasonType::ArangoDB_id: {
              handleUnsupportedType(slice);
              // TODO
              break;
            }
            case JasonType::UTCDate: {
              handleUnsupportedType(slice);
              // TODO
              break;
            }
            case JasonType::Int:
            case JasonType::UInt:
            case JasonType::SmallInt: {
              dumpInteger(slice);
              break;
            }
            case JasonType::String: {
              JasonLength len;
              char const* p = slice.getString(len);
              _buffer->reserve(2 + len);
              _buffer->push_back('"');
              dumpString(p, len);
              _buffer->push_back('"');
              break;
            }
            case JasonType::Binary: {
              // TODO
              handleUnsupportedType(slice);
              break;
            }
            case JasonType::BCD: {
              // TODO
              handleUnsupportedType(slice);
              break;
            }
          }
        }

        void dumpInteger (JasonSlice slice) {
          if (slice.isType(JasonType::UInt)) {
            uint64_t v = slice.getUInt();

            if (10000000000000000000ULL <= v) { _buffer->push_back('0' + (v / 10000000000000000000ULL) % 10); }
            if ( 1000000000000000000ULL <= v) { _buffer->push_back('0' + (v /  1000000000000000000ULL) % 10); }
            if (  100000000000000000ULL <= v) { _buffer->push_back('0' + (v /   100000000000000000ULL) % 10); }
            if (   10000000000000000ULL <= v) { _buffer->push_back('0' + (v /    10000000000000000ULL) % 10); }
            if (    1000000000000000ULL <= v) { _buffer->push_back('0' + (v /     1000000000000000ULL) % 10); }
            if (     100000000000000ULL <= v) { _buffer->push_back('0' + (v /      100000000000000ULL) % 10); }
            if (      10000000000000ULL <= v) { _buffer->push_back('0' + (v /       10000000000000ULL) % 10); }
            if (       1000000000000ULL <= v) { _buffer->push_back('0' + (v /        1000000000000ULL) % 10); }
            if (        100000000000ULL <= v) { _buffer->push_back('0' + (v /         100000000000ULL) % 10); }
            if (         10000000000ULL <= v) { _buffer->push_back('0' + (v /          10000000000ULL) % 10); }
            if (          1000000000ULL <= v) { _buffer->push_back('0' + (v /           1000000000ULL) % 10); }
            if (           100000000ULL <= v) { _buffer->push_back('0' + (v /            100000000ULL) % 10); }
            if (            10000000ULL <= v) { _buffer->push_back('0' + (v /             10000000ULL) % 10); }
            if (             1000000ULL <= v) { _buffer->push_back('0' + (v /              1000000ULL) % 10); }
            if (              100000ULL <= v) { _buffer->push_back('0' + (v /               100000ULL) % 10); }
            if (               10000ULL <= v) { _buffer->push_back('0' + (v /                10000ULL) % 10); }
            if (                1000ULL <= v) { _buffer->push_back('0' + (v /                 1000ULL) % 10); }
            if (                 100ULL <= v) { _buffer->push_back('0' + (v /                  100ULL) % 10); }
            if (                  10ULL <= v) { _buffer->push_back('0' + (v /                   10ULL) % 10); }

            _buffer->push_back('0' + (v % 10));
          } 
          else if (slice.isType(JasonType::Int)) {
            int64_t v = slice.getInt();
            if (v < 0) {
              _buffer->push_back('-');
            }
            if (v == INT64_MIN) {
              _buffer->append("9223372036854775808", 19);
              return;
            }
            v = -v;
          
            if (1000000000000000000LL <= v) { _buffer->push_back('0' + (v / 1000000000000000000LL) % 10); }
            if ( 100000000000000000LL <= v) { _buffer->push_back('0' + (v /  100000000000000000LL) % 10); }
            if (  10000000000000000LL <= v) { _buffer->push_back('0' + (v /   10000000000000000LL) % 10); }
            if (   1000000000000000LL <= v) { _buffer->push_back('0' + (v /    1000000000000000LL) % 10); }
            if (    100000000000000LL <= v) { _buffer->push_back('0' + (v /     100000000000000LL) % 10); }
            if (     10000000000000LL <= v) { _buffer->push_back('0' + (v /      10000000000000LL) % 10); }
            if (      1000000000000LL <= v) { _buffer->push_back('0' + (v /       1000000000000LL) % 10); }
            if (       100000000000LL <= v) { _buffer->push_back('0' + (v /        100000000000LL) % 10); }
            if (        10000000000LL <= v) { _buffer->push_back('0' + (v /         10000000000LL) % 10); }
            if (         1000000000LL <= v) { _buffer->push_back('0' + (v /          1000000000LL) % 10); }
            if (          100000000LL <= v) { _buffer->push_back('0' + (v /           100000000LL) % 10); }
            if (           10000000LL <= v) { _buffer->push_back('0' + (v /            10000000LL) % 10); }
            if (            1000000LL <= v) { _buffer->push_back('0' + (v /             1000000LL) % 10); }
            if (             100000LL <= v) { _buffer->push_back('0' + (v /              100000LL) % 10); }
            if (              10000LL <= v) { _buffer->push_back('0' + (v /               10000LL) % 10); }
            if (               1000LL <= v) { _buffer->push_back('0' + (v /                1000LL) % 10); }
            if (                100LL <= v) { _buffer->push_back('0' + (v /                 100LL) % 10); }
            if (                 10LL <= v) { _buffer->push_back('0' + (v /                  10LL) % 10); }

            _buffer->push_back('0' + (v % 10));
          }
          else if (slice.isType(JasonType::SmallInt)) {
            int64_t v = slice.getSmallInt();
            if (v < 0) {
              _buffer->push_back('-');
              v = -v;
            }
            _buffer->push_back('0' + v);
          }
          else {
            throw JasonDumper::JasonDumperError("unexpected number type");
          }
        }

        void dumpString (char const* src, JasonLength len) {
          static char const EscapeTable[256] = {
            //0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
            'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'b', 't', 'n', 'u', 'f', 'r', 'u', 'u', // 00
            'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', // 10
              0,   0, '"',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, '/', // 20
              0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 30~4F
              0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
              0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,'\\',   0,   0,   0, // 50
              0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 60~FF
              0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
              0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
              0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
              0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
              0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
              0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
              0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
              0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
              0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
          };

          uint8_t const* p = reinterpret_cast<uint8_t const*>(src);
          uint8_t const* e = p + len;
          while (p < e) {
            uint8_t c = *p;

            if ((c & 0x80) == 0) {
              // check for control characters
              char esc = EscapeTable[c];

              if (esc) {
                _buffer->push_back('\\');
                _buffer->push_back(static_cast<char>(esc));

                if (esc == 'u') { 
                  uint16_t i1 = (((uint16_t) c) & 0xf0) >> 4;
                  uint16_t i2 = (((uint16_t) c) & 0x0f);

                  _buffer->append("00", 2);
                  _buffer->push_back(static_cast<char>((i1 < 10) ? ('0' + i1) : ('A' + i1 - 10)));
                  _buffer->push_back(static_cast<char>((i2 < 10) ? ('0' + i2) : ('A' + i2 - 10)));
                }
              }
              else {
                _buffer->push_back(static_cast<char>(c));
              }
            }
            else if ((c & 0xe0) == 0xc0) {
              // two-byte sequence
              if (p + 1 >= e) {
                throw JasonDumper::JasonDumperError("unexpected end of string");
              }

              _buffer->append(reinterpret_cast<char const*>(p), 2);
        #if 0
              uint8_t d = *(p + 1);

              if ((d & 0xc0) != 0x80) {
                throw JasonDumper::JasonDumperError("invalid UTF-8 sequence");
              }

              dumpEscapedCharacter(((c & 0x1f) << 6) | (d & 0x3f));
        #endif
              ++p;
            }
            else if ((c & 0xf0) == 0xe0) {
              // three-byte sequence
              if (p + 2 >= e) {
                throw JasonDumper::JasonDumperError("unexpected end of string");
              }

              _buffer->append(reinterpret_cast<char const*>(p), 3);
        #if 0
              uint8_t d = *(p + 1);
              uint8_t e = *(p + 2);

              if ((d & 0xc0) != 0x80 || (e & 0xc0) != 0x80) {
                throw JasonDumper::JasonDumperError("invalid UTF-8 sequence");
              }
            
              dumpEscapedCharacter(((c & 0x0f) << 12) | ((d & 0x3f) << 6) | (e & 0x3f));
        #endif
              p += 2;
            }
            else if ((c & 0xf8) == 0xf0) {
              // four-byte sequence
              if (p + 3 >= e) {
                throw JasonDumper::JasonDumperError("unexpected end of string");
              }

              _buffer->append(reinterpret_cast<char const*>(p), 4);
        #if 0
              uint8_t d = *(p + 1);
              uint8_t e = *(p + 2);
              uint8_t f = *(p + 3);

              if ((d & 0xc0) != 0x80 || (e & 0xc0) != 0x80 || (f & 0xc0) != 0x80) {
                throw JasonDumper::JasonDumperError("invalid UTF-8 sequence");
              }

              uint32_t n = ((c & 0x0f) << 18) | ((d & 0x3f) << 12) | ((e & 0x3f) << 6) | (f & 0x3f);
              n -= 0x10000;
            
              dumpEscapedCharacter(((n & 0xffc00) >> 10) + 0xd800);
              dumpEscapedCharacter((n & 0x3ff) + 0xdc00);
        #endif
              p += 3;
            }

            ++p;
          }
        }

        void dumpEscapedCharacter (uint32_t n) {
          _buffer->reserve(6);

          _buffer->append("\\u", 2);
          dumpHexCharacter((n & 0xf000) >> 12);
          dumpHexCharacter((n & 0x0f00) >>  8);
          dumpHexCharacter((n & 0x00f0) >>  4);
          dumpHexCharacter((n & 0x000f));
        }

        void dumpHexCharacter (uint16_t u) {
          _buffer->append((u < 10) ? ('0' + u) : ('A' + u - 10));
        }

        void handleUnsupportedType (JasonSlice /*slice*/) {
          if (_strategy == STRATEGY_SUPPRESS) {
            return;
          }

          throw JasonDumperError("unsupported type - cannot convert to JSON");
        }

      private:

        T* _buffer;

        UnsupportedTypeStrategy _strategy;

        int _indentation;

    };

    // some alias types for easier usage
    typedef JasonDumper<JasonCharBuffer, false> JasonBufferDumper;
    typedef JasonDumper<std::string, false> JasonStringDumper;
    typedef JasonDumper<std::string, true> JasonStringPrettyDumper;

  }  // namespace triagens::basics
}  // namespace triagens

#endif
