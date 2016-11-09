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
#include <iostream>

#include "tests-common.h"

TEST(BufferTest, CreateEmpty) {
  Buffer<uint8_t> buffer;

  ASSERT_EQ(0UL, buffer.size());
  ASSERT_EQ(0UL, buffer.length());
  ASSERT_EQ(0UL, buffer.byteSize());
}

TEST(BufferTest, CreateAndAppend) {
  std::string const value("this is a test string");
  Buffer<uint8_t> buffer;

  buffer.append(value.c_str(), value.size());
  ASSERT_EQ(value.size(), buffer.size());
  ASSERT_EQ(value.size(), buffer.length());
  ASSERT_EQ(value.size(), buffer.byteSize());
}

TEST(BufferTest, CreateAndAppendLong) {
  std::string const value("this is a test string");
  Buffer<uint8_t> buffer;

  for (size_t i = 0; i < 1000; ++i) {
    buffer.append(value.c_str(), value.size());
  }

  ASSERT_EQ(1000 * value.size(), buffer.size());
  ASSERT_EQ(1000 * value.size(), buffer.length());
  ASSERT_EQ(1000 * value.size(), buffer.byteSize());
}

TEST(BufferTest, CopyConstruct) {
  std::string const value("this is a test string");
  Buffer<char> buffer;
  buffer.append(value.c_str(), value.size());

  Buffer<char> buffer2(buffer);
  ASSERT_EQ(value.size(), buffer2.size());
  ASSERT_EQ(buffer.size(), buffer2.size());
  ASSERT_EQ(value, std::string(buffer2.data(), buffer2.size()));
  ASSERT_NE(buffer.data(), buffer2.data());
}

TEST(BufferTest, CopyConstructLongValue) {
  std::string const value("this is a test string");
  
  Buffer<char> buffer;
  for (size_t i = 0; i < 1000; ++i) {
    buffer.append(value.c_str(), value.size());
  }

  Buffer<char> buffer2(buffer);
  ASSERT_EQ(1000 * value.size(), buffer2.size());
  ASSERT_EQ(buffer.size(), buffer2.size());
  ASSERT_NE(buffer.data(), buffer2.data());
}

TEST(BufferTest, CopyAssign) {
  std::string const value("this is a test string");
  Buffer<char> buffer;
  buffer.append(value.c_str(), value.size());

  Buffer<char> buffer2;
  buffer2 = buffer;
  ASSERT_EQ(value.size(), buffer2.size());
  ASSERT_EQ(buffer.size(), buffer2.size());
  ASSERT_EQ(value, std::string(buffer2.data(), buffer2.size()));
  ASSERT_NE(buffer.data(), buffer2.data());
}

TEST(BufferTest, CopyAssignLongValue) {
  std::string const value("this is a test string");

  Buffer<char> buffer;
  for (size_t i = 0; i < 1000; ++i) {
    buffer.append(value.c_str(), value.size());
  }

  Buffer<char> buffer2;
  buffer2 = buffer;
  ASSERT_EQ(1000 * value.size(), buffer2.size());
  ASSERT_EQ(buffer.size(), buffer2.size());
  ASSERT_NE(buffer.data(), buffer2.data());
}

TEST(BufferTest, CopyAssignDiscardOwnValue) {
  std::string const value("this is a test string");

  Buffer<char> buffer;
  for (size_t i = 0; i < 1000; ++i) {
    buffer.append(value.c_str(), value.size());
  }

  Buffer<char> buffer2;
  for (size_t i = 0; i < 100; ++i) {
    buffer2.append(value.c_str(), value.size());
  }

  buffer2 = buffer;
  ASSERT_EQ(1000 * value.size(), buffer2.size());
  ASSERT_EQ(buffer.size(), buffer2.size());
  ASSERT_NE(buffer.data(), buffer2.data());
}

TEST(BufferTest, MoveConstruct) {
  std::string const value("this is a test string");
  Buffer<char> buffer;
  buffer.append(value.c_str(), value.size());

  Buffer<char> buffer2(std::move(buffer));
  ASSERT_EQ(value.size(), buffer2.size());
  ASSERT_EQ(0UL, buffer.size()); // should be empty
  ASSERT_EQ(value, std::string(buffer2.data(), buffer2.size()));
  ASSERT_NE(buffer.data(), buffer2.data());
}

TEST(BufferTest, MoveConstructLongValue) {
  std::string const value("this is a test string");
  
  Buffer<char> buffer;
  for (size_t i = 0; i < 1000; ++i) {
    buffer.append(value.c_str(), value.size());
  }

  Buffer<char> buffer2(std::move(buffer));
  ASSERT_EQ(1000 * value.size(), buffer2.size());
  ASSERT_EQ(0UL, buffer.size()); // should be empty
  ASSERT_NE(buffer.data(), buffer2.data());
}

TEST(BufferTest, MoveAssign) {
  std::string const value("this is a test string");
  Buffer<char> buffer;
  buffer.append(value.c_str(), value.size());

  Buffer<char> buffer2;
  buffer2 = std::move(buffer);
  ASSERT_EQ(value.size(), buffer2.size());
  ASSERT_EQ(0UL, buffer.size()); // should be empty
  ASSERT_EQ(value, std::string(buffer2.data(), buffer2.size()));
  ASSERT_NE(buffer.data(), buffer2.data());
}

TEST(BufferTest, MoveAssignLongValue) {
  std::string const value("this is a test string");
  
  Buffer<char> buffer;
  for (size_t i = 0; i < 1000; ++i) {
    buffer.append(value.c_str(), value.size());
  }

  Buffer<char> buffer2;
  buffer2 = std::move(buffer);
  ASSERT_EQ(1000 * value.size(), buffer2.size());
  ASSERT_EQ(0UL, buffer.size()); // should be empty
  ASSERT_NE(buffer.data(), buffer2.data());
}

TEST(BufferTest, MoveAssignDiscardOwnValue) {
  std::string const value("this is a test string");
  
  Buffer<char> buffer;
  for (size_t i = 0; i < 1000; ++i) {
    buffer.append(value.c_str(), value.size());
  }

  Buffer<char> buffer2;
  for (size_t i = 0; i < 100; ++i) {
    buffer2.append(value.c_str(), value.size());
  }

  buffer2 = std::move(buffer);
  ASSERT_EQ(1000 * value.size(), buffer2.size());
  ASSERT_EQ(0UL, buffer.size()); // should be empty
  ASSERT_NE(buffer.data(), buffer2.data());
}

TEST(BufferTest, SizeEmpty) {
  Buffer<uint8_t> buffer;

  ASSERT_EQ(0UL, buffer.size());
  ASSERT_EQ(0UL, buffer.length());
  ASSERT_EQ(0UL, buffer.byteSize());
  ASSERT_TRUE(buffer.empty());
}

TEST(BufferTest, SizeNonEmpty) {
  Buffer<uint8_t> buffer;
  buffer.append("foobar", 6);

  ASSERT_EQ(6UL, buffer.size());
  ASSERT_EQ(6UL, buffer.length());
  ASSERT_EQ(6UL, buffer.byteSize());
  ASSERT_TRUE(!buffer.empty());
}
   
TEST(BufferTest, SizeAfterClear) {
  Buffer<uint8_t> buffer;
  buffer.append("foobar", 6);

  buffer.clear();
  ASSERT_EQ(0UL, buffer.size());
  ASSERT_EQ(0UL, buffer.length());
  ASSERT_EQ(0UL, buffer.byteSize());
  ASSERT_TRUE(buffer.empty());
}

TEST(BufferTest, SizeAfterReset) {
  Buffer<uint8_t> buffer;
  buffer.append("foobar", 6);

  buffer.reset();
  ASSERT_EQ(0UL, buffer.size());
  ASSERT_EQ(0UL, buffer.length());
  ASSERT_EQ(0UL, buffer.byteSize());
  ASSERT_TRUE(buffer.empty());
}

TEST(BufferTest, VectorTest) {
  std::vector<Buffer<uint8_t>> buffers;

  Builder builder;
  builder.add(Value("der hund, der ist so bunt"));
  
  Slice s = builder.slice();
  ASSERT_TRUE(s.isString());
  Buffer<uint8_t> b;
  b.append(s.start(), s.byteSize());

  buffers.push_back(b);
  
  Buffer<uint8_t>& last = buffers.back();
  Slice copy(last.data());
  ASSERT_TRUE(copy.isString());
  ASSERT_TRUE(copy.equals(s));
  ASSERT_EQ("der hund, der ist so bunt", copy.copyString());
}

TEST(BufferTest, VectorMoveTest) {
  std::vector<Buffer<uint8_t>> buffers;

  Builder builder;
  builder.add(Value("der hund, der ist so bunt"));
  
  Slice s = builder.slice();
  ASSERT_TRUE(s.isString());
  Buffer<uint8_t> b;
  b.append(s.start(), s.byteSize());

  buffers.push_back(std::move(b));
  
  Buffer<uint8_t>& last = buffers.back();
  Slice copy(last.data());
  ASSERT_TRUE(copy.isString());
  ASSERT_TRUE(copy.equals(s));
  ASSERT_EQ(0UL, b.byteSize());
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
