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

TEST(ValidatorTest, NoneValue) {
  std::string const value("\x00", 1);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, EmptyArrayValue) {
  std::string const value("\x01", 1);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, EmptyArrayValueWithExtra) {
  std::string const value("\x01\x02", 2);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, NullValue) {
  std::string const value("\x18", 1);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, FalseValue) {
  std::string const value("\x19", 1);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, TrueValue) {
  std::string const value("\x1a", 1);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, DoubleValue) {
  std::string const value("\x1b\x00\x00\x00\x00\x00\x00\x00\x00", 9);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, DoubleValueTruncated) {
  std::string const value("\x1b", 1);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, DoubleValueTooShort) {
  std::string const value("\x1b\x00\x00\x00\x00\x00\x00\x00", 8);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, DoubleValueTooLong) {
  std::string const value("\x1b\x00\x00\x00\x00\x00\x00\x00\x00\x00", 10);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, NullValueWithExtra) {
  std::string const value("\x18\x31", 2);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, StringEmpty) {
  std::string const value("\x40", 1);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, StringEmptyWithExtra) {
  std::string const value("\x40\x41", 2);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, StringValidLength) {
  std::string const value("\x43\x41\x42\x43", 4);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, StringLongerThanSpecified) {
  std::string const value("\x42\x41\x42\x43", 4);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, StringShorterThanSpecified) {
  std::string const value("\x43\x41\x42", 3);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, LongStringEmpty) {
  std::string const value("\xbf\x00\x00\x00\x00\x00\x00\x00\x00", 9);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, LongStringNonEmpty) {
  std::string const value("\xbf\x01\x00\x00\x00\x00\x00\x00\x00\x41", 10);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, LongStringTooShort) {
  std::string const value("\xbf", 1);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, LongStringShorterThanSpecified1) {
  std::string const value("\xbf\x01\x00\x00\x00\x00\x00\x00\x00", 9);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, LongStringShorterThanSpecified2) {
  std::string const value("\xbf\x03\x00\x00\x00\x00\x00\x00\x00\x41\x42", 11);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, LongStringLongerThanSpecified1) {
  std::string const value("\xbf\x00\x00\x00\x00\x00\x00\x00\x00\x41", 10);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, LongStringLongerThanSpecified2) {
  std::string const value("\xbf\x01\x00\x00\x00\x00\x00\x00\x00\x41\x42", 11);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
