#ifndef JASON_BUFFER_H
#define JASON_BUFFER_H 1

#include <cstring>
#include <iostream>

#include "Jason.h"

namespace triagens {
  namespace basics {

    template<typename T>
    class JasonBuffer {

      public:

        JasonBuffer () 
          : _buffer(_local), 
            _alloc(sizeof(_local)), 
            _pos(0) {
#ifdef JASON_DEBUG
          memset(_buffer, 0x0a, _alloc);
#endif
        } 

        explicit JasonBuffer (JasonLength expectedLength) 
          : JasonBuffer() {
          reserve(expectedLength);
        }

        JasonBuffer (JasonBuffer const& that)
          : JasonBuffer() {
          
          if (that._pos > 0) {
            if (that._pos > sizeof(_local)) {
              _buffer = new T[that._pos];
            }
            memcpy(_buffer, that._buffer, that._pos);
            _alloc = that._pos;
            _pos = that._pos;
          }
        }
        
        JasonBuffer& operator= (JasonBuffer const& that) {
          if (this != &that) {
            reset();

            if (that._pos > 0) {
              if (that._pos > sizeof(_local)) {
                _buffer = new T[that._pos];
              }
              memcpy(_buffer, that._buffer, that._pos);
              _alloc = that._pos;
              _pos = that._pos;
            }
          }
          return *this;
        }

        JasonBuffer (JasonBuffer&& that)
          : JasonBuffer() {
          
          if (that._buffer == that._local) {
            memcpy(_buffer, that._buffer, that._pos);
            _pos = that._pos;
            that._pos = 0;
          }
          else {
            _buffer = that._buffer;
            _alloc = that._alloc;
            _pos = that._pos;
            that._buffer = that._local;
            that._alloc = sizeof(that._local);
            that._pos = 0;
          }
        }

        ~JasonBuffer () {
          reset();
        }

        T* data () {
          return _buffer;
        }

        T const* data () const {
          return _buffer;
        }

        JasonLength size () const {
          return _pos;
        }

        JasonLength capacity () const {
          return _alloc;
        }

        void reset () noexcept {
          if (_buffer != _local) {
            delete[] _buffer;
            _buffer = _local;
            _alloc = sizeof(_local);
#ifdef JASON_DEBUG
            memset(_buffer, 0x0a, _alloc);
#endif
          }
          _pos = 0;
        }

        void push_back (char c) {
          reserve(_pos + 1); 
          _buffer[_pos++] = c;
        }

        void append (char c) {
          reserve(_pos + 1); 
          _buffer[_pos++] = c;
        }

        void append (char const* p, JasonLength len) {
          reserve(_pos + len);
          memcpy(_buffer + _pos, p, len);
          _pos += len;
        }

        void append (uint8_t const* p, JasonLength len) {
          reserve(_pos + len);
          memcpy(_buffer + _pos, p, len);
          _pos += len;
        }

        void reserve (JasonLength len) {
          if (_pos + len < _alloc) {
            return;
          }

          JASON_ASSERT(_pos + len >= sizeof(_local));

          static JasonLength const MinLength = sizeof(_local);

          // need reallocation
          JasonLength newLen = _pos + len;
          if (newLen < MinLength) {
            // ensure we don't alloc too small blocks
            newLen = MinLength;
          }
          static double const GrowthFactor = 1.2;
          if (_pos > 0 && newLen < GrowthFactor * _pos) {
            // ensure the buffer grows sensibly and not by 1 byte only
            newLen = GrowthFactor * _pos;
          }
          JASON_ASSERT(newLen > _pos);

          T* p = new T[newLen];
#ifdef JASON_DEBUG
          memset(p, 0x0a, newLen);
#endif
          // copy old data
          memcpy(p, _buffer, _pos);
          if (_buffer != _local) {
            delete[] _buffer;
          }
          _buffer = p;
          _alloc = newLen;
        }

        // reserve and zero fill
        void prealloc (JasonLength len) {
          reserve(len);
          memset(_buffer + _pos, 0, len);
          _pos += len;
        }
       
      private:
 
        T*          _buffer;
        JasonLength _alloc;
        JasonLength _pos;

        // an already initialized space for small values
        T           _local[192];

    };

    typedef JasonBuffer<char> JasonCharBuffer;

  }  // namespace triagens::basics
}  // namespace triagens

#endif
