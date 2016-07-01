////////////////////////////////////////////////////////////////////////////////
/// @brief Library to build up VPack documents.
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

#include "velocypack/velocypack-common.h"
#include "velocypack/Validator.h"
#include "velocypack/Exception.h"
#include "velocypack/Slice.h"
#include "velocypack/ValueType.h"

using namespace arangodb::velocypack;

bool Validator::validate(uint8_t const* ptr, size_t length, bool isSubPart) const {
  if (length == 0) {
    throw Exception(Exception::ValidatorInvalidLength, "length 0 is invalid for any VelocyPack value");
  }

  uint8_t const head = *ptr;

  // type() only reads the first byte, which is safe
  ValueType const type = Slice(ptr).type();

  if (type == ValueType::None && head != 0x00) {
    // invalid type
    throw Exception(Exception::ValidatorInvalidType);
  }

  // special handling for certain types...
  switch (type) {
    case ValueType::None:
    case ValueType::Null: 
    case ValueType::Bool: 
    case ValueType::MinKey: 
    case ValueType::MaxKey:  
    case ValueType::SmallInt:
    case ValueType::Int: 
    case ValueType::UInt: 
    case ValueType::Double: 
    case ValueType::UTCDate: 
    case ValueType::Binary:
    case ValueType::Illegal: {
      break;
    }
    
    case ValueType::String: {
      if (head == 0xbf) {
        // long UTF-8 string. must be at least 9 bytes long so we
        // can read the entire string length safely
        validateBufferLength(1 + 8, length, true);
      } 
      break;
    }

    case ValueType::Array: {
      validateArray(ptr, length);
      break;
    }
    
    case ValueType::Object: {
      validateObject(ptr, length);
      break;
    }
/*
*/    

/*
    case ValueType::Object: {
      ValueLength byteLength = 0;
      if (head >= 0x0b && head <= 0x0e) {
        // Object with index table, with 1-8 bytes bytelength, sorted
        byteLength = 1ULL << (static_cast<ValueLength>(head) - 0x0b);
      } else if (head >= 0x0f && head <= 0x12) {
        // Object with index table, with 1-8 bytes bytelength, unsorted
        byteLength = 1ULL << (static_cast<ValueLength>(head) - 0x0f);
      } else if (head == 0x14) {
        // compact Object without index table
        // TODO
      }

      if (byteLength > 0) {
        validateBufferLength(1 + byteLength, length, true);
        ValueLength nrItems = Slice(ptr).length();
        uint8_t const* p = ptr + 1 + byteLength;
        uint8_t const* e = ptr + length;
        while (nrItems > 0) {
          if (p >= e) {
            throw Exception(Exception::ValidatorInvalidLength, "Object key offset is out of bounds");
          }
          // validate key 
          validate(p, e - p, true);
          // skip over key
          p += Slice(p).byteSize();
          
          if (p >= e) {
            throw Exception(Exception::ValidatorInvalidLength, "Object value offset is out of bounds");
          }
          // validate value
          validate(p, e - p, true);
          // skip over value
          p += Slice(p).byteSize();

          --nrItems;
        }
        
        // now also validate index table
        for (ValueLength i = 0; i < nrItems; ++i) {
          // get offset to key
          ValueLength offset = Slice(ptr).getNthOffset(i);
          if (offset >= length) {
            throw Exception(Exception::ValidatorInvalidLength, "Object key offset is out of bounds");
          }
          // validate length of key
          validate(ptr + offset, length - offset, true);
          // skip over key
          offset += Slice(ptr + offset).byteSize();
          if (offset >= length) {
            throw Exception(Exception::ValidatorInvalidLength, "Object value offset is out of bounds");
          }
          // validate length of value
          validate(ptr + offset, length - offset, true);
        }
      }
      break;
    }
*/
    case ValueType::BCD: {
      throw Exception(Exception::NotImplemented);
    }

    case ValueType::External: {
      // check if Externals are forbidden
      if (options->disallowExternals) {
        throw Exception(Exception::BuilderExternalsDisallowed);
      }
      // validate if Slice length exceeds the given buffer
      validateBufferLength(1 + sizeof(void*), length, true);
      // do not perform pointer validation
      break;
    }

    case ValueType::Custom: {
      ValueLength byteSize = 0;

      if (head == 0xf0) {
        byteSize = 1 + 1;
      } else if (head == 0xf1) {
        byteSize = 1 + 2;
      } else if (head == 0xf2) {
        byteSize = 1 + 4;
      } else if (head == 0xf3) {
        byteSize = 1 + 8;
      } else if (head >= 0xf4 && head <= 0xf6) {
        validateBufferLength(1 + 1, length, true);
        byteSize = 1 + 1 + readInteger<ValueLength>(ptr + 1, 1);
        if (byteSize == 1 + 1) {
          throw Exception(Exception::ValidatorInvalidLength, "Invalid size for Custom type");
        }
      } else if (head >= 0xf7 && head <= 0xf9) {
        validateBufferLength(1 + 2, length, true);
        byteSize = 1 + 2 + readInteger<ValueLength>(ptr + 1, 2); 
        if (byteSize == 1 + 2) {
          throw Exception(Exception::ValidatorInvalidLength, "Invalid size for Custom type");
        }
      } else if (head >= 0xfa && head <= 0xfc) {
        validateBufferLength(1 + 4, length, true);
        byteSize = 1 + 4 + readInteger<ValueLength>(ptr + 1, 4); 
        if (byteSize == 1 + 4) {
          throw Exception(Exception::ValidatorInvalidLength, "Invalid size for Custom type");
        }
      } else if (head >= 0xfd) {
        validateBufferLength(1 + 8, length, true);
        byteSize = 1 + 8 + readInteger<ValueLength>(ptr + 1, 8); 
        if (byteSize == 1 + 8) {
          throw Exception(Exception::ValidatorInvalidLength, "Invalid size for Custom type");
        }
      }
  
      validateSliceLength(ptr, byteSize, isSubPart);
      break;
    }
  }
     
  // common validation that must happen for all types 
  validateSliceLength(ptr, length, isSubPart);
  return true;
}

void Validator::validateArray(uint8_t const* ptr, size_t length) const {
  uint8_t head = *ptr;

  if (head == 0x13U) {
    // compact array
    validateCompactArray(ptr, length);
  } else if (head >= 0x02U && head <= 0x05U) {
    // array without index table
    validateUnindexedArray(ptr, length);
  } else if (head >= 0x06U && head <= 0x09U) {
    // array with index table
    validateIndexedArray(ptr, length);
  } else if (head == 0x01U) {
    // empty array. always valid
  }
}

void Validator::validateCompactArray(uint8_t const* ptr, size_t length) const {
  // compact Array without index table
  validateBufferLength(4, length, true);

  uint8_t const* p = ptr + 1;
  uint8_t const* e = p + length;
  // read byteLength
  ValueLength byteSize = 0;
  ValueLength shifter = 0;
  while (true) {
    uint8_t c = *p;
    byteSize += (c & 0x7fU) << shifter;
    shifter += 7;
    ++p;
    if (!(c & 0x80U)) {
      break;
    }
    if (p == e) {
      throw Exception(Exception::ValidatorInvalidLength, "Array length value is out of bounds");
    }
  }
  if (byteSize > length || byteSize < 4) {
    throw Exception(Exception::ValidatorInvalidLength, "Array length value is out of bounds");
  }

  // read nrItems
  uint8_t const* data = p;
  p = ptr + byteSize - 1;
  ValueLength nrItems = 0;
  shifter = 0;
  while (true) {
    uint8_t c = *p;
    nrItems += (c & 0x7fU) << shifter;
    shifter += 7;
    --p;
    if (!(c & 0x80U)) {
      break;
    }
    if (p == ptr + byteSize) {
      throw Exception(Exception::ValidatorInvalidLength, "Array length value is out of bounds");
    }
  } 
  if (nrItems == 0) {
    throw Exception(Exception::ValidatorInvalidLength, "Array length value is out of bounds");
  }
  ++p;
  
  // validate the array members 
  e = p;
  p = data;
  while (nrItems-- > 0) { 
    validate(p, e - p, true);
    p += Slice(p).byteSize();
  }
}

void Validator::validateUnindexedArray(uint8_t const* ptr, size_t length) const {
  // Array without index table, with 1-8 bytes lengths, all values with same length
  uint8_t head = *ptr;
  ValueLength byteSizeLength = 1ULL << (static_cast<ValueLength>(head) - 0x02U);
  validateBufferLength(1 + byteSizeLength + 1, length, true);
  ValueLength byteSize = readInteger<ValueLength>(ptr + 1, byteSizeLength);
  
  if (byteSize > length) {
    throw Exception(Exception::ValidatorInvalidLength, "Array length is out of bounds");
  }

  // look up first member
  uint8_t const* p = ptr + 1 + byteSizeLength;
  uint8_t const* e = ptr + 1 + (8 - byteSizeLength);
  
  if (e > ptr + byteSize) {
    e = ptr + byteSize;
  }
  while (p < e && *p == '\x00') {
    ++p;
  }

  if (p >= e) {
    throw Exception(Exception::ValidatorInvalidLength, "Array structure is invalid");
  }
  
  validate(p, length - (p - ptr), true);
  ValueLength itemSize = Slice(p).byteSize();
  ValueLength nrItems = (byteSize - (p - ptr)) / itemSize; 
  
  if (nrItems == 0) {
    throw Exception(Exception::ValidatorInvalidLength, "Array nrItems value is invalid");
  }

  e = ptr + length;
  while (nrItems > 0) {
    if (p >= e) {
      throw Exception(Exception::ValidatorInvalidLength, "Array value is out of bounds");
    }
    // validate sub value
    validate(p, e - p, true);
    if (Slice(p).byteSize() != itemSize) {
      // got a sub-object with a different size. this is not allowed
      throw Exception(Exception::ValidatorInvalidLength, "Unexpected Array value length");
    }
    p += itemSize;
    --nrItems;
  }
}

void Validator::validateIndexedArray(uint8_t const* ptr, size_t length) const {
  // Array with index table, with 1-8 bytes lengths
  uint8_t head = *ptr;
  ValueLength byteSizeLength = 1ULL << (static_cast<ValueLength>(head) - 0x06U);
  validateBufferLength(1 + byteSizeLength + byteSizeLength + 1, length, true);
  ValueLength byteSize = readInteger<ValueLength>(ptr + 1, byteSizeLength);
  if (byteSize > length) {
    throw Exception(Exception::ValidatorInvalidLength, "Array length is out of bounds");
  }

  ValueLength nrItems;
  ValueLength dataOffset;
  uint8_t const* indexTable;

  if (head == 0x09U) {
    // byte length = 8
    nrItems = readInteger<ValueLength>(ptr + byteSize - byteSizeLength, byteSizeLength);
    
    if (nrItems == 0) {
      throw Exception(Exception::ValidatorInvalidLength, "Array nrItems value is invalid");
    }

    indexTable = ptr + byteSize - byteSizeLength - (nrItems * byteSizeLength);
    if (indexTable < ptr + byteSizeLength) {
      throw Exception(Exception::ValidatorInvalidLength, "Array index table is out of bounds");
    }
    
    dataOffset = 1 + byteSizeLength;
  } else {
    // byte length = 1, 2 or 4
    nrItems = readInteger<ValueLength>(ptr + 1 + byteSizeLength, byteSizeLength);
    
    if (nrItems == 0) {
      throw Exception(Exception::ValidatorInvalidLength, "Array nrItems value is invalid");
    }
    
    // look up first member
    uint8_t const* p = ptr + 1 + byteSizeLength + byteSizeLength;
    uint8_t const* e = ptr + 1 + (8 - byteSizeLength - byteSizeLength);
    if (e > ptr + byteSize) {
      e = ptr + byteSize;
    }
    while (p < e && *p == '\x00') {
      ++p;
    }

    indexTable = ptr + byteSize - (nrItems * byteSizeLength);
    if (indexTable < ptr + byteSizeLength + byteSizeLength || indexTable < p) {
      throw Exception(Exception::ValidatorInvalidLength, "Array index table is out of bounds");
    }
    
    dataOffset = 1 + byteSizeLength + byteSizeLength;
  }

  while (nrItems > 0) {
    ValueLength offset = readInteger<ValueLength>(indexTable, byteSizeLength);
    if (offset < dataOffset || offset >= indexTable - ptr) {
      throw Exception(Exception::ValidatorInvalidLength, "Array index table entry is out of bounds");
    }
    validate(ptr + offset, length - offset, true);
    indexTable += byteSizeLength; 
    --nrItems;
  }
}

void Validator::validateObject(uint8_t const* ptr, size_t length) const {
}

void Validator::validateBufferLength(size_t expected, size_t actual, bool isSubPart) const {
  if ((expected > actual) ||
      (expected != actual && !isSubPart)) {
    throw Exception(Exception::ValidatorInvalidLength, "given buffer length is unequal to actual length of Slice in buffer");
  }
}
        
void Validator::validateSliceLength(uint8_t const* ptr, size_t length, bool isSubPart) const {
  size_t actual = static_cast<size_t>(Slice(ptr).byteSize());
  validateBufferLength(actual, length, isSubPart);
}
