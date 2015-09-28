#ifndef JASON_BUILDER_H
#define JASON_BUILDER_H

#include "Jason.h"
#include "JasonSlice.h"
#include "JasonType.h"
#include "JasonUtils.h"

#include <vector>
#include <cstring>
#include <algorithm>
           
// Endianess of the system must be configured here:
#undef BIG_ENDIAN
// #define BIG_ENDIAN 1

namespace triagens {
  namespace basics {

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

        struct SortEntrySmall {
          int32_t  nameStartOffset;
          uint16_t nameSize;
          uint16_t offset;
        };

        struct SortEntryLarge {
          uint8_t const* nameStart;
          uint64_t nameSize;
          uint64_t offset;
        };

        // thread local vector for sorting small object attributes
        static thread_local std::vector<SortEntrySmall> SortObjectSmallEntries;
        // thread local vector for sorting large object attributes
        static thread_local std::vector<SortEntryLarge> SortObjectLargeEntries;

        std::vector<uint8_t> _alloc;
        uint8_t*       _start;
        JasonLength    _size;
        JasonLength    _pos;   // the current append position, always <= _size
        bool           _externalMem;          // true if buffer came from the outside
        bool           _attrWritten;  // indicates that an attribute name
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

        // Here comes infrastructure to sort object index tables:
        static void doActualSortSmall (std::vector<SortEntrySmall>& entries,
                                       uint8_t const* objBase) {
          assert(entries.size() > 1);
          std::sort(entries.begin(), entries.end(), [objBase] (SortEntrySmall const& a, SortEntrySmall const& b) {
            // return true iff a < b:
            uint8_t const* pa = objBase + a.nameStartOffset;
            uint16_t sizea = a.nameSize;
            uint8_t const* pb = objBase + b.nameStartOffset;
            uint16_t sizeb = b.nameSize;
            size_t const compareLength = static_cast<size_t>((std::min)(sizea, sizeb));
            int res = memcmp(pa, pb, compareLength);

            return (res < 0 || (res == 0 && sizea < sizeb));
          });
        }

        static void doActualSortLarge (std::vector<SortEntryLarge>& entries) {
          assert(entries.size() > 1);
          std::sort(entries.begin(), entries.end(), [] (SortEntryLarge const& a, SortEntryLarge const& b) {
            // return true iff a < b:
            uint8_t const* pa = a.nameStart;
            uint64_t sizea = a.nameSize;
            uint8_t const* pb = b.nameStart;
            uint64_t sizeb = b.nameSize;
            size_t const compareLength = static_cast<size_t>((std::min)(sizea, sizeb));
            int res = memcmp(pa, pb, compareLength);

            return (res < 0 || (res == 0 && sizea < sizeb));
          });
        };

        static uint8_t const* findAttrName (uint8_t const* base, uint64_t& len) {
          uint8_t const b = *base;
          if (b >= 0x40 && b <= 0xbf) {
            len = b - 0x40;
            return base + 1;
          }
          if (b >= 0xc0 && b <= 0xcf) {
            len = 0;
            uint8_t lenLen = b - 0xbf;
            for (uint8_t i = 1; i <= b - 0xbf; i++) {
              len = (len << 8) + base[i];
            }
            return base + lenLen + 1;
          }
          throw JasonBuilderError("Unimplemented attribute name type.");
        }

        static void sortObjectIndexShort (uint8_t* objBase, JasonLength len) {
          // We know that objBase[0] is 0x06 and objBase[1] is len
          // and that len >= 2 and that the table is there, it only needs
          // to be sorted.

          bool alert = false;   // is set to true when we find an attribute
                                // name longer than 0xffff
          std::vector<SortEntrySmall>& entries = SortObjectSmallEntries; 
          entries.clear();
          entries.reserve(len);
          for (JasonLength i = 0; i < len; i++) {
            SortEntrySmall e;
            e.offset =  static_cast<uint16_t>(objBase[4 + 2 * i]) +
                       (static_cast<uint16_t>(objBase[5 + 2 * i]) << 8);
            uint64_t attrLen;
            uint8_t const* nameStart = findAttrName(objBase + e.offset, attrLen);
            if (attrLen <= 0xffff && nameStart - objBase <= 0xffff) {
              e.nameStartOffset = static_cast<uint16_t>(nameStart - objBase);
              e.nameSize = static_cast<uint16_t>(attrLen);
            }
            else {
              alert = true;
              break;
            }
            entries.push_back(e);
          }
          if (! alert) {
            doActualSortSmall(entries, objBase);
            // And now write info back:
            for (JasonLength i = 0; i < len; i++) {
              objBase[4 + 2 * i] = entries[i].offset & 0xff;
              objBase[5 + 2 * i] = entries[i].offset >> 8;
            }
            return;
          }
          // If we get here, we have to start over, because an attribute name
          // was too long:
          std::vector<SortEntryLarge>& entries2 = SortObjectLargeEntries; 
          entries2.clear();
          entries2.reserve(len);
          for (JasonLength i = 0; i < len; i++) {
            SortEntryLarge e;
            e.offset =  static_cast<uint64_t>(objBase[4 + 2 * i]) +
                       (static_cast<uint64_t>(objBase[5 + 2 * i]) << 8);
            e.nameStart = findAttrName(objBase + e.offset, e.nameSize);
            entries2.push_back(e);
          }
          doActualSortLarge(entries2);
          // And now write info back:
          for (JasonLength i = 0; i < len; i++) {
            objBase[4 + 2 * i] = entries[i].offset & 0xff;
            objBase[5 + 2 * i] = entries[i].offset >> 8;
          }
        }

        static void sortObjectIndexLong (uint8_t* objBase, JasonLength len) {
          std::vector<SortEntryLarge>& entries = SortObjectLargeEntries; 
          entries.clear();
          entries.reserve(len);
          for (JasonLength i = 0; i < len; i++) {
            JasonLength const pos = 16 + 8 * i;
            SortEntryLarge e;
            e.offset = (static_cast<uint64_t>(objBase[pos])) +
                       (static_cast<uint64_t>(objBase[pos + 1]) << 8) +
                       (static_cast<uint64_t>(objBase[pos + 2]) << 16) +
                       (static_cast<uint64_t>(objBase[pos + 3]) << 24) +
                       (static_cast<uint64_t>(objBase[pos + 4]) << 32) +
                       (static_cast<uint64_t>(objBase[pos + 5]) << 40) +
                       (static_cast<uint64_t>(objBase[pos + 6]) << 48) +
                       (static_cast<uint64_t>(objBase[pos + 7]) << 56);
            e.nameStart = findAttrName(objBase + e.offset, e.nameSize);
            entries.push_back(e);
          }
          doActualSortLarge(entries);
          // And now write info back:
          for (JasonLength i = 0; i < len; i++) {
            JasonLength const pos = 16 + 8 * i;
            objBase[pos]     = (entries[i].offset)       & 0xff;
            objBase[pos + 1] = (entries[i].offset >> 8)  & 0xff;
            objBase[pos + 2] = (entries[i].offset >> 16) & 0xff;
            objBase[pos + 3] = (entries[i].offset >> 24) & 0xff;
            objBase[pos + 4] = (entries[i].offset >> 32) & 0xff;
            objBase[pos + 5] = (entries[i].offset >> 40) & 0xff;
            objBase[pos + 6] = (entries[i].offset >> 48) & 0xff;
            objBase[pos + 7] = (entries[i].offset >> 56);
          }
        }

      public:

        JasonOptions options;

        JasonBuilder (JasonType /*type*/ = JasonType::None,
                      JasonLength spaceHint = 1) 
          : _pos(0), _externalMem(false), _attrWritten(false) {
          JasonUtils::CheckSize(spaceHint);
          _alloc.reserve(static_cast<size_t>(spaceHint));
          _alloc.push_back(0);
          _start = _alloc.data();
          _size = _alloc.size();
        }

        JasonBuilder (uint8_t* start, JasonLength size,
                      JasonType /*type*/ = JasonType::None) 
          : _start(start), _size(size), _pos(0),
            _externalMem(true), _attrWritten(false) {
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

        uint8_t* start () const {
          return _start;
        }

        JasonLength size () const {
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
          target.clear();
          if (! _externalMem) {
            _alloc.swap(target);
          }
          else {
            JasonLength s = size();
            target.reserve(static_cast<size_t>(s));
            uint8_t* x = _start;
            for (JasonLength i = 0; i < s; i++) {
              target.push_back(*x++);
            }
          }
          clear();
        }

        void add (std::string const& attrName, Jason const& sub) {
          if (_attrWritten) {
            throw JasonBuilderError("Attribute name already written.");
          }
          if (! _stack.empty()) {
            State& tos = _stack.back();
            if (_start[tos.base] != 0x06 &&
                _start[tos.base] != 0x07) {
              throw JasonBuilderError("Need open object for add() call.");
            }
            reportAdd();
          }
          set(Jason(attrName, JasonType::String));
          set(sub);
        }

        uint8_t* add (std::string const& attrName, JasonPair const& sub) {
          if (_attrWritten) {
            throw JasonBuilderError("Attribute name already written.");
          }
          if (! _stack.empty()) {
            State& tos = _stack.back();
            if (_start[tos.base] != 0x06 &&
                _start[tos.base] != 0x07) {
              throw JasonBuilderError("Need open object for add() call.");
            }
            reportAdd();
          }
          set(Jason(attrName, JasonType::String));
          return set(sub);
        }

        void add (Jason const& sub) {
          if (! _stack.empty()) {
            bool isObject = false;
            State& tos = _stack.back();
            if (_start[tos.base] < 0x04 ||
                _start[tos.base] > 0x07) {
              throw JasonBuilderError("Need open array or object for add() call.");
            }
            if (_start[tos.base] >= 0x06) {   // object or long object
              if (! _attrWritten && ! sub.isString()) {
                throw JasonBuilderError("Need open array for this add() call.");
              }
              isObject = true;
            }
            if (isObject) {
              if (! _attrWritten) {
                reportAdd();
              }
              _attrWritten = ! _attrWritten;
            }
            else {
              reportAdd();
            }
          }
          set(sub);
        }

        uint8_t* add (JasonPair const& sub) {
          if (! _stack.empty()) {
            bool isObject = false;
            State& tos = _stack.back();
            if (_start[tos.base] < 0x04 ||
                _start[tos.base] > 0x07) {
              throw JasonBuilderError("Need open array or object for add() call.");
            }
            if (_start[tos.base] >= 0x06) {   // object or long object
              if (! _attrWritten && ! sub.isString()) {
                throw JasonBuilderError("Need open array for this add() call.");
              }
              isObject = true;
            }
            if (isObject) {
              if (! _attrWritten) {
                reportAdd();
              }
              _attrWritten = ! _attrWritten;
            }
            else {
              reportAdd();
            }
          }
          return set(sub);
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
            if (_start[tos.base] == 0x06 && tos.len >= 2) {
              // sort object attributes
              sortObjectIndexShort(_start + tos.base, tos.len);
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
            if (_start[tos.base] == 0x07 && tos.len >= 2) {
              // sort object attributes
              sortObjectIndexLong(_start + tos.base, tos.len);
            }
          }

          if (options.checkAttributeUniqueness && tos.len > 1 &&
              (_start[tos.base] == 0x06 || _start[tos.base] == 0x07)) {
            checkAttributeUniqueness(JasonSlice(_start + tos.base));
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

        // returns number of bytes required to store the value
        static JasonLength uintLength (uint64_t value) {
          if (value <= 0xff) {
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
          _start[_pos++] = 0x00;
        }

        void addFalse () {
          _start[_pos++] = 0x01;
        }

        void addTrue () {
          _start[_pos++] = 0x02;
        }

        void addDouble (double v) {
          _start[_pos++] = 0x03;
          memcpy(_start + _pos, &v, sizeof(double));
          _pos += sizeof(double);
        }

        void addPosInt (uint64_t v) {
          appendUIntNoCheck(v, 0x1f);
        }

        void addNegInt (uint64_t v) {
          appendUIntNoCheck(v, 0x27);
        }

        void addUInt (uint64_t v) {
          appendUIntNoCheck(v, 0x2f);
        }

        void addUTCDate (uint64_t v) {
          appendUIntNoCheck(v, 0x0f);
        }

        uint8_t* addString (uint64_t strLen) {
          uint8_t* target;
          if (strLen > 127) {
            appendUIntNoCheck(strLen, 0xbf);
            target = _start + _pos;
            _pos += strLen;
          }
          else {
            _start[_pos++] = 0x40 + strLen;
            target = _start + _pos;
            _pos += strLen;
          }
          return target;
        }

        void addArray (int64_t len) {
          // if negative, then a long array is made
          if (len < 0) {
            JasonLength temp = static_cast<JasonLength>(-len);
            _stack.emplace_back(_pos, 0, temp);
            // type
            _start[_pos++] = 0x05; 
            // length
            for (size_t i = 0; i < 7; i++) {
              _start[_pos++] = temp & 0xff;
              temp >>= 8;
            }
            // offsets
            if (len > 1) {
              memset(_start + _pos, 0x00, (len - 1) * 8);
              _pos += (len - 1) * 8;
            }
          }
          else {
            // small array
            JasonLength temp = static_cast<JasonLength>(len);
            _stack.emplace_back(_pos, 0, temp);
            _start[_pos++] = 0x04;
            _start[_pos++] = temp & 0xff;
            _start[_pos++] = 0x00;   // these two bytes will be set at the end
            _start[_pos++] = 0x00;
            if (temp > 1) {
              for (JasonLength i = 0; i < temp-1; i++) {
                _start[_pos++] = 0x00;
                _start[_pos++] = 0x00;
              }
            }
          }
        }
          
        void addObject (int64_t len) {
          // if negative, then a long object is made
          if (len < 0) {
            JasonLength temp = static_cast<JasonLength>(-len);
            _stack.emplace_back(_pos, 0, temp);
            // type
            _start[_pos++] = 0x07; 
            // length
            for (size_t i = 0; i < 7; i++) {
              _start[_pos++] = temp & 0xff;
              temp >>= 8;
            }
            // offsets
            memset(_start + _pos, 0x00, (len+1) * 8);
            _pos += (len+1) * 8;
          }
          else {
            // small object
            JasonLength temp = static_cast<JasonLength>(len);
            _stack.emplace_back(_pos, 0, temp);
            _start[_pos++] = 0x06;
            _start[_pos++] = temp & 0xff;
            _start[_pos++] = 0x00;   // these two bytes will be set at the end
            _start[_pos++] = 0x00;
            if (temp > 0) {
              for (JasonLength i = 0; i < temp; i++) {
                _start[_pos++] = 0x00;
                _start[_pos++] = 0x00;
              }
            }
          }
        }
 
        void set (Jason const& item) {
          auto ctype = item.cType();

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
              if (ctype != Jason::CType::Bool) {
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
              switch (ctype) {
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
              if (ctype != Jason::CType::VoidPtr) {
                throw JasonBuilderError("Must give void pointer for JasonType::External.");
              }
              reserveSpace(1 + sizeof(void*));
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
              switch (ctype) {
                case Jason::CType::Double:
                  if (item.getDouble() < 0.0) {
                    throw JasonBuilderError("Must give non-negative number for JasonType::UInt.");
                  }
                  vv = static_cast<int64_t>(item.getDouble());
                  if (vv >= 0) {
                    v = static_cast<uint64_t>(vv);
                    // positive is already set to true
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
                    // positive is already set to true
                  }
                  else {
                    v = static_cast<uint64_t>(-vv);
                    positive = false;
                  }
                  break;
                case Jason::CType::UInt64:
                  v = item.getUInt64();
                  // positive is already set to true
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
              switch (ctype) {
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
              if (ctype != Jason::CType::String &&
                  ctype != Jason::CType::CharPtr) {
                throw JasonBuilderError("Must give a string or char const* for JasonType::String or JasonType::StringLong.");
              }
              std::string const* s;
              std::string value;
              if (ctype == Jason::CType::String) {
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
              if (ctype != Jason::CType::Int64 &&
                  ctype != Jason::CType::UInt64) {
                throw JasonBuilderError("Must give an integer for JasonType::Array as length.");
              }
              JasonLength len =   ctype == Jason::CType::UInt64 
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
              if (ctype != Jason::CType::Int64 &&
                  ctype != Jason::CType::UInt64) {
                throw JasonBuilderError("Must give an integer for JasonType::ArrayLong as length.");
              }
              JasonLength len =   ctype == Jason::CType::UInt64 
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
                memset(_start + _pos, 0x00, (len - 1) * 8);
                _pos += (len - 1) * 8;
              }
              break;
            }
            case JasonType::Object: {
              if (ctype != Jason::CType::Int64 &&
                  ctype != Jason::CType::UInt64) {
                throw JasonBuilderError("Must give an integer for JasonType::Object as length.");
              }
              JasonLength len =   ctype == Jason::CType::UInt64 
                                ? item.getUInt64()
                                : static_cast<uint64_t>(item.getInt64());
              if (len >= 256) {
                throw JasonBuilderError("Length in JasonType::Object must be < 256.");
              }
              _stack.emplace_back(_pos, 0, len);
              reserveSpace(2 + (len + 1) * 2);
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
              if (ctype != Jason::CType::Int64 &&
                  ctype != Jason::CType::UInt64) {
                throw JasonBuilderError("Must give an integer for JasonType::ObjectLong as length.");
              }
              JasonLength len =   ctype == Jason::CType::UInt64 
                                ? item.getUInt64()
                                : static_cast<uint64_t>(item.getInt64());
              if (len == 0) {
                throw JasonBuilderError("Cannot create empty ObjectLong.");
              }
              if (len >= 0x100000000000000) {
                throw JasonBuilderError("Length in JasonType::ObjectLong must be < 2^56.");
              }
              _stack.emplace_back(_pos, 0, len);
              reserveSpace(8 + (len + 1) * 8);
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
              if (ctype != Jason::CType::UInt64) {
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

        uint8_t* set (JasonPair const& pair) {
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

        void reportAdd () {
          JasonLength itemStart = _pos;
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
        
        void appendUIntNoCheck (uint64_t v, uint8_t base) {
          JasonLength save = _pos++;
          uint8_t count = 0;
          do {
            _start[_pos++] = v & 0xff;
            v >>= 8;
            count++;
          } while (v != 0);
          _start[save] = base + count;
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
 
        void checkAttributeUniqueness (JasonSlice const obj) const {
          JasonLength const n = obj.length();
          JasonSlice previous = obj.keyAt(0);
          JasonLength len;
          char const* p = previous.getString(len);

          for (JasonLength i = 1; i < n; ++i) {
            JasonSlice current = obj.keyAt(i);
            if (! current.isString()) {
              return;
            }
            
            JasonLength len2;
            char const* q = current.getString(len2);

            if (len == len2 && memcmp(p, q, len2) == 0) {
              // identical key
              throw JasonBuilderError("duplicate attribute name.");
            }
            // re-use already calculated values for next round
            len = len2;
            p = q;

            // recurse into sub-objects
            JasonSlice value = obj.valueAt(i);
            if (value.isObject()) {
              checkAttributeUniqueness(value);
            }
          } 
        }

    };

  }  // namespace triagens::basics
}  // namespace triagens

#endif
