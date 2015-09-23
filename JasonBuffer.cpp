
#include "JasonBuffer.h"

using Jason       = triagens::basics::Jason;
using JasonBuffer = triagens::basics::JasonBuffer;
using JasonLength = triagens::basics::JasonLength;
    
double const JasonBuffer::GrowthFactor = 1.2;

void JasonBuffer::reserve (JasonLength len) {
  if (_pos + len >= _alloc) {
    static JasonLength const MinLength = sizeof(_local);

    // need reallocation
    JasonLength newLen = _pos + len;
    if (newLen < MinLength) {
      // ensure we don't alloc too small blocks
      newLen = MinLength;
    }
    if (_pos > 0 && newLen < GrowthFactor * _pos) {
      // ensure the buffer grows sensibly and not by 1 byte only
      newLen = GrowthFactor * _pos;
    }

    char* p = new char[newLen];

    if (_buf != nullptr) {
      // copy old data
      memcpy(p, _buf, _pos);
      if (_buf != _local) {
        delete[] _buf;
      }
    }
    _buf = p;
    _alloc = newLen;
  }
}

