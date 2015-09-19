
#include "JasonDumper.h"
#include "fpconv.h"

using JasonDumper = triagens::basics::JasonDumper;
using JasonSlice  = triagens::basics::JasonSlice;
using JasonType   = triagens::basics::JasonType;

#define Z16 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

static char const EscapeTable[256] = {
  //0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
  'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'b', 't', 'n', 'u', 'f', 'r', 'u', 'u', // 00
  'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', 'u', // 10
    0,   0, '"',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, '/', // 20
  Z16, Z16,                                                                       // 30~4F
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,'\\',   0,   0,   0, // 50
  Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16, Z16                                // 60~FF
};

void JasonDumper::internalDump (JasonSlice slice) {
  switch (slice.type()) {
    case JasonType::None:
      handleUnsupportedType(slice);
      break;
    case JasonType::Null:
      _buffer->append("null", 4);
      break;
    case JasonType::Bool: 
      if (slice.getBool()) {
        _buffer->append("true", 4);
      } 
      else {
        _buffer->append("false", 5);
      }
      break;
    case JasonType::Double: {
      char temp[24];
      int len = fpconv_dtoa(slice.getDouble(), &temp[0]);
      _buffer->append(&temp[0], static_cast<JasonLength>(len));
      break; 
    }
    case JasonType::Array:
    case JasonType::ArrayLong: {
      JasonLength const n = slice.length();
      _buffer->append('[');
      for (JasonLength i = 0; i < n; ++i) {
        if (i > 0) {
          _buffer->append(',');
        }
        internalDump(slice.at(i));
      }
      _buffer->append(']');
      break;
    }
    case JasonType::Object:
      // TODO
      handleUnsupportedType(slice);
      break;
    case JasonType::ObjectLong:
      // TODO
      handleUnsupportedType(slice);
      break;
    case JasonType::External:
      // TODO
      handleUnsupportedType(slice);
      break;
    case JasonType::ID:
      handleUnsupportedType(slice);
      // TODO
      break;
    case JasonType::ArangoDB_id:
      handleUnsupportedType(slice);
      // TODO
      break;
    case JasonType::UTCDate:
      handleUnsupportedType(slice);
      // TODO
      break;
    case JasonType::Int:
      // TODO
      handleUnsupportedType(slice);
      break;
    case JasonType::UInt:
      // TODO
      break;
    case JasonType::String: 
    case JasonType::StringLong: {
      JasonLength len;
      char const* p = slice.getString(len);
      _buffer->append('"');
      dumpString(p, len);
      _buffer->append('"');
      break;
    }
    case JasonType::Binary:
      // TODO
      handleUnsupportedType(slice);
      break;
  }
}

void JasonDumper::dumpString (char const* src, JasonLength len) {
  uint8_t const* p = reinterpret_cast<uint8_t const*>(src);
  uint8_t const* e = p + len;
  while (p < e) {
    uint8_t c = *p;

    if ((c & 0x80) == 0) {
      // check for control characters
      char esc = EscapeTable[c];

      if (esc) {
        _buffer->append('\\');
        _buffer->append(esc);

        if (esc == 'u') { 
          uint16_t i1 = (((uint16_t) c) & 0xf0) >> 4;
          uint16_t i2 = (((uint16_t) c) & 0x0f);

          _buffer->append("00", 2);
          _buffer->append((i1 < 10) ? ('0' + i1) : ('A' + i1 - 10));
          _buffer->append((i2 < 10) ? ('0' + i2) : ('A' + i2 - 10));
        }
      }
      else {
        _buffer->append(*p);
      }
    }
    else if ((c & 0xe0) == 0xc0) {
      // two-byte sequence
      if (p + 1 >= e) {
        throw JasonDumper::JasonDumperError("unexpected end of string");
      }

      uint8_t d = *(p + 1);

      if ((d & 0xc0) != 0x80) {
        throw JasonDumper::JasonDumperError("invalid UTF-8 sequence");
      }

      dumpEscapedCharacter(((c & 0x1f) << 6) | (d & 0x3f));
      ++p;
    }
    else if ((c & 0xf0) == 0xe0) {
      // three-byte sequence
      if (p + 2 >= e) {
        throw JasonDumper::JasonDumperError("unexpected end of string");
      }

      uint8_t d = *(p + 1);
      uint8_t e = *(p + 2);

      if ((d & 0xc0) != 0x80 || (e & 0xc0) != 0x80) {
        throw JasonDumper::JasonDumperError("invalid UTF-8 sequence");
      }
    
      dumpEscapedCharacter(((c & 0x0f) << 12) | ((d & 0x3f) << 6) | (e & 0x3f));
      p += 2;
    }
    else if ((c & 0xf8) == 0xf0) {
      // four-byte sequence
      if (p + 3 >= e) {
        throw JasonDumper::JasonDumperError("unexpected end of string");
      }

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
      p += 3;
    }

    ++p;
  }
}

void JasonDumper::dumpEscapedCharacter (uint32_t n) {
  _buffer->reserve(6);

  _buffer->append("\\u", 2);
  dumpHexCharacter((n & 0xf000) >> 12);
  dumpHexCharacter((n & 0x0f00) >>  8);
  dumpHexCharacter((n & 0x00f0) >>  4);
  dumpHexCharacter((n & 0x000f));
}

void JasonDumper::dumpHexCharacter (uint16_t u) {
  _buffer->append((u < 10) ? ('0' + u) : ('A' + u - 10));
}

