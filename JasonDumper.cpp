
#include "JasonDumper.h"

using JasonDumper = triagens::basics::JasonDumper;
using JasonSlice  = triagens::basics::JasonSlice;
using JasonType   = triagens::basics::JasonType;

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
    case JasonType::String: {
      // JasonLength len;
      // char const* p = slice->getString(len);
      // char const* e = p + len; 
      _buffer->append('"');
      // TODO
      _buffer->append("strings are not yet supported", strlen("strings are not yet supported"));
      _buffer->append('"');
      break;
    }
    case JasonType::StringLong:
      // TODO
      handleUnsupportedType(slice);
      break;
    case JasonType::Binary:
      // TODO
      handleUnsupportedType(slice);
      break;
  }
}
