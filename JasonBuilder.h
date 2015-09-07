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

        std::vector<uint8_t> _alloc;
        bool     _externalMem;          // true if buffer came from the outside
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

        std::vector<State> _stack;   // Has always size() >= 1

        bool _sealed;

        void reserveSpace (size_t len) {
          // Reserves len bytes at pos of the current state (top of stack)
          // or throws an exception
          if (_externalMem) {
            throw std::bad_alloc();
          }
          State& tos = _stack.back();
          if (tos.pos + len <= _size) {
            return;  // All OK, we can just increase tos->pos by len
          }
          _alloc.reserve(tos.pos + len);
          for (size_t i = _size; i < tos.pos + len; i++) {
            _alloc.push_back(0);
          }
          _start = _alloc.data();
          _size = _alloc.size();
        }

      public:

        JasonBuilder () : _externalMem(false), _sealed(false) {
          _alloc.push_back(0);
          _start = _alloc.data();
          _size = _alloc.size();
          _stack.emplace_back(JasonType::None);
        }

        JasonBuilder (JasonType type, size_t spaceHint) 
          : _externalMem(false), _start(nullptr), _size(0) {
        }

        JasonBuilder (uint8_t* start, size_t size) 
          : _externalMem(true), _start(start), _size(size) {
        }
      
        ~JasonBuilder () {
        }

        JasonBuilder (JasonBuilder const& that) {
        }

        JasonBuilder& operator= (JasonBuilder const& that) {
          return *this;
        }

        JasonBuilder (JasonBuilder&& that) {
        }

        JasonBuilder& operator= (JasonBuilder&& that) {
          return *this;
        }

        void setType (JasonType type, bool large = false) {
        }

        size_t seal () {
          return 0ul;
        }

        uint8_t* start () {
          return _start;
        }

        size_t size () {
          return 0ul;
        }

        void add (std::string attrName, Jason sub) {
        }

        void add (Jason sub) {
        }

        void close () {
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
