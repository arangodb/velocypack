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

        friend class JasonParser;

      // This class organizes the buildup of a Jason object. It manages
      // the memory allocation and allows convenience methods to build
      // the object up recursively.
      //
      // Use as follows:                         to build Jason like this:
      //   JasonBuilder b;
      //   b.add(Jason(5, JasonType::Object));    b = {
      //   b.add("a", Jason(1.0));                      "a": 1.0,
      //   b.add("b", Jason());                         "b": null,
      //   b.add("c", Jason(false));                    "c": false,
      //   b.add("d", Jason("xyz"));                    "d": "xyz",
      //   b.add("e", Jason(3, JasonType::Array));      "e": [
      //   b.add(Jason(2.3));                                   2.3,
      //   b.add(Jason("abc"));                                 "abc",
      //   b.add(Jason(true));                                  true
      //   b.close();                                         ],
      //   b.add("f", Jason(2, JasonType::Object));     "f": {
      //   b.add("hans", Jason("Wurst"));                       "hans": "wurst",
      //   b.add("hallo", Jason(3.141));                        "hallo": 3.141
      //   b.close();                                        }
      //
      // Or, if you like fancy syntactic sugar:
      //   JasonBuilder b;
      //   b(Jason(5, JasonType::Object))        b = {
      //    ("a", Jason(1.0))                          "a": 1.0,
      //    ("b", Jason())                             "b": null,
      //    ("c", Jason(false))                        "c": false,
      //    ("d", Jason("xyz"))                        "d": "xyz",
      //    ("e", JasonType::Array, 3)                 "e": [
      //      (Jason(2.3))                                    2.3,
      //      (Jason("abc"))                                 "abc",
      //      (Jason(true))()                                true ],
      //    ("f", JasonType::Object, 2)                "f": {
      //    ("hans", Jason("Wurst"))                          "hans": "wurst",
      //    ("hallo", Jason(3.141)();                         "hallo": 3.141 }

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

        struct SortEntryLarge {
          uint8_t const* nameStart;
          uint64_t nameSize;
          uint64_t offset;
        };

        // thread local vector for sorting large object attributes
        static thread_local std::vector<SortEntryLarge> SortObjectLargeEntries;

        JasonBuffer<uint8_t> _buffer;
        uint8_t*             _start;
        JasonLength          _size;
        JasonLength          _pos;   // the current append position, always <= _size
        bool                 _attrWritten;  // indicates that an attribute name
                                            // in an object has been written
        std::vector<JasonLength>              _stack;
        std::vector<std::vector<JasonLength>> _index;

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

        static void doActualSortLarge (std::vector<SortEntryLarge>& entries);

        static uint8_t const* findAttrName (uint8_t const* base, uint64_t& len);

        static void sortObjectIndexShort (uint8_t* objBase,
                                          std::vector<JasonLength>& offsets);

        static void sortObjectIndexLong (uint8_t* objBase,
                                         std::vector<JasonLength>& offsets);
      public:

        JasonOptions options;

        JasonBuilder ()
          : _buffer({ 0 }),
            _pos(0), 
            _attrWritten(false) {
          _start = _buffer.data();
          _size = _buffer.size();
        }

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

        void clear () {
          _pos = 0;
          _attrWritten = false;
          _stack.clear();
        }

        uint8_t* start () const {
          return _start;
        }

        JasonSlice slice () const {
          return JasonSlice(_start);
        }

        JasonLength size () const {
          // Compute the actual size here, but only when sealed
          if (! _stack.empty()) {
            throw JasonBuilderError("Jason object not sealed.");
          }
          return _pos;
        }

        void add (std::string const& attrName, Jason const& sub);

        uint8_t* add (std::string const& attrName, JasonPair const& sub);

        void add (Jason const& sub);

        uint8_t* add (JasonPair const& sub);

        void close ();

        JasonBuilder& operator() (std::string const& attrName, Jason sub) {
          add(attrName, sub);
          return *this;
        }

        JasonBuilder& operator() (std::string const& attrName, JasonPair sub) {
          add(attrName, sub);
          return *this;
        }

        JasonBuilder& operator() (Jason sub) {
          add(sub);
          return *this;
        }

        JasonBuilder& operator() (JasonPair sub) {
          add(sub);
          return *this;
        }

        JasonBuilder& operator() () {
          close();
          return *this;
        }

        // returns number of bytes required to store the value
        static JasonLength uintLength (uint64_t value) {
          if (value <= 0xff) {
            // shortcut for the common case
            return 1;
          }
          JasonLength vSize = 0;
          do {
            vSize++;
            value >>= 8;
          } 
          while (value != 0);
          return vSize;
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
          uint64_t dv;
          memcpy(&dv, &v, sizeof(double));
          JasonLength vSize = sizeof(double);
          reserveSpace(1 + vSize);
          _start[_pos++] = 0x04;
          for (uint64_t x = dv; vSize > 0; vSize--) {
            _start[_pos++] = x & 0xff;
            x >>= 8;
          }
        }

        void addPosInt (uint64_t v) {
          if (v < 8) {
            reserveSpace(1);
            _start[_pos++] = 0x30 + v;
          }
          else if (v > static_cast<uint64_t>(INT64_MAX)) {
            // value is bigger than INT64_MAX. now save as a Double type
            addDouble(static_cast<double>(v));
          }
          else {
            // value is smaller than INT64_MAX: now save as an Int type
            appendUInt(v, 0x17);
          }
        }

        void addNegInt (uint64_t v) {
          if (v < 9) {
            reserveSpace(1);
            if (v == 0) {
              _start[_pos++] = 0x30;
            }
            else {
              _start[_pos++] = 0x40 - v;
            }
          }
          else if (v > static_cast<uint64_t>(- INT64_MIN)) {
            // value is smaller than INT64_MIN. now save as Double
            addDouble(- static_cast<double>(v));
          }
          else {
            // value is bigger than INT64_MIN. now save as Int
            appendUInt(v, 0x1f);
          }
        }

        void addUInt (uint64_t v) {
          if (v < 8) {
            reserveSpace(1);
            _start[_pos++] = 0x30 + v;
          }
          else {
            appendUInt(v, 0x27);
          }
        }

        void addUTCDate (int64_t v) {
          static_assert(sizeof(int64_t) == sizeof(uint64_t), "invalid int64 size");

          uint64_t dv;
          memcpy(&dv, &v, sizeof(int64_t));
          dv = 1 + (~ dv);
          JasonLength vSize = sizeof(int64_t);
          reserveSpace(1 + vSize);
          _start[_pos++] = 0x0d;
          for (uint64_t x = dv; vSize > 0; vSize--) {
            _start[_pos++] = x & 0xff;
            x >>= 8;
          }
        }

        uint8_t* addString (uint64_t strLen) {
          uint8_t* target;
          if (strLen > 127) {
            // long string
            _start[_pos++] = 0x0c;
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
          addCompoundValue(0x07);
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
          JasonLength vSize = uintLength(v);
          reserveSpace(1 + vSize);
          _start[_pos++] = base + vSize;
          for (uint64_t x = v; vSize > 0; vSize--) {
            _start[_pos++] = x & 0xff;
            x >>= 8;
          }
        }

        void appendInt (int64_t v) {
          if (v >= 0) {
            appendUInt(static_cast<uint64_t>(v), 0x17);
          }
          else {
            appendUInt(static_cast<uint64_t>(-v), 0x1f);
          }
        }
 
        void checkAttributeUniqueness (JasonSlice const obj) const;
    };

  }  // namespace arangodb::jason
}  // namespace arangodb

#endif
