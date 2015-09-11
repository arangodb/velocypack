#ifndef JASON_BUILDER_H
#define JASON_BUILDER_H

#include "JasonType.h"
#include "Jason.h"

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
      //   b.set(Jason(5,(JasonType::Object)));   b = {
      //   b.add("a", Jason(1.0));                      "a": 1.0,
      //   b.add("b", Jason());                         "b": null,
      //   b.add("c", Jason(false));                    "c": false,
      //   b.add("d", Jason("xyz"));                    "d": "xyz",
      //   b.add("e", JasonType::Array, 3);              "e": [
      //   b.add(Jason(2.3));                                   2.3,
      //   b.add(Jason("abc"));                                 "abc",
      //   b.add(Jason(true));                                  true
      //   b.close();                                         ],
      //   b.add("f", JasonType::Object, 2);             "f": {
      //   b.add("hans", Jason("Wurst"));                       "hans": "wurst",
      //   b.add("hallo", Jason(3.141));                        "hallo": 3.141
      //   b.close();                                         }
      //
      // Or, if you like fancy syntactic sugar:
      //   JasonBuilder b(JasonType::Object, 5); b = {
      //   b("a", Jason(1.0))                          "a": 1.0,
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
            JasonBuilderError (std::string msg) : _msg(msg) {
            }
            char const* what() const noexcept {
              return _msg.c_str();
            }
        };

        std::vector<uint8_t> _alloc;
        bool     _externalMem;          // true if buffer came from the outside
        bool     _sealed;
        uint8_t* _start;
        size_t   _size;
        size_t   _pos;   // the current append position, always <= _size

        struct State {
          JasonType type;
          size_t base;   // Start of object currently being built
          size_t index;  // Index in array or object currently being worked on
          bool   large;  // Flag, whether we use the large version

          State (JasonType t) 
            : type(t), base(0), index(0), large(false) {
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

        void reserveSpace (size_t len) {
          // Reserves len bytes at pos of the current state (top of stack)
          // or throws an exception
          if (_pos + len <= _size) {
            return;  // All OK, we can just increase tos->pos by len
          }
          if (_externalMem) {
            throw JasonBuilderError("Cannot allocate more memory.");
          }
          _alloc.reserve(_pos + len);
          _alloc.insert(_alloc.end(), len, 0);
          _start = _alloc.data();
          _size = _alloc.size();
        }

      public:

        JasonBuilder (JasonType type = JasonType::None, size_t spaceHint = 1) 
          : _externalMem(false), _pos(0) {
          _alloc.reserve(spaceHint);
          _alloc.push_back(0);
          _start = _alloc.data();
          _size = _alloc.size();
        }

        JasonBuilder (uint8_t* start, size_t size,
                      JasonType type = JasonType::None) 
          : _externalMem(true), _start(start), _size(size), _pos(0) {
        }
      
        ~JasonBuilder () {
        }

        JasonBuilder (JasonBuilder const& that)
          : _externalMem(false) {
          if (! that._externalMem) {
            _alloc = that._alloc;
          }
          else {
            _alloc.reserve(that._size);
            uint8_t* x = that._start;
            for (size_t i = 0; i < that._size; i++) {
              _alloc.push_back(*x++);
            }
          }
          _start = _alloc.data();
          _size = _alloc.size();
          _pos = that._pos;
          _stack = that._stack;
        }

        JasonBuilder& operator= (JasonBuilder const& that) {
          _externalMem = false;
          if (! that._externalMem) {
            _alloc = that._alloc;
          }
          else {
            _alloc.clear();
            _alloc.reserve(that._size);
            uint8_t* x = that._start;
            for (size_t i = 0; i < that._size; i++) {
              _alloc.push_back(*x++);
            }
          }
          _start = _alloc.data();
          _size = _alloc.size();
          _pos = that._pos;
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
          _stack.clear();
          _stack.swap(that._stack);
          that._start = nullptr;
          that._size = 0;
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
          _stack.clear();
          _stack.swap(that._stack);
          that._start = nullptr;
          that._size = 0;
          return *this;
        }

        void clear () {
          _pos = 0;
          _stack.clear();
        }

        uint8_t* start () {
          return _start;
        }

        size_t size () {
          // Compute the actual size here, but only when sealed
          if (_stack.size() > 0) {
            throw JasonBuilderError("Jason object not sealed.");
          }
          return _pos;
        }

        void stealTo (std::vector<uint8_t>& target) {
          if (_stack.size() > 0) {
            throw JasonBuilderError("Jason object not sealed.");
          }
          if (! _externalMem) {
            target.clear();
            _alloc.swap(target);
            clear();
          }
          else {
            target.clear();
            size_t s = size();
            target.reserve(s);
            uint8_t* x = _start;
            for (size_t i = 0; i < s; i++) {
              target.push_back(*x++);
            }
            clear();
          }
        }

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
              reserveSpace(sizeof(double));
              memcpy(_start + _pos, &v, sizeof(double));
              _pos += sizeof(double);
              break;
            }
            case JasonType::String: {
              if (item.cType() != Jason::CType::String) {
                throw JasonBuilderError("Must give a string for JasonType::String.");
              }
              std::string* s = item.getString();
              size_t size = s->size();
              if (size <= 127) {
                reserveSpace(1+size);
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
              break;
            }

              
            default: {
              throw JasonBuilderError("This JasonType is not yet implemented.");
            }
          }
        }

        void add (std::string attrName, Jason sub) {
        }

        void add (Jason sub) {
        }

        size_t close () {
          return 0; // TODO
        }

        JasonBuilder& operator() (std::string attrName, Jason sub) {
          add(attrName, sub);
          return *this;
        }

        JasonBuilder& operator() (Jason sub) {
          add(sub);
          return *this;
        }

        JasonBuilder& operator() () {
          close();
          return *this;
        }

      private:

        void appendUInt(uint64_t v, uint8_t base) {
          unsigned int vSize = 0;
          uint64_t x = v;
          do {
            vSize++;
            x >>= 8;
          } while (x != 0);
          reserveSpace(1+vSize);
          _start[_pos++] = base + vSize;
          for (x = v; vSize > 0; vSize--) {
            _start[_pos++] = x & 0xff;
            x >>= 8;
          }
        }

        void appendInt(int64_t v) {
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
