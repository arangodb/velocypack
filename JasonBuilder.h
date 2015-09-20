#ifndef JASON_BUILDER_H
#define JASON_BUILDER_H

#include "Jason.h"
#include "JasonType.h"
#include "JasonUtils.h"

#include <vector>
#include <cstring>
           
// Endianess of the system must be configured here:
#undef BIG_ENDIAN
// #define BIG_ENDIAN 1

namespace triagens {
  namespace basics {

    class JasonBuilder {

      // This class organises the buildup of a Jason object. It manages
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

        std::vector<uint8_t> _alloc;
        bool         _externalMem;          // true if buffer came from the outside
        uint8_t*     _start;
        JasonLength  _size;
        JasonLength  _pos;   // the current append position, always <= _size
        bool         _attrWritten;  // indicates that an attribute name
                                    // in an object has been written

        struct State {
          JasonLength base;   // Start of object currently being built
          JasonLength index;  // Index in array or object currently being worked on
          JasonLength len;    // Total length of array or object in entries.

          State (JasonLength b = 0, JasonLength i = 0, JasonLength l = 1)
            : base(b), index(i), len(l) {
          }
        };

        std::vector<State> _stack;   // Has always size() >= 1 when still
                                     // writable and 0 when sealed

        // Here are the mechanics of how this building process works:
        // The whole Jason being built starts at where _start points to
        // and uses at most _size bytes. The variable _pos keeps the 
        // current write position. The method make simply writes a new
        // Jason subobject at the current write position and advances it.
        // Whenever one makes an array or object, a State is pushed onto
        // the _stack, which remembers that we are in the process of building
        // an array or object. If the stack is non-empty, the add methods
        // are used to perform a make followed by keeping track of the 
        // new subobject in the enclosing array or object. The close method
        // seals the innermost array or object that is currently being
        // built and pops a State off the _stack.
        // In the beginning, the _stack is empty, which allows to build
        // a sequence of unrelated Jason objects in the buffer.
        // Whenever the stack is empty, one can use the start, size and
        // stealTo methods to get out the ready built Jason object(s).

        void reserveSpace (JasonLength len) {
          // Reserves len bytes at pos of the current state (top of stack)
          // or throws an exception
          if (_pos + len <= _size) {
            return;  // All OK, we can just increase tos->pos by len
          }
          if (_externalMem) {
            throw JasonBuilderError("Cannot allocate more memory.");
          }
          JasonUtils::CheckSize(_pos + len);
          _alloc.reserve(static_cast<size_t>(_pos + len));

          // fill the (potentially) newly allocated are with zeros
          // TODO: use more optimized method
          for (JasonLength i = 0; i < len; ++i) {
            _alloc.push_back(0);
          }
          _start = _alloc.data();
          _size = _alloc.size();
        }

      public:

        JasonBuilder (JasonType /*type*/ = JasonType::None,
                      JasonLength spaceHint = 1) 
          : _externalMem(false), _pos(0), _attrWritten(false) {
          JasonUtils::CheckSize(spaceHint);
          _alloc.reserve(static_cast<size_t>(spaceHint));
          _alloc.push_back(0);
          _start = _alloc.data();
          _size = _alloc.size();
        }

        JasonBuilder (uint8_t* start, JasonLength size,
                      JasonType /*type*/ = JasonType::None) 
          : _externalMem(true), _start(start), _size(size), _pos(0),
            _attrWritten(false) {
        }
      
        ~JasonBuilder () {
        }

        JasonBuilder (JasonBuilder const& that)
          : _externalMem(false) {
          if (! that._externalMem) {
            _alloc = that._alloc;
          }
          else {
            _alloc.reserve(static_cast<size_t>(that._size));
            uint8_t* x = that._start;
            for (JasonLength i = 0; i < that._size; i++) {
              _alloc.push_back(*x++);
            }
          }
          _start = _alloc.data();
          _size = _alloc.size();
          _pos = that._pos;
          _attrWritten = that._attrWritten;
          _stack = that._stack;
        }

        JasonBuilder& operator= (JasonBuilder const& that) {
          _externalMem = false;
          if (! that._externalMem) {
            _alloc = that._alloc;
          }
          else {
            _alloc.clear();
            _alloc.reserve(static_cast<size_t>(that._size));
            uint8_t* x = that._start;
            for (JasonLength i = 0; i < that._size; i++) {
              _alloc.push_back(*x++);
            }
          }
          _start = _alloc.data();
          _size = _alloc.size();
          _pos = that._pos;
          _attrWritten = that._attrWritten;
          _stack = that._stack;
          return *this;
        }

        JasonBuilder (JasonBuilder&& that) {
          _externalMem = that._externalMem;
          if (_externalMem) {
            _alloc.clear();
            _start = that._start;
            _size = that._size;
          }
          else {
            _alloc.clear();
            _alloc.swap(that._alloc);
            _start = _alloc.data();
            _size = _alloc.size();
          }
          _pos = that._pos;
          _attrWritten = that._attrWritten;
          _stack.clear();
          _stack.swap(that._stack);
          that._start = nullptr;
          that._size = 0;
          that._pos = 0;
          that._attrWritten = false;
        }

        JasonBuilder& operator= (JasonBuilder&& that) {
          _externalMem = that._externalMem;
          if (_externalMem) {
            _alloc.clear();
            _start = that._start;
            _size = that._size;
          }
          else {
            _alloc.clear();
            _alloc.swap(that._alloc);
            _start = _alloc.data();
            _size = _alloc.size();
          }
          _pos = that._pos;
          _attrWritten = that._attrWritten;
          _stack.clear();
          _stack.swap(that._stack);
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

        uint8_t* start () {
          return _start;
        }

        JasonLength size () {
          // Compute the actual size here, but only when sealed
          if (! _stack.empty()) {
            throw JasonBuilderError("Jason object not sealed.");
          }
          return _pos;
        }

        void stealTo (std::vector<uint8_t>& target) {
          if (! _stack.empty()) {
            throw JasonBuilderError("Jason object not sealed.");
          }
          if (! _externalMem) {
            target.clear();
            _alloc.swap(target);
            clear();
          }
          else {
            target.clear();
            JasonLength s = size();
            target.reserve(static_cast<size_t>(s));
            uint8_t* x = _start;
            for (JasonLength i = 0; i < s; i++) {
              target.push_back(*x++);
            }
            clear();
          }
        }

        void add (std::string const& attrName, Jason sub) {
          if (_attrWritten) {
            throw JasonBuilderError("Attribute name already written.");
          }
          if (_stack.empty()) {
            set(Jason(attrName, JasonType::String));
            set(sub);
          }
          else {
            State& tos = _stack.back();
            if (_start[tos.base] != 0x06 &&
                _start[tos.base] != 0x07) {
              throw JasonBuilderError("Need open object for add() call.");
            }
            reportAdd(_pos);
            set(Jason(attrName, JasonType::String));
            set(sub);
          }
        }

        uint8_t* add (std::string const& attrName, JasonPair sub) {
          if (_attrWritten) {
            throw JasonBuilderError("Attribute name already written.");
          }
          uint8_t* ret;
          if (_stack.empty()) {
            set(Jason(attrName, JasonType::String));
            ret = set(sub);
          }
          else {
            State& tos = _stack.back();
            if (_start[tos.base] != 0x06 &&
                _start[tos.base] != 0x07) {
              throw JasonBuilderError("Need open object for add() call.");
            }
            reportAdd(_pos);
            set(Jason(attrName, JasonType::String));
            ret = set(sub);
          }
          return ret;
        }

        void add (Jason sub) {
          if (_stack.empty()) {
            set(sub);
          }
          else {
            bool isObject = false;
            State& tos = _stack.back();
            if (_start[tos.base] < 0x04 ||
                _start[tos.base] > 0x07) {
              throw JasonBuilderError("Need open array or object for add() call.");
            }
            if (_start[tos.base] >= 0x06) {   // object or long object
              if (! _attrWritten &&
                  sub.jasonType() != JasonType::String &&
                  sub.jasonType() != JasonType::StringLong) {
                throw JasonBuilderError("Need open array for this add() call.");
              }
              isObject = true;
            }
            if (isObject) {
              if (! _attrWritten) {
                reportAdd(_pos);
              }
              _attrWritten = ! _attrWritten;
            }
            else {
              reportAdd(_pos);
            }
            set(sub);
          }
        }

        uint8_t* add (JasonPair sub) {
          uint8_t* ret;
          if (_stack.empty()) {
            ret = set(sub);
          }
          else {
            bool isObject = false;
            State& tos = _stack.back();
            if (_start[tos.base] < 0x04 ||
                _start[tos.base] > 0x07) {
              throw JasonBuilderError("Need open array or object for add() call.");
            }
            if (_start[tos.base] >= 0x06) {   // object or long object
              if (! _attrWritten &&
                  sub.jasonType() != JasonType::String &&
                  sub.jasonType() != JasonType::StringLong) {
                throw JasonBuilderError("Need open array for this add() call.");
              }
              isObject = true;
            }
            if (isObject) {
              if (! _attrWritten) {
                reportAdd(_pos);
              }
              _attrWritten = ! _attrWritten;
            }
            else {
              reportAdd(_pos);
            }
            ret = set(sub);
          }
          return ret;
        }

        void close () {
          if (_stack.empty()) {
            throw JasonBuilderError("Need open array or object for close() call.");
          }
          State& tos = _stack.back();
          if (_start[tos.base] < 0x04 || _start[tos.base] > 0x07) {
            throw JasonBuilderError("Need open array or object for close() call.");
          }
          if (tos.index < tos.len) {
            throw JasonBuilderError("Shrinking not yet implemented.");
          }
          if (_start[tos.base] == 0x04 || _start[tos.base] == 0x06) {
            // short array or object:
            JasonLength tableEntry = tos.base + 2;
            JasonLength x = _pos - tos.base;
            if (x >= 65536) {
              throw JasonBuilderError("Offsets too large for small array or object");
            }
            _start[tableEntry] = x & 0xff;
            _start[tableEntry + 1] = (x >> 8) & 0xff;
            if (_start[tos.base] == 0x06) {
#if 0
              tableEntry += 2;
              std::vector<uint16_t> toSort;
              toSort.reserve(tos.index);
              for (JasonLength i = 0; i < tos.index; i++) {
                toSort.push(_start[tableEntry + 2*i] + 
                      static_cast<uint16_t>(_start[tableEntry + 2*i+1]) << 8);
#endif
              // TODO: Sort object entries by key, permute indexes
              ;
            }
          }
          else {
            // long array or object:
            JasonLength tableEntry = tos.base + 8;
            JasonLength x = _pos - tos.base;
            for (size_t i = 0; i < 8; i++) {
              _start[tableEntry + i] = x & 0xff;
              x >>= 8;
            }
          }
          // Now the array or object is complete, we pop a State off the _stack
          _stack.pop_back();
        }

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

        void reserve (JasonLength size) {
          if (_size < size) {
            try {
              reserveSpace(size - _pos);
            }
            catch (...) {
              // Ignore exception here, let it crash later.
            }
          }
        }

      private:

        void set (Jason item) {
          // This method builds a single further Jason item at the current
          // append position. If this is an array or object, then an index
          // table is created and a new State is pushed onto the stack.
          switch (item.jasonType()) {
            case JasonType::None: {
              throw JasonBuilderError("Cannot set a JasonType::None.");
            }
            case JasonType::Null: {
              reserveSpace(1);
              _start[_pos++] = 0x00;
              break;
            }
            case JasonType::Bool: {
              if (item.cType() != Jason::CType::Bool) {
                throw JasonBuilderError("Must give bool for JasonType::Bool.");
              }
              reserveSpace(1);
              if (item.getBool()) {
                _start[_pos++] = 0x02;
              }
              else {
                _start[_pos++] = 0x01;
              }
              break;
            }
            case JasonType::Double: {
              double v = 0.0;
              switch (item.cType()) {
                case Jason::CType::Double:
                  v = item.getDouble();
                  break;
                case Jason::CType::Int64:
                  v = static_cast<double>(item.getInt64());
                  break;
                case Jason::CType::UInt64:
                  v = static_cast<double>(item.getUInt64());
                  break;
                default:
                  throw JasonBuilderError("Must give number for JasonType::Double.");
              }
              reserveSpace(1 + sizeof(double));
              _start[_pos++] = 0x03;
              memcpy(_start + _pos, &v, sizeof(double));
              _pos += sizeof(double);
              break;
            }
            case JasonType::External: {
              if (item.cType() != Jason::CType::VoidPtr) {
                throw JasonBuilderError("Must give void pointer for JasonType::External.");
              }
              reserveSpace(1+sizeof(void*));
              // store pointer. this doesn't need to be portable
              _start[_pos++] = 0x08;
              void const* value = item.getExternal();
              memcpy(_start + _pos, &value, sizeof(void*));
              _pos += sizeof(void*);
              break;
            }
            case JasonType::Int: {
              uint64_t v = 0;
              int64_t vv = 0;
              bool positive = true;
              switch (item.cType()) {
                case Jason::CType::Double:
                  if (item.getDouble() < 0.0) {
                    throw JasonBuilderError("Must give non-negative number for JasonType::UInt.");
                  }
                  vv = static_cast<int64_t>(item.getDouble());
                  if (vv >= 0) {
                    v = static_cast<uint64_t>(vv);
                    positive = true;
                  }
                  else {
                    v = static_cast<uint64_t>(-vv);
                    positive = false;
                  }
                  break;
                case Jason::CType::Int64:
                  vv = item.getInt64();
                  if (vv >= 0) {
                    v = static_cast<uint64_t>(vv);
                    positive = true;
                  }
                  else {
                    v = static_cast<uint64_t>(-vv);
                    positive = false;
                  }
                  break;
                case Jason::CType::UInt64:
                  v = item.getUInt64();
                  positive = true;
                  break;
                default:
                  throw JasonBuilderError("Must give number for JasonType::UInt.");
              }
              JasonLength size = uintLength(v);
              reserveSpace(1 + size);
              if (positive) {
                appendUInt(v, 0x1f);
              }
              else {
                appendUInt(v, 0x27);
              }
              break;
            }
            case JasonType::UInt:
            case JasonType::UTCDate: {
              uint64_t v = 0;
              switch (item.cType()) {
                case Jason::CType::Double:
                  if (item.getDouble() < 0.0) {
                    throw JasonBuilderError("Must give non-negative number for JasonType::UInt.");
                  }
                  v = static_cast<uint64_t>(item.getDouble());
                  break;
                case Jason::CType::Int64:
                  if (item.getInt64() < 0) {
                    throw JasonBuilderError("Must give non-negative number for JasonType::UInt.");
                  }
                  v = static_cast<uint64_t>(item.getInt64());
                  break;
                case Jason::CType::UInt64:
                  v = item.getUInt64();
                  break;
                default:
                  throw JasonBuilderError("Must give number for JasonType::UInt and JasonType::UTCDate.");
              }
              JasonLength size = uintLength(v);
              reserveSpace(1 + size);
              if (item.jasonType() == JasonType::UInt) {
                appendUInt(v, 0x2f);
              }
              else {
                appendUInt(v, 0x0f);
              }
              break;
            }
            case JasonType::String:
            case JasonType::StringLong: {
              if (item.cType() != Jason::CType::String &&
                  item.cType() != Jason::CType::CharPtr) {
                throw JasonBuilderError("Must give a string or char const* for JasonType::String or JasonType::StringLong.");
              }
              std::string const* s;
              std::string value;
              if (item.cType() == Jason::CType::String) {
                s = item.getString();
              }
              else {
                value = item.getCharPtr();
                s = &value;
              }
              size_t size = s->size();
              if (size <= 127) {
                reserveSpace(1 + size);
                _start[_pos++] = 0x40 + size;
                memcpy(_start + _pos, s->c_str(), size);
                _pos += size;
              }
              else {
                appendUInt(size, 0xbf);
                memcpy(_start + _pos, s->c_str(), size);
              }
              break;
            }
            case JasonType::Array: {
              if (item.cType() != Jason::CType::Int64 &&
                  item.cType() != Jason::CType::UInt64) {
                throw JasonBuilderError("Must give an integer for JasonType::Array as length.");
              }
              JasonLength len =   item.cType() == Jason::CType::UInt64 
                                ? item.getUInt64()
                                : static_cast<uint64_t>(item.getInt64());
              if (len >= 256) {
                throw JasonBuilderError("Length in JasonType::Array must be < 256.");
              }
              _stack.emplace_back(_pos, 0, len);
              if (len > 1) {
                reserveSpace(2 + len * 2);  // Offsets needed for indexes 1..
              }
              else {
                reserveSpace(4);   // No offset needed for index 0
              }
              _start[_pos++] = 0x04;
              _start[_pos++] = len & 0xff;
              _start[_pos++] = 0x00;   // these two bytes will be set at the end
              _start[_pos++] = 0x00;
              if (len > 1) {
                for (JasonLength i = 0; i < len-1; i++) {
                  _start[_pos++] = 0x00;
                  _start[_pos++] = 0x00;
                }
              }
              break;
            }
            case JasonType::ArrayLong: {
              if (item.cType() != Jason::CType::Int64 &&
                  item.cType() != Jason::CType::UInt64) {
                throw JasonBuilderError("Must give an integer for JasonType::ArrayLong as length.");
              }
              JasonLength len =   item.cType() == Jason::CType::UInt64 
                                ? item.getUInt64()
                                : static_cast<uint64_t>(item.getInt64());
              if (len == 0) {
                throw JasonBuilderError("Cannot create empty ArrayLong.");
              }
              if (len >= 0x100000000000000) {
                throw JasonBuilderError("Length in JasonType::Array must be < 2^56.");
              }
              _stack.emplace_back(_pos, 0, len);
              if (len > 1) {
                reserveSpace(8 + len * 8);
              }
              else {
                reserveSpace(16);
              }
              // type
              _start[_pos++] = 0x05; 
              // length
              JasonLength temp = len;
              for (size_t i = 0; i < 7; i++) {
                _start[_pos++] = temp & 0xff;
                temp >>= 8;
              }
              // offsets
              if (len > 1) {
                memset(_start + _pos, 0x00, (len-1) * 8);
                _pos += (len-1) * 8;
              }
              break;
            }
            case JasonType::Object: {
              if (item.cType() != Jason::CType::Int64 &&
                  item.cType() != Jason::CType::UInt64) {
                throw JasonBuilderError("Must give an integer for JasonType::Object as length.");
              }
              JasonLength len =   item.cType() == Jason::CType::UInt64 
                                ? item.getUInt64()
                                : static_cast<uint64_t>(item.getInt64());
              if (len >= 256) {
                throw JasonBuilderError("Length in JasonType::Object must be < 256.");
              }
              _stack.emplace_back(_pos, 0, len);
              reserveSpace(2 + (len+1) * 2);
              _start[_pos++] = 0x06;
              _start[_pos++] = len & 0xff;
              _start[_pos++] = 0x00;   // these two bytes will be set at the end
              _start[_pos++] = 0x00;
              if (len > 0) {
                for (JasonLength i = 0; i < len; i++) {
                  _start[_pos++] = 0x00;  // this offset is set with each item
                  _start[_pos++] = 0x00;
                }
              }
              break;
            }
            case JasonType::ObjectLong: {
              if (item.cType() != Jason::CType::Int64 &&
                  item.cType() != Jason::CType::UInt64) {
                throw JasonBuilderError("Must give an integer for JasonType::ObjectLong as length.");
              }
              JasonLength len =   item.cType() == Jason::CType::UInt64 
                                ? item.getUInt64()
                                : static_cast<uint64_t>(item.getInt64());
              if (len == 0) {
                throw JasonBuilderError("Cannot create empty ObjectLong.");
              }
              if (len >= 0x100000000000000) {
                throw JasonBuilderError("Length in JasonType::ObjectLong must be < 2^56.");
              }
              _stack.emplace_back(_pos, 0, len);
              reserveSpace(8 + (len+1) * 8);
              // type
              _start[_pos++] = 0x07; 
              // length
              JasonLength temp = len;
              for (size_t i = 0; i < 7; i++) {
                _start[_pos++] = temp & 0xff;
                temp >>= 8;
              }
              // offsets
              memset(_start + _pos, 0x00, (len+1) * 8);
              _pos += (len+1) * 8;
              break;
            }
            case JasonType::Binary: {
              uint64_t v = 0;
              if (item.cType() != Jason::CType::UInt64) {
                throw JasonBuilderError("Must give unsigned integer for length of binary blob.");
              }
              v = item.getUInt64();
              JasonLength size = uintLength(v);
              reserveSpace(1 + size + v);
              appendUInt(v, 0xcf);
              _pos += v;
              break;
            }
            case JasonType::ArangoDB_id: {
              reserveSpace(1);
              _start[_pos++] = 0x0a;
              break;
            }
            case JasonType::ID: {
              throw JasonBuilderError("Need a JasonPair to build a JasonType::ID.");
            }
          }
        }

        uint8_t* set (JasonPair pair) {
          // This method builds a single further Jason item at the current
          // append position. This is the case for JasonType::ID or
          // JasonType::Binary, which both need two pieces of information
          // to build.
          if (pair.jasonType() == JasonType::ID) {
            reserveSpace(1);
            _start[_pos++] = 0x09;
            set(Jason(pair.getSize(), JasonType::UInt));
            set(Jason(reinterpret_cast<char const*>(pair.getStart()),
                      JasonType::String));
            return nullptr;  // unused here
          }
          else if (pair.jasonType() == JasonType::Binary) {
            uint64_t v = pair.getSize();
            JasonLength size = uintLength(v);
            reserveSpace(1 + size + v);
            appendUInt(v, 0xcf);
            memcpy(_start + _pos, pair.getStart(), v);
            _pos += v;
            return nullptr;  // unused here
          }
          else if (pair.jasonType() == JasonType::String ||
                   pair.jasonType() == JasonType::StringLong) {
            uint64_t size = pair.getSize();
            if (size > 127) {
              JasonLength lenSize = uintLength(size);
              reserveSpace(1 + lenSize + size);
              appendUInt(size, 0xbf);
              _pos += size;
            }
            else {
              reserveSpace(1 + size);
              _start[_pos++] = 0x40 + size;
              _pos += size;
            }
            // Note that the data is not filled in! It is the responsibility
            // of the caller to fill in 
            //   _start + _pos - size .. _start + _pos - 1
            // with valid UTF-8!
            return _start + _pos - size;
          }
          else {
            throw JasonBuilderError("Only JasonType::ID, JasonType::Binary, JasonType::String and JasonType::StringLong are valid for JasonPair argument.");
          }
        }

        void reportAdd (JasonLength itemStart) {
          State& tos = _stack.back();
          if (tos.index >= tos.len) {
            throw JasonBuilderError("Open array or object is already full.");
          }
          if (_start[tos.base] == 0x04) {
            // short array:
            if (_pos - tos.base > 0xffff) {
              throw JasonBuilderError("Short array has grown too long (>0xffff).");
            }
            if (tos.index > 0) {
              JasonLength tableEntry = tos.base + 4 + (tos.index - 1) * 2;
              JasonLength x = itemStart - tos.base;
              _start[tableEntry] = x & 0xff;
              _start[tableEntry + 1] = (x >> 8) & 0xff;
            }
          }
          else if (_start[tos.base] == 0x05) {
            // long array:
            if (tos.index > 0) {
              JasonLength tableEntry = tos.base + 16 + (tos.index - 1) * 8;
              JasonLength x = itemStart - tos.base;
              for (size_t i = 0; i < 8; i++) {
                _start[tableEntry + i] = x & 0xff;
                x >>= 8;
              }
            }
          }
          else if (_start[tos.base] == 0x06) {
            // short object
            if (_pos - tos.base > 0xffff) {
              throw JasonBuilderError("Short object has grown too long (>0xffff).");
            }
            JasonLength tableEntry = tos.base + 4 + tos.index * 2;
            JasonLength x = itemStart - tos.base;
            _start[tableEntry] = x & 0xff;
            _start[tableEntry + 1] = (x >> 8) & 0xff;
          }
          else if (_start[tos.base] == 0x07) {
            // long object
            JasonLength tableEntry = tos.base + 16 + tos.index * 8;
            JasonLength x = itemStart - tos.base;
            for (size_t i = 0; i < 8; i++) {
              _start[tableEntry + i] = x & 0xff;
              x >>= 8;
            }
          }
          else {
            throw JasonBuilderError("Internal error, stack state does not point to object or array.");
          }
          tos.index++;
        }
        
        // returns number of bytes required to store the value
        JasonLength uintLength (uint64_t value) const {
          JasonLength vSize = 0;
          do {
            vSize++;
            value >>= 8;
          } 
          while (value != 0);
          return vSize;
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
            appendUInt(static_cast<uint64_t>(v), 0x1f);
          }
          else {
            appendUInt(static_cast<uint64_t>(-v), 0x27);
          }
        }

    };

  }  // namespace triagens::basics
}  // namespace triagens

#endif
