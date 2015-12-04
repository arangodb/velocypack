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

TEST(HexDumpTest, TestNull) {
  std::shared_ptr<Builder> b = Parser::fromJson("null");
  std::ostringstream out;
  out << HexDump(b->slice());

  ASSERT_EQ("0x18", out.str());
}

TEST(HexDumpTest, TestTrue) {
  std::shared_ptr<Builder> b = Parser::fromJson("true");
  std::ostringstream out;
  out << HexDump(b->slice());

  ASSERT_EQ("0x1a", out.str());
}

TEST(HexDumpTest, TestFalse) {
  std::shared_ptr<Builder> b = Parser::fromJson("false");
  std::ostringstream out;
  out << HexDump(b->slice());

  ASSERT_EQ("0x19", out.str());
}

TEST(HexDumpTest, TestArray) {
  std::shared_ptr<Builder> b = Parser::fromJson("[1,2,3,4,5,6,7,8,9,10]");
  std::ostringstream out;
  out << HexDump(b->slice());

  ASSERT_EQ(
      "0x06 0x18 0x0a 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38 0x39 0x28 0x0a "
      "0x03 0x04 \n0x05 0x06 0x07 0x08 0x09 0x0a 0x0b 0x0c",
      out.str());
}

TEST(HexDumpTest, TestValuesPerLine) {
  std::shared_ptr<Builder> b = Parser::fromJson("[1,2,3,4,5,6,7,8,9,10]");
  std::ostringstream out;
  out << HexDump(b->slice(), 4, " ");

  ASSERT_EQ(
      "0x06 0x18 0x0a 0x31 \n0x32 0x33 0x34 0x35 \n0x36 0x37 0x38 0x39 \n0x28 "
      "0x0a 0x03 0x04 \n0x05 0x06 0x07 0x08 \n0x09 0x0a 0x0b 0x0c",
      out.str());
}

TEST(HexDumpTest, TestSeparator) {
  std::shared_ptr<Builder> b = Parser::fromJson("[1,2,3,4,5,6,7,8,9,10]");
  std::ostringstream out;
  out << HexDump(b->slice(), 16, ", ");

  ASSERT_EQ(
      "0x06, 0x18, 0x0a, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, "
      "0x28, 0x0a, 0x03, 0x04, \n0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, "
      "0x0c",
      out.str());
}

TEST(HexDumpTest, TestEmptySeparator) {
  std::shared_ptr<Builder> b = Parser::fromJson("[1,2,3,4,5,6,7,8,9,10]");
  std::ostringstream out;
  out << HexDump(b->slice(), 16, "");

  ASSERT_EQ(
      "0x060x180x0a0x310x320x330x340x350x360x370x380x390x280x0a0x030x04\n0x050x"
      "060x070x080x090x0a0x0b0x0c",
      out.str());
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
