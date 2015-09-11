#ifndef JASON_BUILDER_H
#define JASON_BUILDER_H

#include "JasonType.h"
#include "Jason.h"

#include <vector>

namespace triagens {
  namespace basics {

    class JasonBuilder {

      // This class organises the buildup of a Jason object. It manages
      // the memory allocation and allows convenience methods to build
      // the object up recursively.
      //
      // Use as follows:                         to build Jason like this:
      //   JasonBuilder b(JasonType::Object, 5);  b = {
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

        struct InternalError : std::exception {
            const char* what() const noexcept {
              return "Internal JasonBuilder error!\n";
            }
        };

        std::vector<uint8_t> _alloc;
        bool     _externalMem;          // true if buffer came from the outside
        bool     _sealed;
        uint8_t* _start;
        size_t   _size;

        struct State {
          JasonType type;
          size_t base;   // Start of object currently being built
          size_t pos;    // Current append position
          size_t index;  // Index in array or object currently being worked on
          bool   large;  // Flag, whether we use the large version

          State (JasonType t) 
            : type(t), base(0), pos(0), index(0), large(false) {
          }

        };

        std::vector<State> _stack;   // Has always size() >= 1 when still
                                     // writable and 0 when sealed

        void reserveSpace (size_t len) {
          // Reserves len bytes at pos of the current state (top of stack)
          // or throws an exception
          if (_stack.size() == 0) {
            throw InternalError();
          }
          if (_externalMem) {
            throw std::bad_alloc();
          }
          State& tos = _stack.back();
          if (tos.pos + len <= _size) {
            return;  // All OK, we can just increase tos->pos by len
          }
          _alloc.reserve(tos.pos + len);
          _alloc.insert(_alloc.end(), len, 0);
          _start = _alloc.data();
          _size = _alloc.size();
        }

      public:

        JasonBuilder (JasonType type = JasonType::None, size_t spaceHint = 1) 
          : _externalMem(false), _sealed(false) {
          _alloc.reserve(spaceHint);
          _alloc.push_back(0);
          _start = _alloc.data();
          _size = _alloc.size();
          _stack.emplace_back(type);
        }

        JasonBuilder (uint8_t* start, size_t size,
                      JasonType type = JasonType::None) 
          : _externalMem(true), _start(start), _size(size) {
          _stack.emplace_back(type);
        }
      
        ~JasonBuilder () {
        }

        JasonBuilder (JasonBuilder const& that)
          : _externalMem(false), _sealed(that._sealed) {
          if (that._externalMem) {
            _alloc.reserve(that._size);
            _alloc.insert(_alloc.end(), that._start, that._start + that._size);
          }
          else {
            _alloc = that._alloc;
          }
          _start = _alloc.data();
          _size = _alloc.size();
          _stack = that._stack;
        }

        JasonBuilder& operator= (JasonBuilder const& that) {
          _externalMem = false;
          _alloc.clear();
          _alloc.reserve(that._size);
          _alloc.insert(_alloc.end(), that._start, that._start + that._size);
          _start = _alloc.data();
          _size = _alloc.size();
          _stack = that._stack;
          _sealed = that._sealed;
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
          _stack.clear();
          _stack.swap(that._stack);
          _sealed = that._sealed;
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
          _stack.clear();
          _stack.swap(that._stack);
          _sealed = that._sealed;
          that._start = nullptr;
          that._size = 0;
          return *this;
        }

        void clear () {
          _stack.clear();
          _stack.emplace_back(JasonType::None);
        }

        uint8_t* start () {
          return _start;
        }

        size_t size () {
          // Compute the actual size here, but only when sealed
          if (_stack.size() > 0) {
            return 0;
          }
          return 0ul;
        }

        void setType (JasonType type, bool large = false) {
          if (_stack.size() == 0) {
            throw std::exception();
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

    };

  }  // namespace triagens::basics
}  // namespace triagens

#endif
