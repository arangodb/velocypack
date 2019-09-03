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
#include <regex>

#include "tests-common.h"

TEST(StringRefTest, CopyStringRef) {
  StringRef s("the-quick-brown-dog");
  StringRef copy(s);

  ASSERT_EQ(19, copy.size());
  ASSERT_EQ(s.data(), copy.data());
  ASSERT_TRUE(s.equals(copy));
  ASSERT_EQ(0, s.compare(copy));
  ASSERT_EQ('t', s.front());
  ASSERT_EQ('t', copy.front());
  ASSERT_EQ('g', s.back());
  ASSERT_EQ('g', copy.back());
}

TEST(StringRefTest, MoveStringRef) {
  StringRef s("the-quick-brown-dog");
  StringRef copy(std::move(s));

  ASSERT_EQ(19, copy.size());
  ASSERT_EQ(s.data(), copy.data());
  ASSERT_TRUE(s.equals(copy));
  ASSERT_EQ(0, s.compare(copy));
  ASSERT_EQ('t', s.front());
  ASSERT_EQ('t', copy.front());
  ASSERT_EQ('g', s.back());
  ASSERT_EQ('g', copy.back());
}

TEST(StringRefTest, CopyAssignStringRef) {
  StringRef s("the-quick-brown-dog");
  StringRef copy("some-rubbish");
 
  ASSERT_EQ(12, copy.size());
    
  copy = s;

  ASSERT_EQ(19, copy.size());
  ASSERT_EQ(s.data(), copy.data());
  ASSERT_TRUE(s.equals(copy));
  ASSERT_EQ(0, s.compare(copy));
  ASSERT_EQ('t', s.front());
  ASSERT_EQ('t', copy.front());
  ASSERT_EQ('g', s.back());
  ASSERT_EQ('g', copy.back());
}

TEST(StringRefTest, MoveAssignStringRef) {
  StringRef s("the-quick-brown-dog");
  StringRef copy("some-rubbish");
 
  ASSERT_EQ(12, copy.size());
    
  copy = std::move(s);

  ASSERT_EQ(19, copy.size());
  ASSERT_EQ(s.data(), copy.data());
  ASSERT_TRUE(s.equals(copy));
  ASSERT_EQ(0, s.compare(copy));
  ASSERT_EQ('t', s.front());
  ASSERT_EQ('t', copy.front());
  ASSERT_EQ('g', s.back());
  ASSERT_EQ('g', copy.back());
}

TEST(StringRefTest, EmptyStringRef) {
  StringRef s;

  ASSERT_TRUE(s.empty());
  ASSERT_EQ(0U, s.size());
  ASSERT_EQ("", s.toString());

  ASSERT_TRUE(s.equals(StringRef()));
  ASSERT_TRUE(s.equals(s));
  ASSERT_EQ(0, s.compare(s));
  ASSERT_EQ(0, s.compare(StringRef()));
}

TEST(StringRefTest, StringRefFromEmptyString) {
  std::string const value;
  StringRef s(value);

  ASSERT_TRUE(s.empty());
  ASSERT_EQ(0U, s.size());
  ASSERT_EQ("", s.toString());

  ASSERT_TRUE(s.equals(StringRef()));
  ASSERT_TRUE(s.equals(s));
  ASSERT_EQ(0, s.compare(s));
  ASSERT_EQ(0, s.compare(StringRef(value)));
}

TEST(StringRefTest, StringRefFromString) {
  std::string const value("the-quick-brown-foxx");
  StringRef s(value);

  ASSERT_TRUE(!s.empty());
  ASSERT_EQ(20U, s.size());
  ASSERT_EQ("the-quick-brown-foxx", s.toString());
  ASSERT_EQ(value.data(), s.data());

  ASSERT_TRUE(s.equals(StringRef(value)));
  ASSERT_TRUE(s.equals(s));
  ASSERT_EQ(0, s.compare(s));
  ASSERT_EQ(0, s.compare(StringRef(value)));
}

TEST(StringRefTest, StringRefFromStringWithNullByte) {
  std::string const value("the-quick\0brown-foxx", 20);
  StringRef s(value);

  ASSERT_TRUE(!s.empty());
  ASSERT_EQ(20U, s.size());
  ASSERT_EQ(std::string("the-quick\0brown-foxx", 20), s.toString());

  ASSERT_TRUE(s.equals(StringRef(value)));
  ASSERT_TRUE(s.equals(s));
  ASSERT_EQ(0, s.compare(s));
  ASSERT_EQ(0, s.compare(StringRef(value)));
}

TEST(StringRefTest, StringRefFromCharLength) {
  char const* value = "the-quick\nbrown-foxx";
  StringRef s(value, 20);

  ASSERT_TRUE(!s.empty());
  ASSERT_EQ(20U, s.size());
  ASSERT_EQ(std::string("the-quick\nbrown-foxx", 20), s.toString());

  ASSERT_TRUE(s.equals(StringRef(value, 20)));
  ASSERT_TRUE(s.equals(s));
  ASSERT_EQ(0, s.compare(s));
  ASSERT_EQ(0, s.compare(StringRef(value, 20)));
}

TEST(StringRefTest, StringRefFromCharLengthWithNullByte) {
  char const* value = "the-quick\0brown-foxx";
  StringRef s(value, 20);

  ASSERT_TRUE(!s.empty());
  ASSERT_EQ(20U, s.size());
  ASSERT_EQ(std::string("the-quick\0brown-foxx", 20), s.toString());

  ASSERT_TRUE(s.equals(StringRef(value, 20)));
  ASSERT_TRUE(s.equals(s));
  ASSERT_EQ(0, s.compare(s));
  ASSERT_EQ(0, s.compare(StringRef(value, 20)));
}

TEST(StringRefTest, StringRefFromNullTerminatedEmpty) {
  char const* value = "";
  StringRef s(value);

  ASSERT_TRUE(s.empty());
  ASSERT_EQ(0U, s.size());
  ASSERT_EQ("", s.toString());
  ASSERT_EQ(value, s.data());

  ASSERT_TRUE(s.equals(StringRef(value)));
  ASSERT_TRUE(s.equals(s));
  ASSERT_EQ(0, s.compare(s));
  ASSERT_EQ(0, s.compare(StringRef(value)));
}

TEST(StringRefTest, StringRefFromNullTerminated) {
  char const* value = "the-quick-brown-foxx";
  StringRef s(value);

  ASSERT_TRUE(!s.empty());
  ASSERT_EQ(20U, s.size());
  ASSERT_EQ("the-quick-brown-foxx", s.toString());
  ASSERT_EQ(value, s.data());

  ASSERT_TRUE(s.equals(StringRef(value)));
  ASSERT_TRUE(s.equals(s));
  ASSERT_EQ(0, s.compare(s));
  ASSERT_EQ(0, s.compare(StringRef(value)));
}

TEST(StringRefTest, StringRefFromEmptyStringSlice) {
  Builder b;
  b.add(Value(""));
  StringRef s(b.slice());

  ASSERT_TRUE(s.empty());
  ASSERT_EQ(0U, s.size());
  ASSERT_EQ("", s.toString());

  ASSERT_TRUE(s.equals(StringRef()));
  ASSERT_TRUE(s.equals(s));
  ASSERT_EQ(0, s.compare(s));
  ASSERT_EQ(0, s.compare(""));
}

TEST(StringRefTest, StringRefFromStringSlice) {
  Builder b;
  b.add(Value("the-quick-brown-foxx"));
  StringRef s(b.slice());
  
  ASSERT_TRUE(!s.empty());
  ASSERT_EQ(20U, s.size());
  ASSERT_EQ("the-quick-brown-foxx", s.toString());

  ASSERT_TRUE(s.equals(s));
  ASSERT_EQ(0, s.compare(s));
  ASSERT_EQ(0, s.compare("the-quick-brown-foxx"));
}

#ifndef VELOCYPACK_DEBUG
TEST(StringRefTest, StringRefFromNonStringSlice) {
  Builder b;
  b.add(Value(123));
  
  ASSERT_VELOCYPACK_EXCEPTION(StringRef(b.slice()), Exception::InvalidValueType);
}
#endif

TEST(StringRefTest, CharacterAccess) {
  std::string const value("the-quick-brown-foxx");
  StringRef s(value);

  ASSERT_EQ('t', s.front());
  ASSERT_EQ('x', s.back());

  for (std::size_t i = 0; i < value.size(); ++i) {
    ASSERT_EQ(value[i], s[i]);
    ASSERT_EQ(value.at(i), s.at(i));
  }
  
  ASSERT_EQ('x', s.at(19));
  ASSERT_VELOCYPACK_EXCEPTION(s.at(20), Exception::IndexOutOfBounds);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(21), Exception::IndexOutOfBounds);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(100), Exception::IndexOutOfBounds);
  ASSERT_VELOCYPACK_EXCEPTION(s.at(10000), Exception::IndexOutOfBounds);
  ASSERT_VELOCYPACK_EXCEPTION(StringRef().at(0), Exception::IndexOutOfBounds);
  ASSERT_VELOCYPACK_EXCEPTION(StringRef().at(1), Exception::IndexOutOfBounds);
  ASSERT_VELOCYPACK_EXCEPTION(StringRef().at(2), Exception::IndexOutOfBounds);
}

TEST(StringRefTest, Substr) {
  std::string const value("the-quick-brown-foxx");
  StringRef s(value);

  ASSERT_TRUE(StringRef().equals(s.substr(0, 0)));
  ASSERT_TRUE(StringRef("t").equals(s.substr(0, 1)));
  ASSERT_TRUE(StringRef("th").equals(s.substr(0, 2)));
  ASSERT_TRUE(StringRef("the").equals(s.substr(0, 3)));
  ASSERT_TRUE(StringRef("the-").equals(s.substr(0, 4)));
  ASSERT_TRUE(StringRef("the-quick-brown").equals(s.substr(0, 15)));
  ASSERT_TRUE(StringRef("the-quick-brown-fox").equals(s.substr(0, 19)));
  ASSERT_TRUE(StringRef("the-quick-brown-foxx").equals(s.substr(0, 20)));
  ASSERT_TRUE(StringRef("the-quick-brown-foxx").equals(s.substr(0, 21)));
  ASSERT_TRUE(StringRef("the-quick-brown-foxx").equals(s.substr(0, 1024)));

  ASSERT_TRUE(StringRef().equals(s.substr(1, 0)));
  ASSERT_TRUE(StringRef("h").equals(s.substr(1, 1)));
  ASSERT_TRUE(StringRef("he").equals(s.substr(1, 2)));
  ASSERT_TRUE(StringRef("he-").equals(s.substr(1, 3)));
  ASSERT_TRUE(StringRef("he-quick-brown-fox").equals(s.substr(1, 18)));
  ASSERT_TRUE(StringRef("he-quick-brown-foxx").equals(s.substr(1, 19)));
  ASSERT_TRUE(StringRef("he-quick-brown-foxx").equals(s.substr(1, 1024)));
  
  ASSERT_TRUE(StringRef().equals(s.substr(18, 0)));
  ASSERT_TRUE(StringRef("x").equals(s.substr(18, 1)));
  ASSERT_TRUE(StringRef("xx").equals(s.substr(18, 2)));
  ASSERT_TRUE(StringRef("xx").equals(s.substr(18, 3)));
  ASSERT_TRUE(StringRef("xx").equals(s.substr(18, 1024)));
  
  ASSERT_TRUE(StringRef("").equals(s.substr(19, 0)));
  ASSERT_TRUE(StringRef("x").equals(s.substr(19, 1)));
  ASSERT_TRUE(StringRef("x").equals(s.substr(19, 2)));
  ASSERT_TRUE(StringRef("x").equals(s.substr(19, 1024)));
  
  ASSERT_TRUE(StringRef("").equals(s.substr(20, 0)));
  ASSERT_TRUE(StringRef("").equals(s.substr(20, 1)));
  ASSERT_TRUE(StringRef("").equals(s.substr(20, 2)));
  ASSERT_TRUE(StringRef("").equals(s.substr(20, 1024)));

  ASSERT_VELOCYPACK_EXCEPTION(s.substr(21, 0), Exception::IndexOutOfBounds);
  ASSERT_VELOCYPACK_EXCEPTION(s.substr(21, 1), Exception::IndexOutOfBounds);
  ASSERT_VELOCYPACK_EXCEPTION(s.substr(21, 1024), Exception::IndexOutOfBounds);
}

TEST(StringRefTest, PopBack) {
  std::string const value("the-quick-brown-foxx");
  StringRef s(value);

  s.pop_back();
  ASSERT_TRUE(s.equals("the-quick-brown-fox"));
  s.pop_back();
  ASSERT_TRUE(s.equals("the-quick-brown-fo"));
  s.pop_back();
  ASSERT_TRUE(s.equals("the-quick-brown-f"));
  s.pop_back();
  ASSERT_TRUE(s.equals("the-quick-brown-"));

  s = "foo";
  ASSERT_TRUE(s.equals("foo"));
  s.pop_back();
  ASSERT_TRUE(s.equals("fo"));
  s.pop_back();
  ASSERT_TRUE(s.equals("f"));
  s.pop_back();
  ASSERT_TRUE(s.equals(""));
}

TEST(StringRefTest, Find) {
  std::string const value("the-quick-brown-foxx");
  StringRef s(value);

  for (std::size_t i = 0; i < 256; ++i) {
    ASSERT_EQ(value.find(static_cast<char>(i)), s.find(static_cast<char>(i)));
  }
}

TEST(StringRefTest, RFind) {
  std::string const value("the-quick-brown-foxx");
  StringRef s(value);

  for (std::size_t i = 0; i < 256; ++i) {
    ASSERT_EQ(value.rfind(static_cast<char>(i)), s.rfind(static_cast<char>(i)));
  }
}

TEST(StringRefTest, IteratorBeginEnd) {
  std::string const value("the-quick-brown-foxx");
  StringRef const s(value);

  auto it = s.begin();
  ASSERT_EQ('t', *it);
  ++it;
  ASSERT_EQ('h', *it);
  ++it;
  ASSERT_EQ('e', *it);
  
  it = s.end();
  --it;
  ASSERT_EQ('x', *it);
  --it;
  ASSERT_EQ('x', *it);
  --it;
  ASSERT_EQ('o', *it);
  --it;
  ASSERT_EQ('f', *it);
}

TEST(StringRefTest, IteratorStl) {
  std::string const value("the-quick-brown-foxx");
  StringRef const s(value);

  std::string result;
  std::for_each(s.begin(), s.end(), [&result](char v) {
    result.push_back(v);
  });

  ASSERT_TRUE(s.equals(result));
}

TEST(StringRefTest, IteratorRegex) {
  std::string const value("the-quick-brown-foxx");
  StringRef const s(value);

  ASSERT_TRUE(std::regex_match(s.begin(), s.end(), std::regex(".*fox.*")));
}

TEST(StringRefTest, IteratorRegexMatch) {
  std::string const value("the-quick-brown-foxx");
  StringRef const s(value);

  std::match_results<char const*> matches;
  ASSERT_TRUE(std::regex_match(s.begin(), s.end(), matches, std::regex(".*fox.*")));
}

TEST(StringRefTest, Equals) {
  StringRef const s("the-quick-brown-foxx");

  ASSERT_TRUE(s.equals("the-quick-brown-foxx"));
  ASSERT_FALSE(s.equals("the-quick-brown-foxx "));
  ASSERT_FALSE(s.equals("the-quick-brown-foxxy"));
  ASSERT_FALSE(s.equals("the-quick-brown-fox"));
  
  ASSERT_TRUE(s.equals(std::string("the-quick-brown-foxx")));
  ASSERT_FALSE(s.equals(std::string("the-quick-brown-foxx ")));
  ASSERT_FALSE(s.equals(std::string("the-quick-brown-foxxy")));
  ASSERT_FALSE(s.equals(std::string("the-quick-brown-fox")));
  
  ASSERT_TRUE(s.equals(StringRef("the-quick-brown-foxx")));
  ASSERT_FALSE(s.equals(StringRef("the-quick-brown-foxx ")));
  ASSERT_FALSE(s.equals(StringRef("the-quick-brown-foxxy")));
  ASSERT_FALSE(s.equals(StringRef("the-quick-brown-fox")));
}

TEST(StringRefTest, EqualsEmpty) {
  StringRef const s("");

  ASSERT_TRUE(s.equals(""));
  ASSERT_FALSE(s.equals(" "));
  ASSERT_FALSE(s.equals("0"));
  
  ASSERT_TRUE(s.equals(std::string("")));
  ASSERT_FALSE(s.equals(std::string(" ")));
  ASSERT_FALSE(s.equals(std::string("0")));
  
  ASSERT_TRUE(s.equals(StringRef("")));
  ASSERT_FALSE(s.equals(StringRef(" ")));
  ASSERT_FALSE(s.equals(StringRef("0")));
}

TEST(StringRefTest, Compare) {
  StringRef const s("the-quick-brown-foxx");

  ASSERT_TRUE(s.compare("the-quick-brown-foxx") == 0);
  ASSERT_TRUE(s.compare("the-quick-brown-foxx ") < 0);
  ASSERT_TRUE(s.compare("the-quick-brown-foxxy") < 0);
  ASSERT_TRUE(s.compare("the-quick-brown-fox") > 0);
  ASSERT_TRUE(s.compare("The-quick-brown-fox") > 0);
  ASSERT_TRUE(s.compare("she-quick-brown-fox") > 0);
  ASSERT_TRUE(s.compare("uhe-quick-brown-fox") < 0);
  ASSERT_TRUE(s.compare("") > 0);
  ASSERT_TRUE(s.compare("~") < 0);
  ASSERT_TRUE(s.compare(s) == 0);
  
  ASSERT_TRUE(s.compare(StringRef("", 0)) > 0);
  ASSERT_TRUE(s.compare(StringRef("\0", 1)) > 0);
  ASSERT_TRUE(s.compare(StringRef("\t", 1)) > 0);
  ASSERT_TRUE(s.compare(StringRef(" ", 1)) > 0);
  ASSERT_TRUE(s.compare(StringRef("@", 1)) > 0);
  ASSERT_TRUE(s.compare(StringRef("~", 1)) < 0);
  
  ASSERT_TRUE(s.compare(StringRef("the-quick-brown-foxx")) == 0);
  ASSERT_TRUE(s.compare(StringRef("the-quick-brown-foxx ")) < 0);
  ASSERT_TRUE(s.compare(StringRef("the-quick-brown-foxx ")) < 0);
  ASSERT_TRUE(s.compare(StringRef("the-quick-brown-fox")) > 0);
  ASSERT_TRUE(s.compare(StringRef("The-quick-brown-fox")) > 0);
  ASSERT_TRUE(s.compare(StringRef("she-quick-brown-fox")) > 0);
  ASSERT_TRUE(s.compare(StringRef("uhe-quick-brown-fox")) < 0);
}

TEST(StringRefTest, CompareEmpty) {
  StringRef s;

  ASSERT_TRUE(s.compare("the-quick-brown-foxx") < 0);
  ASSERT_TRUE(s.compare("the-quick-brown-foxx ") < 0);
  ASSERT_TRUE(s.compare("the-quick-brown-foxxy") < 0);
  ASSERT_TRUE(s.compare("the-quick-brown-fox") < 0);
  ASSERT_TRUE(s.compare("The-quick-brown-fox") < 0);
  ASSERT_TRUE(s.compare("she-quick-brown-fox") < 0);
  ASSERT_TRUE(s.compare("uhe-quick-brown-fox") < 0);
  ASSERT_TRUE(s.compare("") == 0);
  ASSERT_TRUE(s.compare(" ") < 0);
  ASSERT_TRUE(s.compare("\t") < 0);
  ASSERT_TRUE(s.compare("@") < 0);
  ASSERT_TRUE(s.compare("~") < 0);
  
  ASSERT_TRUE(s.compare(StringRef("", 0)) == 0);
  ASSERT_TRUE(s.compare(StringRef("\0", 1)) < 0);
  ASSERT_TRUE(s.compare(StringRef("\t", 1)) < 0);
  ASSERT_TRUE(s.compare(StringRef(" ", 1)) < 0);
  ASSERT_TRUE(s.compare(StringRef("@", 1)) < 0);
  ASSERT_TRUE(s.compare(StringRef("~", 1)) < 0);
}

TEST(StringRefTest, ToStream) {
  StringRef s("the-quick-brown-foxx");

  std::stringstream out;
  out << s;

  ASSERT_EQ("the-quick-brown-foxx", out.str());
}

TEST(StringRefTest, ToStreamEmpty) {
  StringRef s;

  std::stringstream out;
  out << s;

  ASSERT_EQ("", out.str());
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
