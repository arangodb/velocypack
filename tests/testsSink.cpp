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

TEST(SinkTest, CharBufferSink) {
  CharBuffer out;
  CharBufferSink s(&out);

  ASSERT_TRUE(out.empty());

  s.push_back('x');
  ASSERT_EQ(1, out.size());

  out.clear();
  s.append(std::string("foobarbaz"));
  ASSERT_EQ(9, out.size());

  out.clear();
  s.append("foobarbaz");
  ASSERT_EQ(9, out.size());

  out.clear();
  s.append("foobarbaz", 9);
  ASSERT_EQ(9, out.size());
}

TEST(SinkTest, StringSink) {
  std::string out;
  StringSink s(&out);

  ASSERT_TRUE(out.empty());

  s.push_back('x');
  ASSERT_EQ(1, out.size());
  ASSERT_EQ("x", out);

  out.clear();
  s.append(std::string("foobarbaz"));
  ASSERT_EQ(9, out.size());
  ASSERT_EQ("foobarbaz", out);

  out.clear();
  s.append("foobarbaz");
  ASSERT_EQ(9, out.size());
  ASSERT_EQ("foobarbaz", out);

  out.clear();
  s.append("foobarbaz", 9);
  ASSERT_EQ(9, out.size());
  ASSERT_EQ("foobarbaz", out);
}

TEST(SinkTest, SizeConstrainedStringSinkAlwaysEmpty) {
  std::string out;
  SizeConstrainedStringSink s(&out, 0);

  ASSERT_TRUE(out.empty());
  ASSERT_FALSE(s.overflowed);

  s.push_back('x');
  ASSERT_TRUE(out.empty());
  ASSERT_TRUE(s.overflowed);

  s.append("foobarbaz");
  ASSERT_TRUE(out.empty());
  ASSERT_TRUE(s.overflowed);
  
  s.append("123", 3);
  ASSERT_TRUE(out.empty());
  ASSERT_TRUE(s.overflowed);
}

TEST(SinkTest, SizeConstrainedStringSinkSmall) {
  std::string out;
  SizeConstrainedStringSink s(&out, 15);

  ASSERT_TRUE(out.empty());
  ASSERT_FALSE(s.overflowed);

  s.push_back('x');
  ASSERT_EQ("x", out);
  ASSERT_FALSE(s.overflowed);

  s.append("foobarbaz");
  ASSERT_EQ("xfoobarbaz", out);
  ASSERT_FALSE(s.overflowed);
  
  s.append("123", 3);
  ASSERT_EQ("xfoobarbaz123", out);
  ASSERT_FALSE(s.overflowed);
  
  s.push_back('y');
  ASSERT_EQ("xfoobarbaz123y", out);
  ASSERT_FALSE(s.overflowed);
  
  s.append("123", 3);
  ASSERT_EQ("xfoobarbaz123y1", out);
  ASSERT_TRUE(s.overflowed);
}

TEST(SinkTest, SizeConstrainedStringSinkLarger) {
  std::string out;
  SizeConstrainedStringSink s(&out, 2048);

  ASSERT_TRUE(out.empty());
  ASSERT_FALSE(s.overflowed);

  for (std::size_t i = 0; i < 4096; ++i) {
    s.push_back('x');
    if (i >= 2048) {
      ASSERT_EQ(2048, out.size());
      ASSERT_TRUE(s.overflowed);
    } else {
      ASSERT_EQ(i + 1, out.size());
      ASSERT_FALSE(s.overflowed);
    }
  }
}

TEST(SinkTest, SizeConstrainedStringSinkLongStringAppend) {
  std::string out;
  SizeConstrainedStringSink s(&out, 2092);

  ASSERT_TRUE(out.empty());
  ASSERT_FALSE(s.overflowed);

  s.append("meow");
  ASSERT_EQ(4, out.size());
  ASSERT_FALSE(s.overflowed);

  std::string append(16384, 'x');
  s.append(append);
  ASSERT_EQ(2092, out.size());
  ASSERT_EQ(std::string("meow") + append.substr(0, 2088), out);
  ASSERT_TRUE(s.overflowed);
}
  
TEST(SinkTest, SizeConstrainedStringSinkReserve) {
  {
    std::string out;
    SizeConstrainedStringSink s(&out, 0);
    
    std::size_t oldCapacity = out.capacity();
    // should do nothing
    s.reserve(10);
    ASSERT_EQ(oldCapacity, out.capacity());
    
    s.reserve(128);
    ASSERT_EQ(oldCapacity, out.capacity());
    
    s.reserve(4096);
    ASSERT_EQ(oldCapacity, out.capacity());
  }
  
  {
    std::string out;
    SizeConstrainedStringSink s(&out, 4096);
    
    // should do something. however, we don't know
    // the exact capacity, as it depends on the internals
    // of std::string
    s.reserve(128);
    ASSERT_GE(out.capacity(), 128);
    
    std::size_t oldCapacity = out.capacity();
    // should not do anything
    s.reserve(128);
    ASSERT_EQ(oldCapacity, out.capacity());
    
    s.reserve(256);
    ASSERT_GE(out.capacity(), 256);
    
    s.reserve(4096);
    ASSERT_GE(out.capacity(), 4096);

    oldCapacity = out.capacity();
    for (std::size_t i = 0; i < 10; ++i) {
      s.reserve(128);
      // capacity should not have changed
      ASSERT_EQ(oldCapacity, out.capacity());
    }
  }
}

TEST(SinkTest, StringLengthSink) {
  StringLengthSink s;

  ASSERT_EQ(0, s.length);

  s.push_back('x');
  ASSERT_EQ(1, s.length);

  s.append(std::string("foobarbaz"));
  ASSERT_EQ(10, s.length);

  s.append("foobarbaz");
  ASSERT_EQ(19, s.length);

  s.append("foobarbaz", 9);
  ASSERT_EQ(28, s.length);
}

TEST(SinkTest, StringStreamSink) {
  std::ostringstream out;
  StringStreamSink s(&out);

  s.push_back('x');
  ASSERT_EQ("x", out.str());

  s.append(std::string("foobarbaz"));
  ASSERT_EQ("xfoobarbaz", out.str());

  s.append("foobarbaz");
  ASSERT_EQ("xfoobarbazfoobarbaz", out.str());

  s.append("foobarbaz", 9);
  ASSERT_EQ("xfoobarbazfoobarbazfoobarbaz", out.str());
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
