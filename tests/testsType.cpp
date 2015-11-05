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
#include <string>

#include "tests-common.h"

TEST(TypesTest, TestNames) {
  ASSERT_EQ(0, strcmp("none", JasonTypeName(JasonType::None)));
  ASSERT_EQ(0, strcmp("null", JasonTypeName(JasonType::Null)));
  ASSERT_EQ(0, strcmp("bool", JasonTypeName(JasonType::Bool)));
  ASSERT_EQ(0, strcmp("double", JasonTypeName(JasonType::Double)));
  ASSERT_EQ(0, strcmp("string", JasonTypeName(JasonType::String)));
  ASSERT_EQ(0, strcmp("array", JasonTypeName(JasonType::Array)));
  ASSERT_EQ(0, strcmp("object", JasonTypeName(JasonType::Object)));
  ASSERT_EQ(0, strcmp("external", JasonTypeName(JasonType::External)));
  ASSERT_EQ(0, strcmp("utc-date", JasonTypeName(JasonType::UTCDate)));
  ASSERT_EQ(0, strcmp("int", JasonTypeName(JasonType::Int)));
  ASSERT_EQ(0, strcmp("uint", JasonTypeName(JasonType::UInt)));
  ASSERT_EQ(0, strcmp("smallint", JasonTypeName(JasonType::SmallInt)));
  ASSERT_EQ(0, strcmp("binary", JasonTypeName(JasonType::Binary)));
  ASSERT_EQ(0, strcmp("bcd", JasonTypeName(JasonType::BCD)));
  ASSERT_EQ(0, strcmp("min-key", JasonTypeName(JasonType::MinKey)));
  ASSERT_EQ(0, strcmp("max-key", JasonTypeName(JasonType::MaxKey)));
  ASSERT_EQ(0, strcmp("custom", JasonTypeName(JasonType::Custom)));
}

TEST(TypesTest, TestNamesArrays) {
  uint8_t const arrays[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x9 };
  ASSERT_EQ(0, strcmp("array", JasonTypeName(JasonSlice(&arrays[0]).type())));
  ASSERT_EQ(0, strcmp("array", JasonTypeName(JasonSlice(&arrays[1]).type())));
  ASSERT_EQ(0, strcmp("array", JasonTypeName(JasonSlice(&arrays[2]).type())));
  ASSERT_EQ(0, strcmp("array", JasonTypeName(JasonSlice(&arrays[3]).type())));
  ASSERT_EQ(0, strcmp("array", JasonTypeName(JasonSlice(&arrays[4]).type())));
  ASSERT_EQ(0, strcmp("array", JasonTypeName(JasonSlice(&arrays[5]).type())));
  ASSERT_EQ(0, strcmp("array", JasonTypeName(JasonSlice(&arrays[6]).type())));
  ASSERT_EQ(0, strcmp("array", JasonTypeName(JasonSlice(&arrays[7]).type())));
}

TEST(TypesTest, TestNamesObjects) {
  uint8_t const objects[] = { 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12 };
  ASSERT_EQ(0, strcmp("object", JasonTypeName(JasonSlice(&objects[0]).type())));
  ASSERT_EQ(0, strcmp("object", JasonTypeName(JasonSlice(&objects[1]).type())));
  ASSERT_EQ(0, strcmp("object", JasonTypeName(JasonSlice(&objects[2]).type())));
  ASSERT_EQ(0, strcmp("object", JasonTypeName(JasonSlice(&objects[3]).type())));
  ASSERT_EQ(0, strcmp("object", JasonTypeName(JasonSlice(&objects[4]).type())));
  ASSERT_EQ(0, strcmp("object", JasonTypeName(JasonSlice(&objects[5]).type())));
  ASSERT_EQ(0, strcmp("object", JasonTypeName(JasonSlice(&objects[6]).type())));
  ASSERT_EQ(0, strcmp("object", JasonTypeName(JasonSlice(&objects[7]).type())));
  ASSERT_EQ(0, strcmp("object", JasonTypeName(JasonSlice(&objects[8]).type())));
}

int main (int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

