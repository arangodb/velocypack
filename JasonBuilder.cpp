#include "JasonBuilder.h"

using namespace arangodb::jason;

// thread local vector for sorting large object attributes
thread_local std::vector<JasonBuilder::SortEntryLarge> JasonBuilder::SortObjectLargeEntries;

void JasonBuilder::doActualSortLarge (std::vector<SortEntryLarge>& entries) {
  JASON_ASSERT(entries.size() > 1);
  std::sort(entries.begin(), entries.end(), 
    [] (SortEntryLarge const& a, SortEntryLarge const& b) {
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
  if (b >= 0x40 && b <= 0xbf) {
    // short UTF-8 string
    len = b - 0x40;
    return base + 1;
  }
  if (b == 0x0c) {
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
    if (*aa >= 0x40 && *aa <= 0xbf &&
        *bb >= 0x40 && *bb <= 0xbf) {
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
  std::vector<SortEntryLarge>& entries = SortObjectLargeEntries; 
  entries.clear();
  entries.reserve(offsets.size());
  for (JasonLength i = 0; i < offsets.size(); i++) {
    SortEntryLarge e;
    e.offset = offsets[i];
    e.nameStart = findAttrName(objBase + e.offset, e.nameSize);
    entries.push_back(e);
  }
  JASON_ASSERT(entries.size() == offsets.size());
  doActualSortLarge(entries);

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
  if (_start[tos] < 0x05 || _start[tos] > 0x08) {
    throw JasonBuilderError("Need open array or object for close() call.");
  }
  std::vector<JasonLength>& index = _index[_stack.size() - 1];
  // First determine byte length and its format:
  JasonLength tableBase;
  bool smallByteLength;
  bool smallTable;
  if (index.size() < 0x100 && 
      _pos - tos - 8 + 1 + 2 * index.size() < 0x100) {
    // This is the condition for a one-byte bytelength, since in
    // that case we can save 8 bytes in the beginning and the
    // table fits in the 256 bytes as well, using the small format:
    if (_pos > (tos + 10)) {
      memmove(_start + tos + 2, _start + tos + 10,
              _pos - (tos + 10));
    }
    _pos -= 8;
    for (size_t i = 0; i < index.size(); i++) {
      index[i] -= 8;
    }
    smallByteLength = true;
    smallTable = true;
  }
  else {
    smallByteLength = false;
    smallTable = index.size() < 0x100 && 
                 (index.size() == 0 || index.back() < 0x10000);
  }
  tableBase = _pos;
  if (smallTable) {
    if (! index.empty()) {
      reserveSpace(2 * index.size() + 1);
      _pos += 2 * index.size() + 1;
    }
    // Make sure we use the small type (6,5 -> 5 and 8,7 -> 7):
    if ((_start[tos] & 1) == 0) {
      --_start[tos];
    }
    if (_start[tos] == 0x07 && 
        index.size() >= 2 &&
        options.sortAttributeNames) {
      sortObjectIndexShort(_start + tos, index);
    }
    for (size_t i = 0; i < index.size(); i++) {
      uint16_t x = static_cast<uint16_t>(index[i]);
      _start[tableBase + 2 * i] = x & 0xff;
      _start[tableBase + 2 * i + 1] = x >> 8;
    }
    _start[_pos - 1] = static_cast<uint8_t>(index.size());
    // Note that for the case of length 0, we store a zero here
    // but further down we overwrite with the bytelength of 2!
  }
  else {
    // large table:
    reserveSpace(8 * index.size() + 8);
    _pos += 8 * index.size() + 8;
    // Make sure we use the large type (6,5 -> 6 and 8.7 -> 8):
    if ((_start[tos] & 1) == 1) {
      ++_start[tos];
    }
    if (_start[tos] == 0x08 && 
        index.size() >= 2 &&
        options.sortAttributeNames) {
      sortObjectIndexLong(_start + tos, index);
    }
    JasonLength x = index.size();
    for (size_t j = 0; j < 8; j++) {
      _start[_pos - 8 + j] = x & 0xff;
      x >>= 8;
    }
    for (size_t i = 0; i < index.size(); i++) {
      x = index[i];
      for (size_t j = 0; j < 8; j++) {
        _start[tableBase + 8 * i + j] = x & 0xff;
        x >>= 8;
      }
    }
  }
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

  if (options.checkAttributeUniqueness && index.size() > 1 &&
      _start[tos] >= 0x07) {
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
      _start[_pos++] = 0x04;
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
      _start[_pos++] = 0x09;
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
      if (vv < -8 || vv > 7) {
        throw JasonBuilderError("Number out of range of JasonType::SmallInt.");
      } 
      reserveSpace(1);
      if (vv >= 0) {
        _start[_pos++] = static_cast<uint8_t>(vv + 0x30);
      }
      else {
        _start[_pos++] = static_cast<uint8_t>(vv + 8 + 0x38);
      }
      break;
    }
    case JasonType::Int: {
      uint64_t v = 0;
      int64_t vv = 0;
      bool positive = true;
      switch (ctype) {
        case Jason::CType::Double:
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
          throw JasonBuilderError("Must give number for JasonType::Int.");
      }
      JasonLength size = uintLength(v);
      reserveSpace(1 + size);
      if (positive) {
        appendUInt(v, 0x17);
      }
      else {
        appendUInt(v, 0x1f);
      }
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
      JasonLength size = uintLength(v);
      reserveSpace(1 + size);
      if (item.jasonType() == JasonType::UInt) {
        appendUInt(v, 0x27);
      }
      else {
        appendUInt(v, 0x0f);
      }
      break;
    }
    case JasonType::UTCDate: {
      if (item.jasonType() == JasonType::Int) {
        throw JasonBuilderError("Must give number for JasonType::UTCDate.");
      }
      addUTCDate(item.getInt64());
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
      if (size <= 127) {
        // short string
        reserveSpace(1 + size);
        _start[_pos++] = 0x40 + size;
        memcpy(_start + _pos, s->c_str(), size);
        _pos += size;
      }
      else {
        // long string
        reserveSpace(1 + 8 + size);
        _start[_pos++] = 0x0c;
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
      JasonLength size = uintLength(v);
      reserveSpace(1 + size + v);
      appendUInt(v, 0xbf);
      memcpy(_start + _pos, s->c_str(), v);
      _pos += v;
      break;
    }
    case JasonType::ArangoDB_id: {
      reserveSpace(1);
      _start[_pos++] = 0x0b;
      break;
    }
    case JasonType::ID: {
      throw JasonBuilderError("Need a JasonPair to build a JasonType::ID.");
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
  if (pair.jasonType() == JasonType::ID) {
    reserveSpace(1);
    _start[_pos++] = 0x0a;
    set(Jason(pair.getSize(), JasonType::UInt));
    set(Jason(reinterpret_cast<char const*>(pair.getStart()),
              JasonType::String));
    return nullptr;  // unused here
  }
  else if (pair.jasonType() == JasonType::Binary) {
    uint64_t v = pair.getSize();
    JasonLength size = uintLength(v);
    reserveSpace(1 + size + v);
    appendUInt(v, 0xbf);
    memcpy(_start + _pos, pair.getStart(), v);
    _pos += v;
    return nullptr;  // unused here
  }
  else if (pair.jasonType() == JasonType::String) {
    uint64_t size = pair.getSize();
    if (size > 127) { 
      // long string
      reserveSpace(1 + 8 + size);
      _start[_pos++] = 0x0c;
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
  throw JasonBuilderError("Only JasonType::ID, JasonType::Binary and JasonType::String are valid for JasonPair argument.");
}

void JasonBuilder::checkAttributeUniqueness (JasonSlice const obj) const {
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

void JasonBuilder::add (std::string const& attrName, Jason const& sub) {
  if (_attrWritten) {
    throw JasonBuilderError("Attribute name already written.");
  }
  if (! _stack.empty()) {
    JasonLength& tos = _stack.back();
    if (_start[tos] != 0x07 &&
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
    if (_start[tos] != 0x07 &&
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
    if (_start[tos] < 0x05 || _start[tos] > 0x08) {
      // no array or object
      throw JasonBuilderError("Need open array or object for add() call.");
    }
    if (_start[tos] >= 0x07) {   // object or long object
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
    if (_start[tos] < 0x05 || _start[tos] > 0x08) {
      throw JasonBuilderError("Need open array or object for add() call.");
    }
    if (_start[tos] >= 0x07) {   // object or long object
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

