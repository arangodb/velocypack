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

#include <ostream>
#include <string>

#include "tests-common.h"

TEST(TypesTest, TestNames) {
  ASSERT_EQ(0, strcmp("none", ValueTypeName(ValueType::None)));
  ASSERT_EQ(0, strcmp("null", ValueTypeName(ValueType::Null)));
  ASSERT_EQ(0, strcmp("bool", ValueTypeName(ValueType::Bool)));
  ASSERT_EQ(0, strcmp("double", ValueTypeName(ValueType::Double)));
  ASSERT_EQ(0, strcmp("string", ValueTypeName(ValueType::String)));
  ASSERT_EQ(0, strcmp("array", ValueTypeName(ValueType::Array)));
  ASSERT_EQ(0, strcmp("object", ValueTypeName(ValueType::Object)));
  ASSERT_EQ(0, strcmp("external", ValueTypeName(ValueType::External)));
  ASSERT_EQ(0, strcmp("utc-date", ValueTypeName(ValueType::UTCDate)));
  ASSERT_EQ(0, strcmp("int", ValueTypeName(ValueType::Int)));
  ASSERT_EQ(0, strcmp("uint", ValueTypeName(ValueType::UInt)));
  ASSERT_EQ(0, strcmp("smallint", ValueTypeName(ValueType::SmallInt)));
  ASSERT_EQ(0, strcmp("binary", ValueTypeName(ValueType::Binary)));
  ASSERT_EQ(0, strcmp("bcd", ValueTypeName(ValueType::BCD)));
  ASSERT_EQ(0, strcmp("min-key", ValueTypeName(ValueType::MinKey)));
  ASSERT_EQ(0, strcmp("max-key", ValueTypeName(ValueType::MaxKey)));
  ASSERT_EQ(0, strcmp("custom", ValueTypeName(ValueType::Custom)));
}

TEST(TypesTest, TestNamesArrays) {
  uint8_t const arrays[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x9 };
  ASSERT_EQ(0, strcmp("array", ValueTypeName(Slice(&arrays[0]).type())));
  ASSERT_EQ(0, strcmp("array", ValueTypeName(Slice(&arrays[1]).type())));
  ASSERT_EQ(0, strcmp("array", ValueTypeName(Slice(&arrays[2]).type())));
  ASSERT_EQ(0, strcmp("array", ValueTypeName(Slice(&arrays[3]).type())));
  ASSERT_EQ(0, strcmp("array", ValueTypeName(Slice(&arrays[4]).type())));
  ASSERT_EQ(0, strcmp("array", ValueTypeName(Slice(&arrays[5]).type())));
  ASSERT_EQ(0, strcmp("array", ValueTypeName(Slice(&arrays[6]).type())));
  ASSERT_EQ(0, strcmp("array", ValueTypeName(Slice(&arrays[7]).type())));
}

TEST(TypesTest, TestNamesObjects) {
  uint8_t const objects[] = { 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12 };
  ASSERT_EQ(0, strcmp("object", ValueTypeName(Slice(&objects[0]).type())));
  ASSERT_EQ(0, strcmp("object", ValueTypeName(Slice(&objects[1]).type())));
  ASSERT_EQ(0, strcmp("object", ValueTypeName(Slice(&objects[2]).type())));
  ASSERT_EQ(0, strcmp("object", ValueTypeName(Slice(&objects[3]).type())));
  ASSERT_EQ(0, strcmp("object", ValueTypeName(Slice(&objects[4]).type())));
  ASSERT_EQ(0, strcmp("object", ValueTypeName(Slice(&objects[5]).type())));
  ASSERT_EQ(0, strcmp("object", ValueTypeName(Slice(&objects[6]).type())));
  ASSERT_EQ(0, strcmp("object", ValueTypeName(Slice(&objects[7]).type())));
  ASSERT_EQ(0, strcmp("object", ValueTypeName(Slice(&objects[8]).type())));
}

int main (int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

