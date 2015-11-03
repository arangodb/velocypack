////////////////////////////////////////////////////////////////////////////////
/// @brief Library to build up Jason documents.
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

#include <unordered_set>

#include "JasonBuilder.h"

using namespace arangodb::jason;

void JasonBuilder::doActualSort (std::vector<SortEntry>& entries) {
  JASON_ASSERT(entries.size() > 1);
  std::sort(entries.begin(), entries.end(), 
    [] (SortEntry const& a, SortEntry const& b) {
      // return true iff a < b:
      uint8_t const* pa = a.nameStart;
      uint64_t sizea = a.nameSize;
      uint8_t const* pb = b.nameStart;
      uint64_t sizeb = b.nameSize;
      size_t const compareLength
          = static_cast<size_t>((std::min)(sizea, sizeb));
      int res = memcmp(pa, pb, compareLength);

      return (res < 0 || (res == 0 && sizea < sizeb));
    });
};

uint8_t const* JasonBuilder::findAttrName (uint8_t const* base, uint64_t& len) {
  uint8_t const b = *base;
  if (b >= 0x40 && b <= 0xbe) {
    // short UTF-8 string
    len = b - 0x40;
    return base + 1;
  }
  if (b == 0xbf) {
    // long UTF-8 string
    len = 0;
    // read string length
    for (size_t i = 8; i >= 1; i--) {
      len = (len << 8) + base[i];
    }
    return base + 1 + 8; // string starts here
  }
  throw JasonException(JasonException::NotImplemented, "Invalid Object key type");
}

void JasonBuilder::sortObjectIndexShort (uint8_t* objBase,
                                         std::vector<JasonLength>& offsets) {
  auto cmp = [&] (JasonLength a, JasonLength b) -> bool {
    uint8_t const* aa = objBase + a;
    uint8_t const* bb = objBase + b;
    if (*aa >= 0x40 && *aa <= 0xbe &&
        *bb >= 0x40 && *bb <= 0xbe) {
      // The fast path, short strings:
      uint8_t m = (std::min)(*aa - 0x40, *bb - 0x40);
      int c = memcmp(aa+1, bb+1, static_cast<size_t>(m));
      return (c < 0 || (c == 0 && *aa < *bb));
    }
    else {
      uint64_t lena;
      uint64_t lenb;
      aa = findAttrName(aa, lena);
      bb = findAttrName(bb, lenb);
      uint64_t m = (std::min)(lena, lenb);
      int c = memcmp(aa, bb, m);
      return (c < 0 || (c == 0 && lena < lenb));
    }
  };
  std::sort(offsets.begin(), offsets.end(), cmp);
}

void JasonBuilder::sortObjectIndexLong (uint8_t* objBase,
                                        std::vector<JasonLength>& offsets) {

  // on some platforms we can use a thread-local vector
#if __llvm__ == 1
  // nono thread local
  std::vector<JasonBuilder::SortEntry> entries;
#elif defined(_WIN32) && defined(_MSC_VER)
  std::vector<JasonBuilder::SortEntry> entries;
#else
  // thread local vector for sorting large object attributes
  thread_local std::vector<JasonBuilder::SortEntry> entries;
  entries.clear();
#endif
  
  entries.reserve(offsets.size());
  for (JasonLength i = 0; i < offsets.size(); i++) {
    SortEntry e;
    e.offset = offsets[i];
    e.nameStart = findAttrName(objBase + e.offset, e.nameSize);
    entries.push_back(e);
  }
  JASON_ASSERT(entries.size() == offsets.size());
  doActualSort(entries);

  // copy back the sorted offsets 
  for (JasonLength i = 0; i < offsets.size(); i++) {
    offsets[i] = entries[i].offset;
  }
}

void JasonBuilder::sortObjectIndex (uint8_t* objBase,
                                    std::vector<JasonLength>& offsets) {
  if (offsets.size() > 32) {
    sortObjectIndexLong(objBase, offsets);
  }
  else {
    sortObjectIndexShort(objBase, offsets);
  }
}


void JasonBuilder::close () {
  if (_stack.empty()) {
    throw JasonException(JasonException::BuilderNeedOpenObject);
  }
  JasonLength& tos = _stack.back();
  if (_start[tos] != 0x06 && _start[tos] != 0x0b) {
    throw JasonException(JasonException::BuilderNeedOpenObject);
  }
  std::vector<JasonLength>& index = _index[_stack.size() - 1];
  if (index.empty()) {
    _start[tos] = (_start[tos] == 0x06) ? 0x01 : 0x0a;
    JASON_ASSERT(_pos == tos + 9);
    _pos -= 8;  // no bytelength and number subvalues needed
    _stack.pop_back();
    // Intentionally leave _index[depth] intact to avoid future allocs!
    return;
  }
  // From now on index.size() > 0

  bool needIndexTable = true;
  if (index.size() == 1) {
    needIndexTable = false;
  }
  else if (_start[tos] == 0x06 &&   // an array
           (_pos - tos) - index[0] == index.size() * (index[1] - index[0])) {
    // In this case it could be that all entries have the same length
    // and we do not need an offset table at all:
    bool noTable = true;
    JasonLength subLen = index[1] - index[0];
    for (size_t i = 1; i < index.size() - 1; i++) {
      if (index[i + 1] - index[i] != subLen) {
        noTable = false;
        break;
      }
    }
    if ((_pos - tos) - index[index.size()-1] != subLen) {
      noTable = false;
    }
    if (noTable) {
      needIndexTable = false;
    }
  }

  // First determine byte length and its format:
  unsigned int offsetSize;   
        // can be 1, 2, 4 or 8 for the byte width of the offsets,
        // the byte length and the number of subvalues:
  if (_pos - tos + (needIndexTable ? index.size() : 0) - 6 <= 0xff) {
    // We have so far used _pos - tos bytes, including the reserved 8
    // bytes for byte length and number of subvalues. In the 1-byte number
    // case we would win back 6 bytes but would need one byte per subvalue
    // for the index table
    offsetSize = 1;
  }
  else if (_pos - tos + (needIndexTable ? 2 * index.size() : 0) <= 0xffff) {
    offsetSize = 2;
  }
  else if (_pos - tos + (needIndexTable ? 4 * index.size() : 0) <= 0xffffffffu) {
    offsetSize = 4;
  }
  else {
    offsetSize = 8;
  }

  // Maybe we need to move down data:
  if (offsetSize == 1) {
    unsigned int targetPos = 3;
    if (! needIndexTable && _start[tos] == 0x06) {
      targetPos = 2;
    }
    if (_pos > (tos + 9)) {
      memmove(_start + tos + targetPos, _start + tos + 9,
              _pos - (tos + 9));
    }
    _pos -= (9 - targetPos);
    for (size_t i = 0; i < index.size(); i++) {
      index[i] -= (9 - targetPos);
    }
  }
  // One could move down things in the offsetSize == 2 case as well,
  // since we only need 4 bytes in the beginning. However, saving these
  // 4 bytes has been sacrificed on the Altar of Performance.

  // Now build the table:
  if (needIndexTable) {
    JasonLength tableBase;
    reserveSpace(offsetSize * index.size() + (offsetSize == 8 ? 8 : 0));
    tableBase = _pos;
    _pos += offsetSize * index.size();
    if (_start[tos] == 0x0b) {  // an object
      if (! options.sortAttributeNames) {
        _start[tos] = 0x0f;  // unsorted
      }
      else if (index.size() >= 2 &&
               options.sortAttributeNames) {
        sortObjectIndex(_start + tos, index);
      }
    }
    for (size_t i = 0; i < index.size(); i++) {
      uint64_t x = index[i];
      for (size_t j = 0; j < offsetSize; j++) {
        _start[tableBase + offsetSize * i + j] = x & 0xff;
        x >>= 8;
      }
    }
  }
  else {  // no index table
    if (_start[tos] == 0x06) {
      _start[tos] = 0x02;
    }
  }
  // Finally fix the byte width in the type byte:
  if (offsetSize > 1) {
    if (offsetSize == 2) {
      _start[tos] += 1;
    }
    else if (offsetSize == 4) {
      _start[tos] += 2;
    }
    else {   // offsetSize == 8
      _start[tos] += 3;
      appendLength(index.size(), 8);
    }
  }

  // Fix the byte length in the beginning:
  JasonLength x = _pos - tos;
  for (unsigned int i = 1; i <= offsetSize; i++) {
    _start[tos + i] = x & 0xff;
    x >>= 8;
  }

  if (offsetSize < 8) {
    x = index.size();
    for (unsigned int i = offsetSize + 1; i <= 2 * offsetSize; i++) {
      _start[tos + i] = x & 0xff;
      x >>= 8;
    }
  }

  // And, if desired, check attribute uniqueness:
  if (options.checkAttributeUniqueness && index.size() > 1 &&
      _start[tos] >= 0x0b) {
    // check uniqueness of attribute names
    checkAttributeUniqueness(JasonSlice(_start + tos));
  }

  // Now the array or object is complete, we pop a JasonLength 
  // off the _stack:
  _stack.pop_back();
  // Intentionally leave _index[depth] intact to avoid future allocs!
}

void JasonBuilder::set (Jason const& item) {
  auto ctype = item.cType();

  // This method builds a single further Jason item at the current
  // append position. If this is an array or object, then an index
  // table is created and a new JasonLength is pushed onto the stack.
  switch (item.jasonType()) {
    case JasonType::None: {
      throw JasonException(JasonException::BuilderUnexpectedType, "Cannot set a JasonType::None");
    }
    case JasonType::Null: {
      reserveSpace(1);
      _start[_pos++] = 0x18;
      break;
    }
    case JasonType::Bool: {
      if (ctype != Jason::CType::Bool) {
        throw JasonException(JasonException::BuilderUnexpectedValue, "Must give bool for JasonType::Bool");
      }
      reserveSpace(1);
      if (item.getBool()) {
        _start[_pos++] = 0x1a;
      }
      else {
        _start[_pos++] = 0x19;
      }
      break;
    }
    case JasonType::Double: {
      static_assert(sizeof(double) == sizeof(uint64_t),
                    "size of double is not 8 bytes");
      double v = 0.0;
      uint64_t x;
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
          throw JasonException(JasonException::BuilderUnexpectedValue, "Must give number for JasonType::Double");
      }
      reserveSpace(1 + sizeof(double));
      _start[_pos++] = 0x1b;
      memcpy(&x, &v, sizeof(double));
      appendLength(x, 8);
      break;
    }
    case JasonType::External: {
      if (ctype != Jason::CType::VoidPtr) {
        throw JasonException(JasonException::BuilderUnexpectedValue, "Must give void pointer for JasonType::External");
      }
      reserveSpace(1 + sizeof(void*));
      // store pointer. this doesn't need to be portable
      _start[_pos++] = 0x1d;
      void const* value = item.getExternal();
      memcpy(_start + _pos, &value, sizeof(void*));
      _pos += sizeof(void*);
      break;
    }
    case JasonType::SmallInt: {
      int64_t vv = 0;
      switch (ctype) {
        case Jason::CType::Double:
          vv = static_cast<int64_t>(item.getDouble());
          break;
        case Jason::CType::Int64:
          vv = item.getInt64();
          break;
        case Jason::CType::UInt64:
          vv = static_cast<int64_t>(item.getUInt64());
        default:
          throw JasonException(JasonException::BuilderUnexpectedValue, "Must give number for JasonType::SmallInt");
      }
      if (vv < -6 || vv > 9) {
        throw JasonException(JasonException::NumberOutOfRange, "Number out of range of JasonType::SmallInt");
      } 
      reserveSpace(1);
      if (vv >= 0) {
        _start[_pos++] = static_cast<uint8_t>(vv + 0x30);
      }
      else {
        _start[_pos++] = static_cast<uint8_t>(vv + 0x40);
      }
      break;
    }
    case JasonType::Int: {
      int64_t v;
      switch (ctype) {
        case Jason::CType::Double:
          v = static_cast<int64_t>(item.getDouble());
          break;
        case Jason::CType::Int64:
          v = item.getInt64();
          break;
        case Jason::CType::UInt64:
          v = toInt64(item.getUInt64());
          break;
        default:
          throw JasonException(JasonException::BuilderUnexpectedValue, "Must give number for JasonType::Int");
      }
      addInt(v);
      break;
    }
    case JasonType::UInt: {
      uint64_t v = 0;
      switch (ctype) {
        case Jason::CType::Double:
          if (item.getDouble() < 0.0) {
            throw JasonException(JasonException::BuilderUnexpectedValue, "Must give non-negative number for JasonType::UInt");
          }
          v = static_cast<uint64_t>(item.getDouble());
          break;
        case Jason::CType::Int64:
          if (item.getInt64() < 0) {
            throw JasonException(JasonException::BuilderUnexpectedValue, "Must give non-negative number for JasonType::UInt");
          }
          v = static_cast<uint64_t>(item.getInt64());
          break;
        case Jason::CType::UInt64:
          v = item.getUInt64();
          break;
        default:
          throw JasonException(JasonException::BuilderUnexpectedValue, "Must give number for JasonType::UInt");
      }
      addUInt(v); 
      break;
    }
    case JasonType::UTCDate: {
      int64_t v;
      switch (ctype) {
        case Jason::CType::Double:
          v = static_cast<int64_t>(item.getDouble());
          break;
        case Jason::CType::Int64:
          v = item.getInt64();
          break;
        case Jason::CType::UInt64:
          v = toInt64(item.getUInt64());
          break;
        default:
          throw JasonException(JasonException::BuilderUnexpectedValue, "Must give number for JasonType::UTCDate");
      }
      addUTCDate(v);
      break;
    }
    case JasonType::String: {
      if (ctype != Jason::CType::String &&
          ctype != Jason::CType::CharPtr) {
        throw JasonException(JasonException::BuilderUnexpectedValue, "Must give a string or char const* for JasonType::String");
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
      if (size <= 126) {
        // short string
        reserveSpace(1 + size);
        _start[_pos++] = static_cast<uint8_t>(0x40 + size);
        memcpy(_start + _pos, s->c_str(), size);
      }
      else {
        // long string
        reserveSpace(1 + 8 + size);
        _start[_pos++] = 0xbf;
        appendLength(size, 8);
        memcpy(_start + _pos, s->c_str(), size);
      }
      _pos += size;
      break;
    }
    case JasonType::Array: {
      addArray();
      break;
    }
    case JasonType::Object: {
      addObject();
      break;
    }
    case JasonType::Binary: {
      if (ctype != Jason::CType::String &&
          ctype != Jason::CType::CharPtr) {
        throw JasonException(JasonException::BuilderUnexpectedValue, "Must provide std::string or char const* for JasonType::Binary");
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
      JasonLength v = s->size();
      appendUInt(v, 0xbf);
      memcpy(_start + _pos, s->c_str(), v);
      _pos += v;
      break;
    }
    case JasonType::MinKey: {
      reserveSpace(1);
      _start[_pos++] = 0x1e;
      break;
    }
    case JasonType::MaxKey: {
      reserveSpace(1);
      _start[_pos++] = 0x1f;
      break;
    }
    case JasonType::BCD: {
      throw JasonException(JasonException::NotImplemented);
    }
    case JasonType::Custom: {
      throw JasonException(JasonException::BuilderUnexpectedType, "Cannot set a JasonType::Custom with this method");
    }
  }
}

uint8_t* JasonBuilder::set (JasonPair const& pair) {
  // This method builds a single further Jason item at the current
  // append position. This is the case for JasonType::ID or
  // JasonType::Binary, which both need two pieces of information
  // to build.
  if (pair.jasonType() == JasonType::Binary) {
    uint64_t v = pair.getSize();
    appendUInt(v, 0xbf);
    memcpy(_start + _pos, pair.getStart(), v);
    _pos += v;
    return nullptr;  // unused here
  }
  else if (pair.jasonType() == JasonType::String) {
    uint64_t size = pair.getSize();
    if (size > 126) { 
      // long string
      reserveSpace(1 + 8 + size);
      _start[_pos++] = 0xbf;
      appendLength(size, 8);
      _pos += size;
    }
    else {
      // short string
      reserveSpace(1 + size);
      _start[_pos++] = static_cast<uint8_t>(0x40 + size);
      _pos += size;
    }
    // Note that the data is not filled in! It is the responsibility
    // of the caller to fill in 
    //   _start + _pos - size .. _start + _pos - 1
    // with valid UTF-8!
    return _start + _pos - size;
  }
  else if (pair.jasonType() == JasonType::Custom) {
    // We only reserve space here, the caller has to fill in the custom type
    uint64_t size = pair.getSize();
    reserveSpace(size);
    uint8_t const* p = pair.getStart();
    if (p != nullptr) {
      memcpy(_start + _pos, p, size);
    }
    _pos += size;
    return _start + _pos - size;
  }
  throw JasonException(JasonException::BuilderUnexpectedType, "Only JasonType::Binary, JasonType::String and JasonType::Custom are valid for JasonPair argument");
}

void JasonBuilder::checkAttributeUniqueness (JasonSlice const obj) const {
  JASON_ASSERT(options.checkAttributeUniqueness == true);
  JasonLength const n = obj.length();

  if (obj.isSorted()) {
    // object attributes are sorted
    JasonSlice previous = obj.keyAt(0);
    JasonLength len;
    char const* p = previous.getString(len);

    // compare each two adjacent attribute names
    for (JasonLength i = 1; i < n; ++i) {
      JasonSlice current = obj.keyAt(i);
      if (! current.isString()) {
        throw JasonException(JasonException::InternalError, "Expecting String key");
      }
      
      JasonLength len2;
      char const* q = current.getString(len2);

      if (len == len2 && memcmp(p, q, len2) == 0) {
        // identical key
        throw JasonException(JasonException::DuplicateAttributeName);
      }
      // re-use already calculated values for next round
      len = len2;
      p = q;
    }
  }
  else {
    std::unordered_set<std::string> keys;

    for (size_t i = 0; i < n; ++i) {
      JasonSlice key = obj.keyAt(i);
      if (! key.isString()) {
        throw JasonException(JasonException::InternalError, "Expecting String key");
      }
      
      if (! keys.emplace(key.copyString()).second) {
        throw JasonException(JasonException::DuplicateAttributeName);
      }
    }
  }
}

void JasonBuilder::add (std::string const& attrName, Jason const& sub) {
  if (_attrWritten) {
    throw JasonException(JasonException::InternalError, "Attribute name already written");
  }
  if (! _stack.empty()) {
    JasonLength& tos = _stack.back();
    if (_start[tos] != 0x06 &&
        _start[tos] != 0x0b) {
      throw JasonException(JasonException::BuilderNeedOpenObject);
    }
    reportAdd(tos);
  }
  set(Jason(attrName, JasonType::String));
  set(sub);
}

uint8_t* JasonBuilder::add (std::string const& attrName, JasonPair const& sub) {
  if (_attrWritten) {
    throw JasonException(JasonException::InternalError, "Attribute name already written");
  }
  if (! _stack.empty()) {
    JasonLength& tos = _stack.back();
    if (_start[tos] != 0x06 &&
        _start[tos] != 0x0b) {
      throw JasonException(JasonException::BuilderNeedOpenObject);
    }
    reportAdd(tos);
  }
  set(Jason(attrName, JasonType::String));
  return set(sub);
}

void JasonBuilder::add (Jason const& sub) {
  if (! _stack.empty()) {
    JasonLength& tos = _stack.back();
    if (_start[tos] != 0x06 && _start[tos] != 0x0b) {
      // no array or object
      throw JasonException(JasonException::BuilderNeedOpenObject);
    }
    if (_start[tos] == 0x0b) {   // object
      if (! _attrWritten && ! sub.isString()) {
        throw JasonException(JasonException::BuilderNeedOpenObject);
      }
      if (! _attrWritten) {
        reportAdd(tos);
      }
      _attrWritten = ! _attrWritten;
    }
    else {
      reportAdd(tos);
    }
  }
  set(sub);
}

uint8_t* JasonBuilder::add (JasonPair const& sub) {
  if (! _stack.empty()) {
    JasonLength& tos = _stack.back();
    if (_start[tos] != 0x06 && _start[tos] != 0x0b) {
      throw JasonException(JasonException::BuilderNeedOpenObject);
    }
    if (_start[tos] == 0x06) {   // object
      if (! _attrWritten && ! sub.isString()) {
        throw JasonException(JasonException::BuilderNeedOpenObject);
      }
      if (! _attrWritten) {
        reportAdd(tos);
      }
      _attrWritten = ! _attrWritten;
    }
    else {
      reportAdd(tos);
    }
  }
  return set(sub);
}

