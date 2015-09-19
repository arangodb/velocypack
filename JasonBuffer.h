#ifndef JASON_BUFFER_H
#define JASON_BUFFER_H 1

#include <cstring>
#include "Jason.h"

namespace triagens {
  namespace basics {

    class JasonBuffer {
        JasonBuffer (JasonBuffer const&) = delete;
        JasonBuffer& operator= (JasonBuffer const&) = delete;

      public:

        JasonBuffer () : _buf(nullptr), _alloc(0), _pos(0) {
        } 

        explicit JasonBuffer (JasonLength expectedLength) : JasonBuffer() {
          reserve(expectedLength);
        }

        ~JasonBuffer () {
          if (_buf != nullptr) {
            delete[] _buf;
          }
        }

        char const* data () const {
          return _buf;
        }

        JasonLength size () const {
          return _pos;
        }

        char* steal () {
          char* buf = _buf;
          _buf = nullptr;
          _pos = 0;
          _alloc = 0;
          return buf;
        }

        void append (char c) {
          reserve(_pos + 1); 
          _buf[_pos++] = c;
        }

        void append (char const* p, JasonLength len) {
          reserve(_pos + len);
          memcpy(_buf + _pos, p, len);
          _pos += len;
        }

        void append (uint8_t const* p, JasonLength len) {
          reserve(_pos + len);
          memcpy(_buf + _pos, p, len);
          _pos += len;
        }

        void reserve (JasonLength len) {
          if (_pos + len >= _alloc) {
            static JasonLength const MinLength = 128;
            static double const GrowthFactor = 1.2;

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
              delete[] _buf;
            }
            _buf = p;
            _alloc = newLen;
          }
        }
       
      private:
 
        char*       _buf;
        JasonLength _alloc;
        JasonLength _pos;

    };

  }  // namespace triagens::basics
}  // namespace triagens

#endif
