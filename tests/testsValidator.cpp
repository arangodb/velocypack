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
  
TEST(ValidatorTest, NoOptions) {
  ASSERT_VELOCYPACK_EXCEPTION(Validator(nullptr), Exception::InternalError);
}

TEST(ValidatorTest, ReservedValue1) {
  std::string const value("\x15", 1);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidType);
}

TEST(ValidatorTest, ReservedValue2) {
  std::string const value("\x16", 1);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidType);
}

TEST(ValidatorTest, ReservedValue3) {
  std::string const value("\xd8", 1);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidType);
}

TEST(ValidatorTest, NoneValue) {
  std::string const value("\x00", 1);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, NullValue) {
  std::string const value("\x18", 1);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, NullValueWithExtra) {
  std::string const value("\x18\x41", 2);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, FalseValue) {
  std::string const value("\x19", 1);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, FalseValueWithExtra) {
  std::string const value("\x19\x41", 2);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, TrueValue) {
  std::string const value("\x1a", 1);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, TrueValueWithExtra) {
  std::string const value("\x1a\x41", 2);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, Illegal) {
  std::string const value("\x17", 1);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, IllegalWithExtra) {
  std::string const value("\x17\x41", 2);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, MinKey) {
  std::string const value("\x1e", 1);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, MinKeyWithExtra) {
  std::string const value("\x1e\x41", 2);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, MaxKey) {
  std::string const value("\x1f", 1);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, MaxKeyWithExtra) {
  std::string const value("\x1f\x41", 2);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
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

TEST(ValidatorTest, UTCDate) {
  std::string const value("\x1c\x00\x00\x00\x00\x00\x00\x00\x00", 9);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, UTCDateTruncated) {
  std::string const value("\x1c", 1);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, UTCDateTooShort) {
  std::string const value("\x1c\x00\x00\x00\x00\x00\x00\x00", 8);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, UTCDateTooLong) {
  std::string const value("\x1c\x00\x00\x00\x00\x00\x00\x00\x00\x00", 10);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, SmallInt) {
  for (uint8_t i = 0; i <= 9; ++i) {
    std::string value;
    value.push_back(0x30 + i);

    Validator validator;
    ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
  }
}

TEST(ValidatorTest, SmallIntWithExtra) {
  std::string const value("\x30\x41", 2);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, SmallIntNegative) {
  for (uint8_t i = 0; i <= 5; ++i) {
    std::string value;
    value.push_back(0x3a + i);

    Validator validator;
    ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
  }
}

TEST(ValidatorTest, SmallIntNegativeWithExtra) {
  std::string const value("\x3a\x41", 2);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, IntPositiveOneByte) {
  std::string const value("\x20\x00", 2);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, IntPositiveOneByteTooShort) {
  std::string const value("\x20", 1);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, IntPositiveOneByteWithExtra) {
  std::string const value("\x20\x00\x41", 3);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, IntPositiveTwoBytes) {
  std::string const value("\x21\x00\x00", 3);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, IntPositiveTwoBytesTooShort) {
  std::string const value("\x21", 1);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, IntPositiveTwoBytesWithExtra) {
  std::string const value("\x21\x00\x00\x41", 4);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, IntPositiveEightBytes) {
  std::string const value("\x27\x00\x00\x00\x00\x00\x00\x00\x00", 9);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, IntPositiveEightBytesTooShort) {
  std::string const value("\x27", 1);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, IntPositiveEightBytesWithExtra) {
  std::string const value("\x27\x00\x00\x00\x00\x00\x00\x00\x00\x41", 10);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, UIntPositiveOneByte) {
  std::string const value("\x28\x00", 2);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, UIntPositiveOneByteTooShort) {
  std::string const value("\x28", 1);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, UIntPositiveOneByteWithExtra) {
  std::string const value("\x28\x00\x41", 3);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, UIntPositiveTwoBytes) {
  std::string const value("\x29\x00\x00", 3);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, UIntPositiveTwoBytesTooShort) {
  std::string const value("\x29", 1);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, UIntPositiveTwoBytesWithExtra) {
  std::string const value("\x29\x00\x00\x41", 4);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, UIntPositiveEightBytes) {
  std::string const value("\x2f\x00\x00\x00\x00\x00\x00\x00\x00", 9);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, UIntPositiveEightBytesTooShort) {
  std::string const value("\x2f", 1);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, UIntPositiveEightBytesWithExtra) {
  std::string const value("\x2f\x00\x00\x00\x00\x00\x00\x00\x00\x41", 10);

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

TEST(ValidatorTest, StringValidUtf8Empty) {
  std::string const value("\x40", 1);

  Options options;
  options.validateUtf8Strings = true;
  Validator validator(&options);
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, StringValidUtf8OneByte) {
  std::string const value("\x41\x0a", 2);

  Options options;
  options.validateUtf8Strings = true;
  Validator validator(&options);
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, StringValidUtf8TwoBytes) {
  std::string const value("\x42\xc2\xa2", 3);

  Options options;
  options.validateUtf8Strings = true;
  Validator validator(&options);
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, StringValidUtf8ThreeBytes) {
  std::string const value("\x43\xe2\x82\xac", 4);

  Options options;
  options.validateUtf8Strings = true;
  Validator validator(&options);
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, StringValidUtf8FourBytes) {
  std::string const value("\x44\xf0\xa4\xad\xa2", 5);

  Options options;
  options.validateUtf8Strings = true;
  Validator validator(&options);
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, StringValidUtf8Long) {
  std::string const value("\xbf\x04\x00\x00\x00\x00\x00\x00\x00\x40\x41\x42\x43", 13);

  Options options;
  options.validateUtf8Strings = true;
  Validator validator(&options);
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, StringInvalidUtf8NoValidation) {
  std::string const value("\x41\xff", 2);

  Options options;
  options.validateUtf8Strings = false;
  Validator validator(&options);
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, StringInvalidUtf8WithValidation1) {
  std::string const value("\x41\x80", 2);

  Options options;
  options.validateUtf8Strings = true;
  Validator validator(&options);
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::InvalidUtf8Sequence);
}

TEST(ValidatorTest, StringInvalidUtf8WithValidation2) {
  std::string const value("\x41\xff", 2);

  Options options;
  options.validateUtf8Strings = true;
  Validator validator(&options);
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::InvalidUtf8Sequence);
}

TEST(ValidatorTest, StringInvalidUtf8WithValidation3) {
  std::string const value("\x42\xff\x70", 3);

  Options options;
  options.validateUtf8Strings = true;
  Validator validator(&options);
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::InvalidUtf8Sequence);
}

TEST(ValidatorTest, StringInvalidUtf8WithValidation4) {
  std::string const value("\x43\xff\xff\x07", 4);

  Options options;
  options.validateUtf8Strings = true;
  Validator validator(&options);
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::InvalidUtf8Sequence);
}

TEST(ValidatorTest, StringInvalidUtf8WithValidation5) {
  std::string const value("\x44\xff\xff\xff\x07", 5);

  Options options;
  options.validateUtf8Strings = true;
  Validator validator(&options);
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::InvalidUtf8Sequence);
}

TEST(ValidatorTest, StringInvalidUtf8Long) {
  std::string const value("\xbf\x04\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\x07", 13);

  Options options;
  options.validateUtf8Strings = true;
  Validator validator(&options);
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::InvalidUtf8Sequence);
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

TEST(ValidatorTest, ExternalAllowed) {
  std::string const value("\x1d\x00\x00\x00\x00\x00\x00\x00\x00", 9);

  Options options;
  options.disallowExternals = false;
  Validator validator(&options);
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, ExternalDisallowed) {
  std::string const value("\x1d\x00\x00\x00\x00\x00\x00\x00\x00", 9);

  Options options;
  options.disallowExternals = true;
  Validator validator(&options);
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::BuilderExternalsDisallowed);
}

TEST(ValidatorTest, ExternalWithExtra) {
  std::string const value("\x1d\x00\x00\x00\x00\x00\x00\x00\x00\x41", 10);

  Options options;
  options.disallowExternals = false;
  Validator validator(&options);
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, CustomOneByte) {
  std::string const value("\xf0\xff", 2);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, CustomOneByteTooShort) {
  std::string const value("\xf0", 1);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, CustomOneByteWithExtra) {
  std::string const value("\xf0\xff\x41", 3);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, CustomTwoBytes) {
  std::string const value("\xf1\xff\xff", 3);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, CustomTwoBytesTooShort) {
  std::string const value("\xf1", 1);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, CustomTwoBytesWithExtra) {
  std::string const value("\xf1\xff\xff\x41", 4);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, CustomFourBytes) {
  std::string const value("\xf2\xff\xff\xff\xff", 5);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, CustomFourBytesTooShort) {
  std::string const value("\xf2", 1);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, CustomFourBytesWithExtra) {
  std::string const value("\xf2\xff\xff\xff\xff\x41", 6);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, CustomEightBytes) {
  std::string const value("\xf3\xff\xff\xff\xff\xff\xff\xff\xff", 9);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, CustomEightBytesTooShort) {
  std::string const value("\xf3", 1);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, CustomEightBytesWithExtra) {
  std::string const value("\xf3\xff\xff\xff\xff\xff\xff\xff\xff\x41", 10);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, CustomOneByteF4) {
  std::string const value("\xf4\x01\xff", 3);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, CustomOneByteF4ZeroLength) {
  std::string const value("\xf4\x00", 2);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, CustomOneByteF4TooShort) {
  std::string const value("\xf4\x01", 2);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, CustomOneByteF4WithExtra) {
  std::string const value("\xf4\x01\xff\x41", 4);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, CustomOneByteF7) {
  std::string const value("\xf7\x01\x00\xff", 4);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, CustomOneByteF7ZeroLength) {
  std::string const value("\xf7\x00\x00", 3);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, CustomOneByteF7TooShort1) {
  std::string const value("\xf7\x01", 2);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, CustomOneByteF7TooShort2) {
  std::string const value("\xf7\x01\x00", 3);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, CustomOneByteF7WithExtra) {
  std::string const value("\xf7\x01\x00\xff\x41", 5);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, CustomOneByteFA) {
  std::string const value("\xfa\x01\x00\x00\x00\xff", 6);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, CustomOneByteFAZeroLength) {
  std::string const value("\xfa\x00\x00\x00\x00", 5);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, CustomOneByteFATooShort1) {
  std::string const value("\xfa\x01", 2);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, CustomOneByteFATooShort2) {
  std::string const value("\xfa\x01\x00", 3);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, CustomOneByteFATooShort3) {
  std::string const value("\xfa\x01\x00\x00", 4);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, CustomOneByteFATooShort4) {
  std::string const value("\xfa\x01\x00\x00\x00", 5);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, CustomOneByteFAWithExtra) {
  std::string const value("\xfa\x01\x00\x00\x00\xff\x41", 7);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, CustomOneByteFD) {
  std::string const value("\xfd\x01\x00\x00\x00\x00\x00\x00\x00\xff", 10);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, CustomOneByteFDZeroLength) {
  std::string const value("\xfd\x00\x00\x00\x00\x00\x00\x00\x00", 9);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, CustomOneByteFDTooShort1) {
  std::string const value("\xfd\x01", 2);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, CustomOneByteFDTooShort2) {
  std::string const value("\xfd\x01\x00", 3);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, CustomOneByteFDTooShort3) {
  std::string const value("\xfd\x01\x00\x00", 4);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, CustomOneByteFDTooShort4) {
  std::string const value("\xfd\x01\x00\x00\x00\x00\x00\x00", 7);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, CustomOneByteFDTooShort5) {
  std::string const value("\xfd\x01\x00\x00\x00\x00\x00\x00\x00", 8);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, CustomOneByteFDWithExtra) {
  std::string const value("\xfd\x01\x00\x00\x00\x00\x00\x00\x00\xff\x41", 11);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, BCD) {
  std::string const value("\xd0", 1);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::NotImplemented);
}

TEST(ValidatorTest, EmptyArray) {
  std::string const value("\x01", 1);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, EmptyArrayWithExtra) {
  std::string const value("\x01\x02", 2);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}










TEST(ValidatorTest, ArrayOneByte) {
  std::string const value("\x02\x03\x18", 3);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, ArrayOneByteTooShort) {
  std::string const value("\x02\x04\x18", 3);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayOneByteTooShortBytesize) {
  std::string const value("\x02\x05", 2);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayOneByteMultipleMembers) {
  std::string const value("\x02\x05\x18\x18\x18", 5);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, ArrayOneByteTooFewMembers1) {
  std::string const value("\x02\x05\x18", 3);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayOneByteTooFewMembers2) {
  std::string const value("\x02\x05\x18\x18", 4);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayOneByteMultipleMembersDifferentSizes) {
  std::string const value("\x02\x05\x18\x28\x00", 5);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayTwoBytes) {
  std::string const value("\x03\x04\x00\x18", 4);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, ArrayTwoBytesTooShort) {
  std::string const value("\x03\x05\x00\x18", 4);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayTwoBytesTooShortBytesize1) {
  std::string const value("\x03\x05", 2);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayTwoBytesTooShortBytesize2) {
  std::string const value("\x03\x05\x00", 3);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayTwoBytesMultipleMembers) {
  std::string const value("\x03\x06\x00\x18\x18\x18", 6);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, ArrayTwoBytesTooFewMembers1) {
  std::string const value("\x03\x05\x00\x18", 4);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayTwoBytesTooFewMembers2) {
  std::string const value("\x03\x06\x00\x18\x18", 5);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayTwoBytesMultipleMembersDifferentSizes) {
  std::string const value("\x03\x05\x18\x28\x00", 5);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayOneByteIndexed) {
  std::string const value("\x06\x05\x01\x18\x03", 5);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, ArrayOneByteIndexedTooShort1) {
  std::string const value("\x06\x05", 2);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayOneByteIndexedTooShort2) {
  std::string const value("\x06\x05\x01", 3);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayOneByteIndexedTooShort3) {
  std::string const value("\x06\x05\x01\x18", 4);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayOneByteIndexedIndexOutOfBounds1) {
  std::string const value("\x06\x05\x01\x18\x04", 5);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayOneByteIndexedIndexOutOfBounds2) {
  std::string const value("\x06\x05\x01\x18\x05", 5);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayOneByteIndexedIndexOutOfBounds3) {
  std::string const value("\x06\x05\x01\x18\x00", 5);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayOneByteIndexedMultipleMembers) {
  std::string const value("\x06\x09\x02\x18\x18\x18\x03\x04\x05", 9);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, ArrayCompact) {
  std::string const value("\x13\x04\x18\x01", 4);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, ArrayCompactWithExtra) {
  std::string const value("\x13\x04\x18\x01\x41", 5);
 
  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayCompactTooShort1) {
  std::string const value("\x13\x04", 2);
 
  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayCompactTooShort2) {
  std::string const value("\x13\x04\x18", 3);
 
  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayCompactTooShort3) {
  std::string const value("\x13\x80", 2);
 
  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayCompactTooShort4) {
  std::string const value("\x13\x80\x80", 3);
 
  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayCompactTooShort5) {
  std::string const value("\x13\x80\x05\x18", 4);
 
  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayCompactTooShort6) {
  std::string const value("\x13\x04\x18\x02", 4);
 
  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayCompactTooShort7) {
  std::string const value("\x13\x04\x18\xff", 4);
 
  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayCompactTooShort8) {
  std::string const value("\x13\x04\x06\x01", 4);
 
  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayCompactTooShort9) {
  std::string const value("\x13\x81", 2);
 
  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayCompactEmpty) {
  std::string const value("\x13\x04\x18\x00", 4);
 
  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayCompactNrItemsWrong1) {
  std::string const value("\x13\x04\x18\x81", 4);
 
  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayCompactNrItemsWrong2) {
  std::string const value("\x13\x05\x18\x81\x81", 5);
 
  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayCompactNrItemsWrong3) {
  std::string const value("\x13\x05\x18\x01\x80", 5);
 
  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayEqualSize) {
  std::string const value("\x02\x04\x01\x18", 4);
 
  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, ArrayEqualSizeMultiple) {
  std::string const value("\x02\x04\x18\x18", 4);
 
  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, ArrayEqualSizeMultipleWithExtra) {
  std::string const value("\x02\x04\x18\x18\x41", 5);
 
  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayEqualSizeTooShort) {
  std::string const value("\x02\x05\x18\x18", 4);
 
  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayEqualSizeContainingNone) {
  std::string const value("\x02\x03\x00", 3);
 
  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, ArrayEqualSizeUnequalElements) {
  std::string const value("\x02\x05\x18\x41\x40", 5);
 
  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

TEST(ValidatorTest, EmptyObject) {
  std::string const value("\x0a", 1);

  Validator validator;
  ASSERT_TRUE(validator.validate(value.c_str(), value.size()));
}

TEST(ValidatorTest, EmptyObjectWithExtra) {
  std::string const value("\x0a\x02", 2);

  Validator validator;
  ASSERT_VELOCYPACK_EXCEPTION(validator.validate(value.c_str(), value.size()), Exception::ValidatorInvalidLength);
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
