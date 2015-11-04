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

#include <ostream>
#include <fstream>
#include <string>

#include "Jason.h"
#include "JasonBuffer.h"
#include "JasonBuilder.h"
#include "JasonDump.h"
#include "JasonException.h"
#include "JasonOptions.h"
#include "JasonParser.h"
#include "JasonSlice.h"
#include "JasonType.h"

#include "gtest/gtest.h"

using namespace arangodb::jason;

// With the following function we check type determination and size
// of the produced Jason value:

static void checkBuild (JasonSlice s, JasonType t, JasonLength byteSize) {
  ASSERT_EQ(t, s.type());
  ASSERT_TRUE(s.isType(t));
  JasonType other = (t == JasonType::String) ? JasonType::Int
                                             : JasonType::String;
  ASSERT_FALSE(s.isType(other));
  ASSERT_FALSE(other == s.type());

  ASSERT_EQ(byteSize, s.byteSize());

  switch (t) {
    case JasonType::None:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      break;
    case JasonType::Null:
      ASSERT_TRUE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::Bool:
      ASSERT_FALSE(s.isNull());
      ASSERT_TRUE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::Double:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_TRUE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_TRUE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::Array:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_TRUE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::Object:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_TRUE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::External:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_TRUE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::UTCDate:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_TRUE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::Int:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_TRUE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_TRUE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::UInt:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_TRUE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_TRUE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::SmallInt:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_TRUE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_TRUE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::String:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_TRUE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::Binary:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_TRUE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::BCD:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_TRUE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::MinKey:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_TRUE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::MaxKey:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_TRUE(s.isMaxKey());
      ASSERT_FALSE(s.isCustom());
      break;
    case JasonType::Custom:
      ASSERT_FALSE(s.isNull());
      ASSERT_FALSE(s.isBool());
      ASSERT_FALSE(s.isDouble());
      ASSERT_FALSE(s.isArray());
      ASSERT_FALSE(s.isObject());
      ASSERT_FALSE(s.isExternal());
      ASSERT_FALSE(s.isUTCDate());
      ASSERT_FALSE(s.isInt());
      ASSERT_FALSE(s.isUInt());
      ASSERT_FALSE(s.isSmallInt());
      ASSERT_FALSE(s.isString());
      ASSERT_FALSE(s.isBinary());
      ASSERT_FALSE(s.isNumber());
      ASSERT_FALSE(s.isBCD());
      ASSERT_FALSE(s.isMinKey());
      ASSERT_FALSE(s.isMaxKey());
      ASSERT_TRUE(s.isCustom());
      break;
  }
}


// Let the tests begin...

TEST(BuilderTest, FixedArraysSizes) {
  JasonLength const kB = 1024;
  JasonLength const GB = 1024*1024*1024;
  JasonLength const nrs[] = {1,               // bytelen < 256
                             2,               // 256 <= bytelen < 64k
                             (64*kB)/127 - 1, // 256 <= bytelen < 64k
                             (64*kB)/127,     // 64k <= bytelen < 4G
                             (4*GB)/127,      // 64k <= bytelen < 4G
                             (4*GB)/127+1};   // 4G <= bytelen
  JasonLength const byteSizes[] = {1 + 1 + 1 * 127,
                                   1 + 8 + 2 * 127,
                                   1 + 8 + ((64 * kB) / 127 - 1) * 127,
                                   1 + 8 + ((64 * kB) / 127) * 127,
                                   1 + 8 + ((4 * GB) / 127) * 127,
                                   1 + 8 + ((4 * GB) / 127 + 1) * 127};
  int nr = sizeof(nrs) / sizeof(JasonLength);

  std::string x;
  for (size_t i = 0; i < 128-2; i++) {
    x.push_back('x');
  }
  // Now x has length 128-2 and thus will use 128 bytes as an entry in an array

  for (int i = 0; i < nr; i++) {
    JasonBuilder b;
    b.reserve(byteSizes[i]);
    b.add(Jason(JasonType::Array));
    for (JasonLength j = 0; j < nrs[i]; j++) {
      b.add(Jason(x));
    }
    b.close();
    uint8_t* start = b.start();

    JasonSlice s(start);
    checkBuild(s, JasonType::Array, byteSizes[i]);
    ASSERT_TRUE(0x02 <= *start && *start <= 0x05);  // Array without index tab
    ASSERT_TRUE(s.isArray());
    ASSERT_EQ(nrs[i], s.length());
    ASSERT_TRUE(s[0].isString());
    JasonLength len;
    char const* p = s[0].getString(len);
    ASSERT_EQ(x.size(), len);
    ASSERT_EQ(x, std::string(p, len));
  }
}


int main (int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

