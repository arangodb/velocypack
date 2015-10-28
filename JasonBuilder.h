////////////////////////////////////////////////////////////////////////////////
/// @brief Library to build up Jason documents.
///
/// @file JasonBuilder.h
///
/// DISCLAIMER
///
/// Copyright 2015 ArangoDB GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Max Neunhoeffer
/// @author Jan Steemann
/// @author Copyright 2015, ArangoDB GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef JASON_BUILDER_H
#define JASON_BUILDER_H

#include <iostream>
#include <vector>
#include <cstring>
#include <cstdint>
#include <algorithm>

#include "Jason.h"
#include "JasonBuffer.h"
#include "JasonSlice.h"
#include "JasonType.h"

namespace arangodb {
  namespace jason {

    class JasonBuilder {

        friend class JasonParser;   // The parser needs access to internals.

      public:

        struct JasonBuilderError : std::exception {
          private:
            std::string _msg;
          public:
            JasonBuilderError (std::string const& msg) : _msg(msg) {
            }
            char const* what() const noexcept {
              return _msg.c_str();
            }
        };

      private:

        // A struct for sorting index tables for objects:

        struct SortEntry {
          uint8_t const* nameStart;
          uint64_t nameSize;
          uint64_t offset;
        };

        // thread local vector for sorting large object attributes
        static thread_local std::vector<SortEntry> SortObjectEntries;

        JasonBuffer<uint8_t> _buffer;  // Here we collect the result
        uint8_t*             _start;   // Always points to the start of _buffer
        JasonLength          _size;    // Always contains the size of _buffer
        JasonLength          _pos;     // the append position, always <= _size
        bool                 _attrWritten;  // indicates that an attribute name
                                            // in an object has been written
        std::vector<JasonLength>              _stack;  // Start positions of
                                                       // open objects/arrays
        std::vector<std::vector<JasonLength>> _index;  // Indices for starts
                                                       // of subindex

        // Here are the mechanics of how this building process works:
        // The whole Jason being built starts at where _start points to
        // and uses at most _size bytes. The variable _pos keeps the
        // current write position. The method "set" simply writes a new
        // Jason subobject at the current write position and advances
        // it. Whenever one makes an array or object, a JasonLength for
        // the beginning of the value is pushed onto the _stack, which
        // remembers that we are in the process of building an array or
        // object. The _index vectors are used to collect information
        // for the index tables of arrays and objects, which are written
        // behind the subvalues. The add methods are used to keep track
        // of the new subvalue in _index followed by a set, and are
        // what the user from the outside calls. The close method seals
        // the innermost array or object that is currently being built
        // and pops a JasonLength off the _stack. The vectors in _index
        // stay until the next clearTemporary() is called to minimize
        // allocations. In the beginning, the _stack is empty, which
        // allows to build a sequence of unrelated Jason objects in the
        // buffer. Whenever the stack is empty, one can use the start,
        // size and stealTo methods to get out the ready built Jason
        // object(s).

        void reserveSpace (JasonLength len) {
          // Reserves len bytes at pos of the current state (top of stack)
          // or throws an exception
          if (_pos + len <= _size) {
            return;  // All OK, we can just increase tos->pos by len
          }
          JasonCheckSize(_pos + len);

          _buffer.prealloc(len);
          _start = _buffer.data();
          _size = _buffer.size();
        }

        // Sort the indices by attribute name:
        static void doActualSort (std::vector<SortEntry>& entries);

        // Find the actual bytes of the attribute name of the Jason value
        // at position base, also determine the length len of the attribute.
        // This takes into account the different possibilities for the format
        // of attribute names:
        static uint8_t const* findAttrName (uint8_t const* base, uint64_t& len);

        static void sortObjectIndexShort (uint8_t* objBase,
                                          std::vector<JasonLength>& offsets);

        static void sortObjectIndexLong (uint8_t* objBase,
                                         std::vector<JasonLength>& offsets);
      public:

        JasonOptions options;

        // Constructor and destructor:

        JasonBuilder ()
          : _buffer({ 0 }),
            _pos(0), 
            _attrWritten(false) {
          _start = _buffer.data();
          _size = _buffer.size();
        }

        // The rule of five:

        ~JasonBuilder () {
        }

        JasonBuilder (JasonBuilder const& that) {
          _buffer = that._buffer;
          _start = _buffer.data();
          _size = _buffer.size();
          _pos = that._pos;
          _attrWritten = that._attrWritten;
          _stack = that._stack;
          _index = that._index;
        }

        JasonBuilder& operator= (JasonBuilder const& that) {
          _buffer = that._buffer;
          _start = _buffer.data();
          _size = _buffer.size();
          _pos = that._pos;
          _attrWritten = that._attrWritten;
          _stack = that._stack;
          _index = that._index;
          return *this;
        }

        JasonBuilder (JasonBuilder&& that) {
          _buffer.reset();
          _buffer = that._buffer;
          that._buffer.reset();
          _start = _buffer.data();
          _size = _buffer.size();
          _pos = that._pos;
          _attrWritten = that._attrWritten;
          _stack.clear();
          _stack.swap(that._stack);
          _index.clear();
          _index.swap(that._index);
          that._start = nullptr;
          that._size = 0;
          that._pos = 0;
          that._attrWritten = false;
        }

        JasonBuilder& operator= (JasonBuilder&& that) {
          _buffer.reset();
          _buffer = that._buffer;
          that._buffer.reset();
          _start = _buffer.data();
          _size = _buffer.size();
          _pos = that._pos;
          _attrWritten = that._attrWritten;
          _stack.clear();
          _stack.swap(that._stack);
          _index.clear();
          _index.swap(that._index);
          that._start = nullptr;
          that._size = 0;
          that._pos = 0;
          that._attrWritten = false;
          return *this;
        }

        // Clear and start from scratch:
        void clear () {
          _pos = 0;
          _attrWritten = false;
          _stack.clear();
        }

        // Return a pointer to the start of the result:
        uint8_t* start () const {
          return _start;
        }

        // Return a JasonSlice of the result:
        JasonSlice slice () const {
          return JasonSlice(_start);
        }

        // Compute the actual size here, but only when sealed
        JasonLength size () const {
          if (! _stack.empty()) {
            throw JasonBuilderError("Jason object not sealed.");
          }
          return _pos;
        }

        // Add a subvalue into an object from a Jason:
        void add (std::string const& attrName, Jason const& sub);

        // Add a subvalue into an object from a JasonPair:
        uint8_t* add (std::string const& attrName, JasonPair const& sub);

        // Add a subvalue into an array from a Jason:
        void add (Jason const& sub);

        // Add a subvalue into an array from a JasonPair:
        uint8_t* add (JasonPair const& sub);

        // Seal the innermost array or object:
        void close ();

        // Syntactic sugar for add:
        JasonBuilder& operator() (std::string const& attrName, Jason sub) {
          add(attrName, sub);
          return *this;
        }

        // Syntactic sugar for add:
        JasonBuilder& operator() (std::string const& attrName, JasonPair sub) {
          add(attrName, sub);
          return *this;
        }

        // Syntactic sugar for add:
        JasonBuilder& operator() (Jason sub) {
          add(sub);
          return *this;
        }

        // Syntactic sugar for add:
        JasonBuilder& operator() (JasonPair sub) {
          add(sub);
          return *this;
        }

        // Syntactic sugar for close:
        JasonBuilder& operator() () {
          close();
          return *this;
        }

      private:

        void addNull () {
          reserveSpace(1);
          _start[_pos++] = 0x01;
        }

        void addFalse () {
          reserveSpace(1);
          _start[_pos++] = 0x02;
        }

        void addTrue () {
          reserveSpace(1);
          _start[_pos++] = 0x03;
        }

        void addDouble (double v) {
          static_assert(sizeof(double) == 8, "double is not 8 bytes");

          uint64_t dv;
          memcpy(&dv, &v, sizeof(double));
          JasonLength vSize = sizeof(double);
          reserveSpace(1 + vSize);
          _start[_pos++] = 0x0e;
          for (uint64_t x = dv; vSize > 0; vSize--) {
            _start[_pos++] = x & 0xff;
            x >>= 8;
          }
        }

        void addInt (int64_t v) {
          if (v >= 0 && v <= 9) {
            reserveSpace(1);
            _start[_pos++] = 0x30 + v;
          }
          else if (v < 0 && v >= -6) {
            reserveSpace(1);
            _start[_pos++] = 0x40 + v;
          }
          appendInt(v, 0x1f);
        }

        void addUInt (uint64_t v) {
          if (v <= 9) {
            reserveSpace(1);
            _start[_pos++] = 0x30 + v;
          }
          else {
            appendUInt(v, 0x27);
          }
        }

        void addUTCDate (int64_t v) {
          uint8_t vSize = sizeof(int64_t);   // is always 8
          uint64_t x = toUInt64(v);
          reserveSpace(1 + vSize);
          _start[_pos++] = 0x0f;
          appendLength(x, 8);
        }

        uint8_t* addString (uint64_t strLen) {
          uint8_t* target;
          if (strLen > 126) {
            // long string
            _start[_pos++] = 0xbf;
            // write string length
            appendLength(strLen, 8);
          }
          else {
            // short string
            _start[_pos++] = 0x40 + strLen;
          }
          target = _start + _pos;
          _pos += strLen;
          return target;
        }

        void addCompoundValue (uint8_t type) {
          reserveSpace(10);
          // an array is started:
          _stack.push_back(_pos);
          while (_stack.size() > _index.size()) {
            _index.emplace_back();
          }
          _index[_stack.size() - 1].clear();
          _start[_pos++] = type;
          _start[_pos++] = 0x00;  // Will be filled later with short bytelength
          _pos += 8;              // Possible space for long bytelength
        }

        void addArray () {
          addCompoundValue(0x05);
        }
          
        void addObject () {
          addCompoundValue(0x08);
        }
 
        void set (Jason const& item);

        uint8_t* set (JasonPair const& pair);
        
        void reportAdd (JasonLength base) {
          size_t depth = _stack.size() - 1;
          _index[depth].push_back(_pos - base);
        }

        void appendLength (JasonLength v, uint64_t n) {
          reserveSpace(n);
          for (uint64_t i = 0; i < n; ++i) {
            _start[_pos++] = v & 0xff;
            v >>= 8;
          }
        }

        void appendUInt (uint64_t v, uint8_t base) {
          reserveSpace(9);
          JasonLength save = _pos++;
          uint8_t vSize = 0;
          do {
            vSize++;
            _start[_pos++] = static_cast<uint8_t>(v & 0xff);
            v >>= 8;
          } while (v != 0);
          _start[save] = base + vSize;
        }

        // returns number of bytes required to store the value in 2s-complement
        static inline uint8_t intLength (int64_t value) {
          if (value >= -0x80 && value <= 0x7f) {
            // shortcut for the common case
            return 1;
          }
          uint64_t x = value >= 0 ? static_cast<uint64_t>(value)
                                  : static_cast<uint64_t>(-(value + 1)) + 1;
          uint8_t xSize = 0;
          do {
            xSize++;
            x >>= 8;
          } 
          while (x >= 0x80);
          return xSize + 1;
        }

        void appendInt (int64_t v, uint8_t base) {
          uint8_t vSize = intLength(v);
          uint64_t x;
          if (vSize == 8) {
            x = toUInt64(v);
          }
          else {
            int64_t shift = 1LL << (vSize * 8 - 1);  // will never overflow!
            x = v >= 0 ? static_cast<uint64_t>(v)
                       : static_cast<uint64_t>(v + shift) + shift;
          }
          reserveSpace(1 + vSize);
          _start[_pos++] = base + vSize;
          while (vSize-- > 0) {
            _start[_pos++] = x & 0xff;
            x >>= 8;
          }
        }
 
        void checkAttributeUniqueness (JasonSlice const obj) const;
    };

  }  // namespace arangodb::jason
}  // namespace arangodb

#endif
