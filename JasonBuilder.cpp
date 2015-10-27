////////////////////////////////////////////////////////////////////////////////
/// @brief Library to build up Jason documents.
///
/// @file JasonBuilder.cpp
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

#include "JasonBuilder.h"

using namespace arangodb::jason;

// thread local vector for sorting large object attributes
thread_local std::vector<JasonBuilder::SortEntry> JasonBuilder::SortObjectEntries;

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
  throw JasonBuilderError("Unimplemented attribute name type.");
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
  std::vector<SortEntry>& entries = SortObjectEntries; 
  entries.clear();
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

void JasonBuilder::close () {
  if (_stack.empty()) {
    throw JasonBuilderError("Need open array or object for close() call.");
  }
  JasonLength& tos = _stack.back();
  if (_start[tos] != 0x05 && _start[tos] != 0x08) {
    throw JasonBuilderError("Need open array or object for close() call.");
  }
  std::vector<JasonLength>& index = _index[_stack.size() - 1];
  if (index.size() == 0) {
    _start[tos] = _start[tos] == 0x05 ? 0x04 : 0x08;
    _start[tos+1] = 0x02;
    JASON_ASSERT(_pos == tos + 2);
    _stack.pop_back();
    // Intentionally leave _index[depth] intact to avoid future allocs!
    return;
  }
  // From now on index.size() > 0

  // First determine byte length and its format:
  bool smallNrElms = index.size() <= 0xff;
  unsigned int offsetSize;   
        // can be 0, 2, 4 or 8 for the byte length of the offsets,
        // where 0 indicates no offset table at all
  if (index.back() <= 0xffff) {
    offsetSize = 2;
  }
  else if (index.back() <= 0xffffffffu) {
    offsetSize = 4;
  }
  else {
    offsetSize = 8;
  }
  if (index.size() == 1) {
    offsetSize = 0;
  }
  else if (_start[tos] == 0x05 &&   // an array
           _pos - index[0] == index.size() * (index[1] - index[0])) {
    // In this case it could be that all entries have the same length
    // and we do not need an offset table at all:
    bool noTable = true;
    JasonLength subLen = index[1] - index[0];
    for (size_t i = 1; i < index.size()-1; i++) {
      if (index[i+1] - index[i] != subLen) {
        noTable = false;
        break;
      }
    }
    if (noTable) {
      offsetSize = 0;
    }
  }

  // Determine whether we need a small byte length or a large one:
  bool smallByteLength;
  if (_pos - tos - 8 + 1 + offsetSize * index.size() < 0x100) {
    // This is the condition for a one-byte bytelength, since in
    // that case we can save 8 bytes in the beginning and the
    // table fits in the 256 bytes as well, using the small format.
    // Note that the number of elements will be < 256 anyway, since 
    // each subvalue needs at least one byte, so we do not have to
    // consider smallNrElms == false here.
    if (_pos > (tos + 10)) {
      memmove(_start + tos + 2, _start + tos + 10,
              _pos - (tos + 10));
    }
    _pos -= 8;
    for (size_t i = 0; i < index.size(); i++) {
      index[i] -= 8;
    }
    smallByteLength = true;
  }
  else {
    smallByteLength = false;
  }

  // Now build the table:
  JasonLength tableBase;
  reserveSpace(offsetSize * index.size() + (smallNrElms ? 1 : 9));
  if (offsetSize > 0) {
    tableBase = _pos;
    _pos += offsetSize * index.size();
    if (offsetSize == 2) {
      // In this case the type 0x05 or 0x08 is already correct, unless
      // sorting of attribute names is switched off!
      if (_start[tos] == 0x08 && ! options.sortAttributeNames) {
        _start[tos] = 0x0b;
      }
      if (index.size() >= 2 &&
          options.sortAttributeNames) {
        sortObjectIndexShort(_start + tos, index);
      }
      for (size_t i = 0; i < index.size(); i++) {
        uint16_t x = static_cast<uint16_t>(index[i]);
        _start[tableBase + 2 * i] = x & 0xff;
        _start[tableBase + 2 * i + 1] = x >> 8;
      }
    }
    else {
      // large table:
      // Make sure we use the right type:
      if (_start[tos] == 0x05) {   // array case
        _start[tos] = offsetSize == 4 ? 0x06 : 0x07;
      }
      else {   // object case
        _start[tos] =   (offsetSize == 4 ? 0x09 : 0x0a)
                      + (options.sortAttributeNames ? 0 : 3);
      }
      if (index.size() >= 2 &&
          options.sortAttributeNames) {
        sortObjectIndexLong(_start + tos, index);
      }
      for (size_t i = 0; i < index.size(); i++) {
        uint64_t x = index[i];
        for (size_t j = 0; j < offsetSize; j++) {
          _start[tableBase + offsetSize * i + j] = x & 0xff;
          x >>= 8;
        }
      }
    }
  }
  else {  // offsetSize == 0
    _start[tos] = 0x04;
  }

  // Now write the number of elements:
  if (smallNrElms) {
    _start[_pos++] = static_cast<uint8_t>(index.size());
  }
  else {
    appendLength(index.size(), 8);
    _start[_pos++] = 0;
  }

  // Fix the byte length in the beginning:
  if (smallByteLength) {
    _start[tos + 1] = _pos - tos;
  }
  else {
    _start[tos + 1] = 0x00;
    JasonLength x = _pos - tos;
    for (size_t i = 2; i <= 9; i++) {
      _start[tos + i] = x & 0xff;
      x >>= 8;
    }
  }

  // And, if desired, check attribute uniqueness:
  if (options.checkAttributeUniqueness && index.size() > 1 &&
      _start[tos] >= 0x08) {
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
    case JasonType::Custom: {
      throw JasonBuilderError("Cannot set a JasonType::Custom with this method.");
    }
    case JasonType::None: {
      throw JasonBuilderError("Cannot set a JasonType::None.");
    }
    case JasonType::Null: {
      reserveSpace(1);
      _start[_pos++] = 0x01;
      break;
    }
    case JasonType::Bool: {
      if (ctype != Jason::CType::Bool) {
        throw JasonBuilderError("Must give bool for JasonType::Bool.");
      }
      reserveSpace(1);
      if (item.getBool()) {
        _start[_pos++] = 0x03;
      }
      else {
        _start[_pos++] = 0x02;
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
          throw JasonBuilderError("Must give number for JasonType::Double.");
      }
      reserveSpace(1 + sizeof(double));
      _start[_pos++] = 0x0e;
      memcpy(&x, &v, sizeof(double));
      appendLength(x, 8);
      break;
    }
    case JasonType::External: {
      if (ctype != Jason::CType::VoidPtr) {
        throw JasonBuilderError("Must give void pointer for JasonType::External.");
      }
      reserveSpace(1 + sizeof(void*));
      // store pointer. this doesn't need to be portable
      _start[_pos++] = 0x10;
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
          throw JasonBuilderError("Must give number for JasonType::SmallInt.");
      }
      if (vv < -6 || vv > 9) {
        throw JasonBuilderError("Number out of range of JasonType::SmallInt.");
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
          throw JasonBuilderError("Must give number for JasonType::Int.");
      }
      appendInt(v, 0x1f);
      break;
    }
    case JasonType::UInt: {
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
          throw JasonBuilderError("Must give number for JasonType::UInt.");
      }
      appendUInt(v, 0x27);
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
          throw JasonBuilderError("Must give number for JasonType::UTCDate.");
      }
      addUTCDate(v);
      break;
    }
    case JasonType::String: {
      if (ctype != Jason::CType::String &&
          ctype != Jason::CType::CharPtr) {
        throw JasonBuilderError("Must give a string or char const* for JasonType::String.");
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
        _start[_pos++] = 0x40 + size;
        memcpy(_start + _pos, s->c_str(), size);
        _pos += size;
      }
      else {
        // long string
        reserveSpace(1 + 8 + size);
        _start[_pos++] = 0xbf;
        appendLength(size, 8);
        memcpy(_start + _pos, s->c_str(), size);
      }
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
        throw JasonBuilderError("Must give a string or char const* for JasonType::Binary.");
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
      _start[_pos++] = 0x11;
      break;
    }
    case JasonType::MaxKey: {
      reserveSpace(1);
      _start[_pos++] = 0x12;
      break;
    }
    case JasonType::BCD: {
      throw JasonBuilderError("BCD not yet implemented.");
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
      _start[_pos++] = 0x40 + size;
      _pos += size;
    }
    // Note that the data is not filled in! It is the responsibility
    // of the caller to fill in 
    //   _start + _pos - size .. _start + _pos - 1
    // with valid UTF-8!
    return _start + _pos - size;
  }
  throw JasonBuilderError("Only JasonType::Binary and JasonType::String are valid for JasonPair argument.");
}

void JasonBuilder::checkAttributeUniqueness (JasonSlice const obj) const {
  // FIXME: For the case ! options.sortAttributeNames, we have to use
  // a different algorithm!
  JASON_ASSERT(options.sortAttributeNames == true);
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
  } 
}

void JasonBuilder::add (std::string const& attrName, Jason const& sub) {
  if (_attrWritten) {
    throw JasonBuilderError("Attribute name already written.");
  }
  if (! _stack.empty()) {
    JasonLength& tos = _stack.back();
    if (_start[tos] != 0x05 &&
        _start[tos] != 0x08) {
      throw JasonBuilderError("Need open object for add() call.");
    }
    reportAdd(tos);
  }
  set(Jason(attrName, JasonType::String));
  set(sub);
}

uint8_t* JasonBuilder::add (std::string const& attrName, JasonPair const& sub) {
  if (_attrWritten) {
    throw JasonBuilderError("Attribute name already written.");
  }
  if (! _stack.empty()) {
    JasonLength& tos = _stack.back();
    if (_start[tos] != 0x05 &&
        _start[tos] != 0x08) {
      throw JasonBuilderError("Need open object for add() call.");
    }
    reportAdd(tos);
  }
  set(Jason(attrName, JasonType::String));
  return set(sub);
}

void JasonBuilder::add (Jason const& sub) {
  if (! _stack.empty()) {
    JasonLength& tos = _stack.back();
    if (_start[tos] != 0x05 && _start[tos] != 0x08) {
      // no array or object
      throw JasonBuilderError("Need open array or object for add() call.");
    }
    if (_start[tos] == 0x08) {   // object
      if (! _attrWritten && ! sub.isString()) {
        throw JasonBuilderError("Need open object for this add() call.");
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
    if (_start[tos] != 0x05 && _start[tos] != 0x08) {
      throw JasonBuilderError("Need open array or object for add() call.");
    }
    if (_start[tos] == 0x08) {   // object
      if (! _attrWritten && ! sub.isString()) {
        throw JasonBuilderError("Need open object for this add() call.");
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

