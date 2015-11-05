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

TEST(ParserTest, Garbage1) {
  std::string const value("z");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(0U, parser.errorPos());
}

TEST(ParserTest, Garbage2) {
  std::string const value("foo");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(1U, parser.errorPos());
}

TEST(ParserTest, Garbage3) {
  std::string const value("truth");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(3U, parser.errorPos());
}

TEST(ParserTest, Garbage4) {
  std::string const value("tru");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(2U, parser.errorPos());
}

TEST(ParserTest, Garbage5) {
  std::string const value("truebar");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(4U, parser.errorPos());
}

TEST(ParserTest, Garbage6) {
  std::string const value("fals");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(3U, parser.errorPos());
}

TEST(ParserTest, Garbage7) {
  std::string const value("falselaber");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(5U, parser.errorPos());
}

TEST(ParserTest, Garbage8) {
  std::string const value("zauberzauber");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(0U, parser.errorPos());
}

TEST(ParserTest, Garbage9) {
  std::string const value("true,");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(4U, parser.errorPos());
}

TEST(ParserTest, Punctuation1) {
  std::string const value(",");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(0U, parser.errorPos());
}

TEST(ParserTest, Punctuation2) {
  std::string const value("/");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(0U, parser.errorPos());
}

TEST(ParserTest, Punctuation3) {
  std::string const value("@");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(0U, parser.errorPos());
}

TEST(ParserTest, Punctuation4) {
  std::string const value(":");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(0U, parser.errorPos());
}

TEST(ParserTest, Punctuation5) {
  std::string const value("!");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(0U, parser.errorPos());
}

TEST(ParserTest, Null) {
  std::string const value("null");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Null, 1ULL);

  checkDump(s, value);
}

TEST(ParserTest, False) {
  std::string const value("false");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Bool, 1ULL);
  ASSERT_FALSE(s.getBool());

  checkDump(s, value);
}

TEST(ParserTest, True) {
  std::string const value("true");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Bool, 1ULL);
  ASSERT_TRUE(s.getBool());

  checkDump(s, value);
}

TEST(ParserTest, Zero) {
  std::string const value("0");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::SmallInt, 1ULL);
  ASSERT_EQ(0, s.getSmallInt());

  checkDump(s, value);
}

TEST(ParserTest, ZeroInvalid) {
  std::string const value("00");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(1u, parser.errorPos());
}

TEST(ParserTest, NumberIncomplete) {
  std::string const value("-");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(0u, parser.errorPos());
}

TEST(ParserTest, Int1) {
  std::string const value("1");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::SmallInt, 1ULL);
  ASSERT_EQ(1, s.getSmallInt());

  checkDump(s, value);
}

TEST(ParserTest, IntM1) {
  std::string const value("-1");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::SmallInt, 1ULL);
  ASSERT_EQ(-1LL, s.getSmallInt());

  checkDump(s, value);
}

TEST(ParserTest, Int2) {
  std::string const value("100000");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::UInt, 4ULL);
  ASSERT_EQ(100000ULL, s.getUInt());

  checkDump(s, value);
}

TEST(ParserTest, Int3) {
  std::string const value("-100000");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Int, 4ULL);
  ASSERT_EQ(-100000LL, s.getInt());

  checkDump(s, value);
}

TEST(ParserTest, UIntMaxNeg) {
  std::string value("-");
  value.append(std::to_string(UINT64_MAX));

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Double, 9ULL);
  // handle rounding errors
  ASSERT_DOUBLE_EQ(-18446744073709551615., s.getDouble());
}

TEST(ParserTest, IntMin) {
  std::string const value(std::to_string(INT64_MIN));

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Int, 9ULL);
  ASSERT_EQ(INT64_MIN, s.getInt());

  checkDump(s, value);
}

TEST(ParserTest, IntMinMinusOne) {
  std::string const value("-9223372036854775809"); // INT64_MIN - 1

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Double, 9ULL);
  ASSERT_DOUBLE_EQ(-9223372036854775809., s.getDouble());
}

TEST(ParserTest, IntMax) {
  std::string const value(std::to_string(INT64_MAX));

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::UInt, 9ULL);
  ASSERT_EQ(static_cast<uint64_t>(INT64_MAX), s.getUInt());

  checkDump(s, value);
}

TEST(ParserTest, IntMaxPlusOne) {
  std::string const value("9223372036854775808"); // INT64_MAX + 1

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::UInt, 9ULL);
  ASSERT_EQ(static_cast<uint64_t>(INT64_MAX) + 1, s.getUInt());

  checkDump(s, value);
}

TEST(ParserTest, UIntMax) {
  std::string const value(std::to_string(UINT64_MAX));

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::UInt, 9ULL);
  ASSERT_EQ(UINT64_MAX, s.getUInt());

  checkDump(s, value);
}

TEST(ParserTest, UIntMaxPlusOne) {
  std::string const value("18446744073709551616"); // UINT64_MAX + 1

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Double, 9ULL);
  ASSERT_DOUBLE_EQ(18446744073709551616., s.getDouble());
}

TEST(ParserTest, Double1) {
  std::string const value("1.0124");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Double, 9ULL);
  ASSERT_EQ(1.0124, s.getDouble());

  checkDump(s, value);
}

TEST(ParserTest, Double2) {
  std::string const value("-1.0124");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Double, 9ULL);
  ASSERT_EQ(-1.0124, s.getDouble());

  checkDump(s, value);
}

TEST(ParserTest, DoubleScientific1) {
  std::string const value("-1.0124e42");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Double, 9ULL);
  ASSERT_EQ(-1.0124e42, s.getDouble());

  std::string const valueOut("-1.0124e+42");
  checkDump(s, valueOut);
}

TEST(ParserTest, DoubleScientific2) {
  std::string const value("-1.0124e+42");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Double, 9ULL);
  ASSERT_EQ(-1.0124e42, s.getDouble());

  checkDump(s, value);
}

TEST(ParserTest, DoubleScientific3) {
  std::string const value("3122243.0124e-42");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Double, 9ULL);
  ASSERT_EQ(3122243.0124e-42, s.getDouble());

  std::string const valueOut("3.1222430124e-36");
  checkDump(s, valueOut);
}

TEST(ParserTest, DoubleScientific4) {
  std::string const value("2335431.0124E-42");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Double, 9ULL);
  ASSERT_EQ(2335431.0124E-42, s.getDouble());

  std::string const valueOut("2.3354310124e-36");
  checkDump(s, valueOut);
}

TEST(ParserTest, IntMinusInf) {
  std::string const value("-999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::NumberOutOfRange);
}

TEST(ParserTest, IntPlusInf) {
  std::string const value("999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::NumberOutOfRange);
}

TEST(ParserTest, DoubleMinusInf) {
  std::string const value("-1.2345e999");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::NumberOutOfRange);
}

TEST(ParserTest, DoublePlusInf) {
  std::string const value("1.2345e999");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::NumberOutOfRange);
}

TEST(ParserTest, Empty) {
  std::string const value("");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(0U, parser.errorPos());
}

TEST(ParserTest, WhitespaceOnly) {
  std::string const value("  ");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(1U, parser.errorPos());
}

TEST(ParserTest, UnterminatedStringLiteral) {
  std::string const value("\"der hund");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(8U, parser.errorPos());
}

TEST(ParserTest, StringLiteral) {
  std::string const value("\"der hund ging in den wald und aß den fuxx\"");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  std::string const correct = "der hund ging in den wald und aß den fuxx";
  checkBuild(s, JasonType::String, 1 + correct.size());
  char const* p = s.getString(len);
  ASSERT_EQ(correct.size(), len);
  ASSERT_EQ(0, strncmp(correct.c_str(), p, len));
  std::string out = s.copyString();
  ASSERT_EQ(correct, out);

  std::string valueOut = "\"der hund ging in den wald und aß den fuxx\"";
  checkDump(s, valueOut);
}

TEST(ParserTest, StringLiteralEmpty) {
  std::string const value("\"\"");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::String, 1ULL);
  char const* p = s.getString(len);
  ASSERT_EQ(0, strncmp("", p, len));
  ASSERT_EQ(0ULL, len);
  std::string out = s.copyString();
  std::string empty;
  ASSERT_EQ(empty, out);

  checkDump(s, value);
}

TEST(ParserTest, StringLiteralInvalidUtfValue1) {
  std::string value;
  value.push_back('"');
  value.push_back(static_cast<unsigned char>(0x80));
  value.push_back('"');

  JasonParser parser;
  parser.options.validateUtf8Strings = true;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::InvalidUtf8Sequence);
  ASSERT_EQ(1U, parser.errorPos());
  parser.options.validateUtf8Strings = false;
  ASSERT_EQ(1ULL, parser.parse(value));
}

TEST(ParserTest, StringLiteralInvalidUtfValue2) {
  std::string value;
  value.push_back('"');
  value.push_back(static_cast<unsigned char>(0xff));
  value.push_back(static_cast<unsigned char>(0xff));
  value.push_back('"');

  JasonParser parser;
  parser.options.validateUtf8Strings = true;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::InvalidUtf8Sequence);
  ASSERT_EQ(1U, parser.errorPos());
  parser.options.validateUtf8Strings = false;
  ASSERT_EQ(1ULL, parser.parse(value));
}

TEST(ParserTest, StringLiteralControlCharacter) {
  for (char c = 0; c < 0x20; c++) {
    std::string value;
    value.push_back('"');
    value.push_back(c);
    value.push_back('"');

    JasonParser parser;
    EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::UnexpectedControlCharacter);
    ASSERT_EQ(1U, parser.errorPos());
  }
}

TEST(ParserTest, StringLiteralUnfinishedUtfSequence1) {
  std::string const value("\"\\u\"");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(3U, parser.errorPos());
}

TEST(ParserTest, StringLiteralUnfinishedUtfSequence2) {
  std::string const value("\"\\u0\"");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(4U, parser.errorPos());
}

TEST(ParserTest, StringLiteralUnfinishedUtfSequence3) {
  std::string const value("\"\\u01\"");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(5U, parser.errorPos());
}

TEST(ParserTest, StringLiteralUnfinishedUtfSequence4) {
  std::string const value("\"\\u012\"");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(6U, parser.errorPos());
}

TEST(ParserTest, StringLiteralUtf8SequenceLowerCase) {
  std::string const value("\"der m\\u00d6ter\"");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::String, 11ULL);
  char const* p = s.getString(len);
  ASSERT_EQ(10ULL, len);
  std::string correct = "der m\xc3\x96ter";
  ASSERT_EQ(0, strncmp(correct.c_str(), p, len));
  std::string out = s.copyString();
  ASSERT_EQ(correct, out);

  std::string const valueOut("\"der mÖter\"");
  checkDump(s, valueOut);
}

TEST(ParserTest, StringLiteralUtf8SequenceUpperCase) {
  std::string const value("\"der m\\u00D6ter\"");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  std::string correct = "der mÖter";
  checkBuild(s, JasonType::String, 1 + correct.size());
  char const* p = s.getString(len);
  ASSERT_EQ(correct.size(), len);
  ASSERT_EQ(0, strncmp(correct.c_str(), p, len));
  std::string out = s.copyString();
  ASSERT_EQ(correct, out);

  checkDump(s, std::string("\"der mÖter\""));
}

TEST(ParserTest, StringLiteralUtf8Chars) {
  std::string const value("\"der mötör klötörte mät dän fößen\"");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  std::string correct = "der mötör klötörte mät dän fößen";
  checkBuild(s, JasonType::String, 1 + correct.size());
  char const* p = s.getString(len);
  ASSERT_EQ(correct.size(), len);
  ASSERT_EQ(0, strncmp(correct.c_str(), p, len));
  std::string out = s.copyString();
  ASSERT_EQ(correct, out);

  checkDump(s, value);
}

TEST(ParserTest, StringLiteralWithSpecials) {
  std::string const value("  \"der\\thund\\nging\\rin\\fden\\\\wald\\\"und\\b\\nden'fux\"  ");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  std::string correct = "der\thund\nging\rin\fden\\wald\"und\b\nden'fux";
  checkBuild(s, JasonType::String, 1 + correct.size());
  char const* p = s.getString(len);
  ASSERT_EQ(correct.size(), len);
  ASSERT_EQ(0, strncmp(correct.c_str(), p, len));
  std::string out = s.copyString();
  ASSERT_EQ(correct, out);

  std::string const valueOut("\"der\\thund\\nging\\rin\\fden\\\\wald\\\"und\\b\\nden'fux\"");
  checkDump(s, valueOut);
}

TEST(ParserTest, StringLiteralWithSurrogatePairs) {
  std::string const value("\"\\ud800\\udc00\\udbff\\udfff\\udbc8\\udf45\"");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  std::string correct = "\xf0\x90\x80\x80\xf4\x8f\xbf\xbf\xf4\x82\x8d\x85";
  checkBuild(s, JasonType::String, 1 + correct.size());
  char const* p = s.getString(len);
  ASSERT_EQ(correct.size(), len);
  ASSERT_EQ(0, strncmp(correct.c_str(), p, len));
  std::string out = s.copyString();
  ASSERT_EQ(correct, out);

  std::string const valueOut("\"\xf0\x90\x80\x80\xf4\x8f\xbf\xbf\xf4\x82\x8d\x85\"");
  checkDump(s, valueOut);
}

TEST(ParserTest, EmptyArray) {
  std::string const value("[]");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Array, 1);
  ASSERT_EQ(0ULL, s.length());

  checkDump(s, value);
}

TEST(ParserTest, WhitespacedArray) {
  std::string const value("  [    ]   ");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Array, 1);
  ASSERT_EQ(0ULL, s.length());

  std::string const valueOut = "[]";
  checkDump(s, valueOut);
}

TEST(ParserTest, Array1) {
  std::string const value("[1]");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Array, 3);
  ASSERT_EQ(1ULL, s.length());
  JasonSlice ss = s[0];
  checkBuild(ss, JasonType::SmallInt, 1);
  ASSERT_EQ(1ULL, ss.getUInt());

  checkDump(s, value);
}

TEST(ParserTest, Array2) {
  std::string const value("[1,2]");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Array, 4);
  ASSERT_EQ(2ULL, s.length());
  JasonSlice ss = s[0];
  checkBuild(ss, JasonType::SmallInt, 1);
  ASSERT_EQ(1ULL, ss.getUInt());
  ss = s[1];
  checkBuild(ss, JasonType::SmallInt, 1);
  ASSERT_EQ(2ULL, ss.getUInt());

  checkDump(s, value);
}

TEST(ParserTest, Array3) {
  std::string const value("[-1,2, 4.5, 3, -99.99]");
  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Array, 29);
  ASSERT_EQ(5ULL, s.length());

  JasonSlice ss = s[0];
  checkBuild(ss, JasonType::SmallInt, 1);
  ASSERT_EQ(-1LL, ss.getInt());

  ss = s[1];
  checkBuild(ss, JasonType::SmallInt, 1);
  ASSERT_EQ(2ULL, ss.getUInt());

  ss = s[2];
  checkBuild(ss, JasonType::Double, 9);
  ASSERT_EQ(4.5, ss.getDouble());

  ss = s[3];
  checkBuild(ss, JasonType::SmallInt, 1);
  ASSERT_EQ(3ULL, ss.getUInt());

  ss = s[4];
  checkBuild(ss, JasonType::Double, 9);
  ASSERT_EQ(-99.99, ss.getDouble());

  std::string const valueOut = "[-1,2,4.5,3,-99.99]";
  checkDump(s, valueOut);
}

TEST(ParserTest, Array4) {
  std::string const value("[\"foo\", \"bar\", \"baz\", null, true, false, -42.23 ]");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Array, 34);
  ASSERT_EQ(7ULL, s.length());

  JasonSlice ss = s[0];
  checkBuild(ss, JasonType::String, 4);
  std::string correct = "foo";
  ASSERT_EQ(correct, ss.copyString());

  ss = s[1];
  checkBuild(ss, JasonType::String, 4);
  correct = "bar";
  ASSERT_EQ(correct, ss.copyString());

  ss = s[2];
  checkBuild(ss, JasonType::String, 4);
  correct = "baz";
  ASSERT_EQ(correct, ss.copyString());

  ss = s[3];
  checkBuild(ss, JasonType::Null, 1);

  ss = s[4];
  checkBuild(ss, JasonType::Bool, 1);
  ASSERT_TRUE(ss.getBool());

  ss = s[5];
  checkBuild(ss, JasonType::Bool, 1);
  ASSERT_FALSE(ss.getBool());

  ss = s[6];
  checkBuild(ss, JasonType::Double, 9);
  ASSERT_EQ(-42.23, ss.getDouble());

  std::string const valueOut = "[\"foo\",\"bar\",\"baz\",null,true,false,-42.23]";
  checkDump(s, valueOut);
}

TEST(ParserTest, NestedArray1) {
  std::string const value("[ [ ] ]");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Array, 3);
  ASSERT_EQ(1ULL, s.length());

  JasonSlice ss = s[0];
  checkBuild(ss, JasonType::Array, 1);
  ASSERT_EQ(0ULL, ss.length());

  std::string const valueOut = "[[]]";
  checkDump(s, valueOut);
}

TEST(ParserTest, NestedArray2) {
  std::string const value("[ [ ],[[]],[],[ [[ [], [ ], [ ] ], [ ] ] ], [] ]");
  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Array, 27);
  ASSERT_EQ(5ULL, s.length());

  JasonSlice ss = s[0];
  checkBuild(ss, JasonType::Array, 1);
  ASSERT_EQ(0ULL, ss.length());

  ss = s[1];
  checkBuild(ss, JasonType::Array, 3);
  ASSERT_EQ(1ULL, ss.length());

  JasonSlice sss = ss[0];
  checkBuild(sss, JasonType::Array, 1);
  ASSERT_EQ(0ULL, sss.length());

  ss = s[2];
  checkBuild(ss, JasonType::Array, 1);
  ASSERT_EQ(0ULL, ss.length());

  ss = s[3];
  checkBuild(ss, JasonType::Array, 13);
  ASSERT_EQ(1ULL, ss.length());

  sss = ss[0];
  checkBuild(sss, JasonType::Array, 11);
  ASSERT_EQ(2ULL, sss.length());

  JasonSlice ssss = sss[0];
  checkBuild(ssss, JasonType::Array, 5);
  ASSERT_EQ(3ULL, ssss.length());

  JasonSlice sssss = ssss[0];
  checkBuild(sssss, JasonType::Array, 1);
  ASSERT_EQ(0ULL, sssss.length());

  sssss = ssss[1];
  checkBuild(sssss, JasonType::Array, 1);
  ASSERT_EQ(0ULL, sssss.length());

  sssss = ssss[2];
  checkBuild(sssss, JasonType::Array, 1);
  ASSERT_EQ(0ULL, sssss.length());

  ssss = sss[1];
  checkBuild(ssss, JasonType::Array, 1);
  ASSERT_EQ(0ULL, ssss.length());

  ss = s[4];
  checkBuild(ss, JasonType::Array, 1);
  ASSERT_EQ(0ULL, ss.length());

  std::string const valueOut = "[[],[[]],[],[[[[],[],[]],[]]],[]]";
  checkDump(s, valueOut);
}

TEST(ParserTest, NestedArray3) {
  std::string const value("[ [ \"foo\", [ \"bar\", \"baz\", null ], true, false ], -42.23 ]");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Array, 42);
  ASSERT_EQ(2ULL, s.length());

  JasonSlice ss = s[0];
  checkBuild(ss, JasonType::Array, 28);
  ASSERT_EQ(4ULL, ss.length());

  JasonSlice sss = ss[0];
  checkBuild(sss, JasonType::String, 4);
  std::string correct = "foo";
  ASSERT_EQ(correct, sss.copyString());

  sss = ss[1];
  checkBuild(sss, JasonType::Array, 15);
  ASSERT_EQ(3ULL, sss.length());

  JasonSlice ssss = sss[0];
  checkBuild(ssss, JasonType::String, 4);
  correct = "bar";
  ASSERT_EQ(correct, ssss.copyString());

  ssss = sss[1];
  checkBuild(ssss, JasonType::String, 4);
  correct = "baz";
  ASSERT_EQ(correct, ssss.copyString());

  ssss = sss[2];
  checkBuild(ssss, JasonType::Null, 1);

  sss = ss[2];
  checkBuild(sss, JasonType::Bool, 1);
  ASSERT_TRUE(sss.getBool());

  sss = ss[3];
  checkBuild(sss, JasonType::Bool, 1);
  ASSERT_FALSE(sss.getBool());

  ss = s[1];
  checkBuild(ss, JasonType::Double, 9);
  ASSERT_EQ(-42.23, ss.getDouble());

  std::string const valueOut = "[[\"foo\",[\"bar\",\"baz\",null],true,false],-42.23]";
  checkDump(s, valueOut);
}

TEST(ParserTest, NestedArrayInvalid1) {
  std::string const value("[ [ ]");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(4U, parser.errorPos());
}

TEST(ParserTest, NestedArrayInvalid2) {
  std::string const value("[ ] ]");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(4U, parser.errorPos());
}

TEST(ParserTest, NestedArrayInvalid3) {
  std::string const value("[ [ \"foo\", [ \"bar\", \"baz\", null ] ]");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(34U, parser.errorPos());
}

TEST(ParserTest, BrokenArray1) {
  std::string const value("[");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(0U, parser.errorPos());
}

TEST(ParserTest, BrokenArray2) {
  std::string const value("[,");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(1U, parser.errorPos());
}

TEST(ParserTest, BrokenArray3) {
  std::string const value("[1,");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(2U, parser.errorPos());
}

TEST(ParserTest, ShortArrayMembers) {
  std::string value("[");
  for (size_t i = 0; i < 255; ++i) {
    if (i > 0) {
      value.push_back(',');
    }
    value.append(std::to_string(i));
  }
  value.push_back(']');

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  ASSERT_EQ(7ULL, s.head()); 
  checkBuild(s, JasonType::Array, 1019);
  ASSERT_EQ(255ULL, s.length());
  
  for (size_t i = 0; i < 255; ++i) {
    JasonSlice ss = s[i];
    if (i <= 9) {
      checkBuild(ss, JasonType::SmallInt, 1);
    }
    else {
      checkBuild(ss, JasonType::UInt, 2);
    }
    ASSERT_EQ(i, ss.getUInt());
  }
}

TEST(ParserTest, LongArrayFewMembers) {
  std::string single("0123456789abcdef");
  single.append(single);
  single.append(single);
  single.append(single);
  single.append(single);
  single.append(single);
  single.append(single); // 1024 bytes

  std::string value("[");
  for (size_t i = 0; i < 65; ++i) {
    if (i > 0) {
      value.push_back(',');
    }
    value.push_back('"');
    value.append(single);
    value.push_back('"');
  }
  value.push_back(']');

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  ASSERT_EQ(4ULL, s.head());
  checkBuild(s, JasonType::Array, 67154);
  ASSERT_EQ(65ULL, s.length());
  
  for (size_t i = 0; i < 65; ++i) {
    JasonSlice ss = s[i];
    checkBuild(ss, JasonType::String, 1033);
    JasonLength len;
    char const* s = ss.getString(len);
    ASSERT_EQ(1024ULL, len);
    ASSERT_EQ(0, strncmp(s, single.c_str(), len));
  }
}

TEST(ParserTest, LongArrayManyMembers) {
  std::string value("[");
  for (size_t i = 0; i < 256; ++i) {
    if (i > 0) {
      value.push_back(',');
    }
    value.append(std::to_string(i));
  }
  value.push_back(']');

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  ASSERT_EQ(7ULL, s.head()); 
  checkBuild(s, JasonType::Array, 1023);
  ASSERT_EQ(256ULL, s.length());
  
  for (size_t i = 0; i < 256; ++i) {
    JasonSlice ss = s[i];
    if (i <= 9) {
      checkBuild(ss, JasonType::SmallInt, 1);
    }
    else {
      checkBuild(ss, JasonType::UInt, 2);
    }
    ASSERT_EQ(i, ss.getUInt());
  }
}

TEST(ParserTest, EmptyObject) {
  std::string const value("{}");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Object, 1);
  ASSERT_EQ(0ULL, s.length());

  checkDump(s, value);
}

TEST(ParserTest, BrokenObject1) {
  std::string const value("{");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(0U, parser.errorPos());
}

TEST(ParserTest, BrokenObject2) {
  std::string const value("{,");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(0U, parser.errorPos());
}

TEST(ParserTest, BrokenObject3) {
  std::string const value("{1,");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(0U, parser.errorPos());
}

TEST(ParserTest, BrokenObject4) {
  std::string const value("{\"foo");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(4U, parser.errorPos());
}

TEST(ParserTest, BrokenObject5) {
  std::string const value("{\"foo\"");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(5U, parser.errorPos());
}

TEST(ParserTest, BrokenObject6) {
  std::string const value("{\"foo\":");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(6U, parser.errorPos());
}

TEST(ParserTest, BrokenObject7) {
  std::string const value("{\"foo\":\"foo");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(10U, parser.errorPos());
}

TEST(ParserTest, BrokenObject8) {
  std::string const value("{\"foo\":\"foo\", ");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(13U, parser.errorPos());
}

TEST(ParserTest, BrokenObject9) {
  std::string const value("{\"foo\":\"foo\", }");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(13U, parser.errorPos());
}

TEST(ParserTest, BrokenObject10) {
  std::string const value("{\"foo\" }");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
  ASSERT_EQ(6U, parser.errorPos());
}

TEST(ParserTest, ObjectSimple1) {
  std::string const value("{ \"foo\" : 1}");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Object, 8);
  ASSERT_EQ(1ULL, s.length());

  JasonSlice ss = s.keyAt(0);
  checkBuild(ss, JasonType::String, 4);

  std::string correct = "foo";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(0);
  checkBuild(ss, JasonType::SmallInt, 1);
  ASSERT_EQ(1, ss.getSmallInt());

  std::string valueOut = "{\"foo\":1}";
  checkDump(s, valueOut);
}

TEST(ParserTest, ObjectSimple2) {
  std::string const value("{ \"foo\" : \"bar\", \"baz\":true}");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Object, 18);
  ASSERT_EQ(2ULL, s.length());

  JasonSlice ss = s.keyAt(0);
  checkBuild(ss, JasonType::String, 4);
  std::string correct = "baz";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(0);
  checkBuild(ss, JasonType::Bool, 1);
  ASSERT_TRUE(ss.getBool());

  ss = s.keyAt(1);
  checkBuild(ss, JasonType::String, 4);
  correct = "foo";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(1);
  checkBuild(ss, JasonType::String, 4);
  correct = "bar";
  ASSERT_EQ(correct, ss.copyString());

  std::string valueOut = "{\"baz\":true,\"foo\":\"bar\"}";
  checkDump(s, valueOut);
}

TEST(ParserTest, ObjectDenseNotation) {
  std::string const value("{\"a\":\"b\",\"c\":\"d\"}");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Object, 13);
  ASSERT_EQ(2ULL, s.length());

  JasonSlice ss = s.keyAt(0);
  checkBuild(ss, JasonType::String, 2);
  std::string correct = "a";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(0);
  checkBuild(ss, JasonType::String, 2);
  correct = "b";
  ASSERT_EQ(correct, ss.copyString());

  ss = s.keyAt(1);
  checkBuild(ss, JasonType::String, 2);
  correct = "c";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(1);
  checkBuild(ss, JasonType::String, 2);
  correct = "d";
  ASSERT_EQ(correct, ss.copyString());

  checkDump(s, value);
}

TEST(ParserTest, ObjectReservedKeys) {
  std::string const value("{ \"null\" : \"true\", \"false\":\"bar\", \"true\":\"foo\"}");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Object, 35);
  ASSERT_EQ(3ULL, s.length());

  JasonSlice ss = s.keyAt(0);
  checkBuild(ss, JasonType::String, 6);
  std::string correct = "false";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(0);
  checkBuild(ss, JasonType::String, 4);
  correct = "bar";
  ASSERT_EQ(correct, ss.copyString());

  ss = s.keyAt(1);
  checkBuild(ss, JasonType::String, 5);
  correct = "null";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(1);
  checkBuild(ss, JasonType::String, 5);
  correct = "true";
  ASSERT_EQ(correct, ss.copyString());

  ss = s.keyAt(2);
  checkBuild(ss, JasonType::String, 5);
  correct = "true";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(2);
  checkBuild(ss, JasonType::String, 4);
  correct = "foo";
  ASSERT_EQ(correct, ss.copyString());

  std::string const valueOut = "{\"false\":\"bar\",\"null\":\"true\",\"true\":\"foo\"}";
  checkDump(s, valueOut);
}

TEST(ParserTest, ObjectMixed) {
  std::string const value("{\"foo\":null,\"bar\":true,\"baz\":13.53,\"qux\":[1],\"quz\":{}}");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Object, 43);
  ASSERT_EQ(5ULL, s.length());

  JasonSlice ss = s.keyAt(0);
  checkBuild(ss, JasonType::String, 4);
  std::string correct = "bar";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(0);
  checkBuild(ss, JasonType::Bool, 1);
  ASSERT_TRUE(ss.getBool());

  ss = s.keyAt(1);
  checkBuild(ss, JasonType::String, 4);
  correct = "baz";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(1);
  checkBuild(ss, JasonType::Double, 9);
  ASSERT_EQ(13.53, ss.getDouble());

  ss = s.keyAt(2);
  checkBuild(ss, JasonType::String, 4);
  correct = "foo";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(2);
  checkBuild(ss, JasonType::Null, 1);

  ss = s.keyAt(3);
  checkBuild(ss, JasonType::String, 4);
  correct = "qux";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(3);
  checkBuild(ss, JasonType::Array, 3);

  JasonSlice sss = ss[0];
  checkBuild(sss, JasonType::SmallInt, 1);
  ASSERT_EQ(1ULL, sss.getUInt());

  ss = s.keyAt(4);
  checkBuild(ss, JasonType::String, 4);
  correct = "quz";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(4);
  checkBuild(ss, JasonType::Object, 1);
  ASSERT_EQ(0ULL, ss.length());

  std::string const valueOut("{\"bar\":true,\"baz\":13.53,\"foo\":null,\"qux\":[1],\"quz\":{}}");
  checkDump(s, valueOut);
}

TEST(ParserTest, ObjectInvalidQuotes) {
  std::string const value("{'foo':'bar' }");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
}

TEST(ParserTest, ObjectMissingQuotes) {
  std::string const value("{foo:\"bar\" }");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
}

TEST(ParserTest, ShortObjectMembers) {
  std::string value("{");
  for (size_t i = 0; i < 255; ++i) {
    if (i > 0) {
      value.push_back(',');
    }
    value.append("\"test");
    if (i < 100) {
      value.push_back('0');
      if (i < 10) {
        value.push_back('0');
      }
    }
    value.append(std::to_string(i));
    value.append("\":");
    value.append(std::to_string(i));
  }
  value.push_back('}');

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  ASSERT_EQ(0xcULL, s.head()); 
  checkBuild(s, JasonType::Object, 3059);
  ASSERT_EQ(255ULL, s.length());
  
  for (size_t i = 0; i < 255; ++i) {
    JasonSlice sk = s.keyAt(i);
    JasonLength len;
    char const* str = sk.getString(len);
    std::string key("test");
    if (i < 100) {
      key.push_back('0');
      if (i < 10) {
        key.push_back('0');
      }
    }
    key.append(std::to_string(i));

    ASSERT_EQ(key.size(), len);
    ASSERT_EQ(0, strncmp(str, key.c_str(), len));
    JasonSlice sv = s.valueAt(i);
    if (i <= 9) {
      checkBuild(sv, JasonType::SmallInt, 1);
    }
    else {
      checkBuild(sv, JasonType::UInt, 2);
    }
    ASSERT_EQ(i, sv.getUInt());
  }
}

TEST(ParserTest, LongObjectFewMembers) {
  std::string single("0123456789abcdef");
  single.append(single);
  single.append(single);
  single.append(single);
  single.append(single);
  single.append(single);
  single.append(single); // 1024 bytes

  std::string value("{");
  for (size_t i = 0; i < 64; ++i) {
    if (i > 0) {
      value.push_back(',');
    }
    value.append("\"test");
    if (i < 100) {
      value.push_back('0');
      if (i < 10) {
        value.push_back('0');
      }
    }
    value.append(std::to_string(i));
    value.append("\":\"");
    value.append(single);
    value.push_back('\"');
  }
  value.push_back('}');

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  ASSERT_EQ(0x0dULL, s.head()); // object with offset size 4
  checkBuild(s, JasonType::Object, 66889);
  ASSERT_EQ(64ULL, s.length());
  
  for (size_t i = 0; i < 64; ++i) {
    JasonSlice sk = s.keyAt(i);
    JasonLength len;
    char const* str = sk.getString(len);
    std::string key("test");
    if (i < 100) {
      key.push_back('0');
      if (i < 10) {
        key.push_back('0');
      }
    }
    key.append(std::to_string(i));

    ASSERT_EQ(key.size(), len);
    ASSERT_EQ(0, strncmp(str, key.c_str(), len));
    JasonSlice sv = s.valueAt(i);
    str = sv.getString(len);
    ASSERT_EQ(1024ULL, len);
    ASSERT_EQ(0, strncmp(str, single.c_str(), len));
  }
}

TEST(ParserTest, LongObjectManyMembers) {
  std::string value("{");
  for (size_t i = 0; i < 256; ++i) {
    if (i > 0) {
      value.push_back(',');
    }
    value.append("\"test");
    if (i < 100) {
      value.push_back('0');
      if (i < 10) {
        value.push_back('0');
      }
    }
    value.append(std::to_string(i));
    value.append("\":");
    value.append(std::to_string(i));
  }
  value.push_back('}');

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  ASSERT_EQ(0x0cULL, s.head()); // long object
  checkBuild(s, JasonType::Object, 3071);
  ASSERT_EQ(256ULL, s.length());
  
  for (size_t i = 0; i < 256; ++i) {
    JasonSlice sk = s.keyAt(i);
    JasonLength len;
    char const* str = sk.getString(len);
    std::string key("test");
    if (i < 100) {
      key.push_back('0');
      if (i < 10) {
        key.push_back('0');
      }
    }
    key.append(std::to_string(i));

    ASSERT_EQ(key.size(), len);
    ASSERT_EQ(0, strncmp(str, key.c_str(), len));
    JasonSlice sv = s.valueAt(i);
    if (i <= 9) {
      checkBuild(sv, JasonType::SmallInt, 1);
    }
    else {
      checkBuild(sv, JasonType::UInt, 2);
    }
    ASSERT_EQ(i, sv.getUInt());
  }
}

TEST(ParserTest, Utf8Bom) {
  std::string const value("\xef\xbb\xbf{\"foo\":1}");

  JasonParser parser;
  JasonLength len = parser.parse(value);
  ASSERT_EQ(1ULL, len);

  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  checkBuild(s, JasonType::Object, 8);
  ASSERT_EQ(1ULL, s.length());

  JasonSlice ss = s.keyAt(0);
  checkBuild(ss, JasonType::String, 4);
  std::string correct = "foo";
  ASSERT_EQ(correct, ss.copyString());
  ss = s.valueAt(0);
  checkBuild(ss, JasonType::SmallInt, 1);
  ASSERT_EQ(1ULL, ss.getUInt());

  std::string valueOut = "{\"foo\":1}";
  checkDump(s, valueOut);
}

TEST(ParserTest, Utf8BomBroken) {
  std::string const value("\xef\xbb");

  JasonParser parser;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::ParseError);
}

TEST(ParserTest, DuplicateAttributesAllowed) {
  std::string const value("{\"foo\":1,\"foo\":2}");

  JasonParser parser;
  parser.parse(value);
  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());

  JasonSlice v = s.get("foo");
  ASSERT_TRUE(v.isNumber());
  ASSERT_EQ(1ULL, v.getUInt());
}

TEST(ParserTest, DuplicateAttributesDisallowed) {
  std::string const value("{\"foo\":1,\"foo\":2}");

  JasonParser parser;
  parser.options.checkAttributeUniqueness = true;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::DuplicateAttributeName);
}

TEST(ParserTest, DuplicateAttributesDisallowedUnsortedObject) {
  std::string const value("{\"foo\":1,\"bar\":3,\"foo\":2}");

  JasonParser parser;
  parser.options.sortAttributeNames = false;
  parser.options.checkAttributeUniqueness = true;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::DuplicateAttributeName);
}

TEST(ParserTest, DuplicateSubAttributesAllowed) {
  std::string const value("{\"foo\":{\"bar\":1},\"baz\":{\"bar\":2},\"bar\":{\"foo\":23,\"baz\":9}}");

  JasonParser parser;
  parser.options.checkAttributeUniqueness = true;
  parser.parse(value);
  JasonBuilder builder = parser.steal();
  JasonSlice s(builder.start());
  JasonSlice v = s.get(std::vector<std::string>({ "foo", "bar" })); 
  ASSERT_TRUE(v.isNumber());
  ASSERT_EQ(1ULL, v.getUInt());
}

TEST(ParserTest, DuplicateSubAttributesDisallowed) {
  std::string const value("{\"roo\":{\"bar\":1,\"abc\":true,\"def\":7,\"abc\":2}}");

  JasonParser parser;
  parser.options.checkAttributeUniqueness = true;
  EXPECT_JASON_EXCEPTION(parser.parse(value), JasonException::DuplicateAttributeName);
}

int main (int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

