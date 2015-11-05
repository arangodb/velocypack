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

#include "tests-common.h"
  
static std::string readFile (std::string const& filename) {
  std::string s;
  std::ifstream ifs(filename.c_str(), std::ifstream::in);

  if (! ifs.is_open()) {
    throw "cannot open input file";
  }
  
  char buffer[4096];
  while (ifs.good()) {
    ifs.read(&buffer[0], sizeof(buffer));
    s.append(buffer, ifs.gcount());
  }
  ifs.close();
  return s;
}

static bool parseFile (std::string const& filename) {
  std::string const data = readFile(filename);
 
  JasonParser parser;
  try {
    parser.parse(data);
    return true;
  }
  catch (...) {
    return false;
  }
}

TEST(StaticFilesTest, CommitsJson) {
  ASSERT_TRUE(parseFile("jsonSample/commits.json"));
}

TEST(StaticFilesTest, SampleJson) {
  ASSERT_TRUE(parseFile("jsonSample/sample.json"));
}

TEST(StaticFilesTest, SampleNoWhiteJson) {
  ASSERT_TRUE(parseFile("jsonSample/sampleNoWhite.json"));
}

TEST(StaticFilesTest, SmallJson) {
  ASSERT_TRUE(parseFile("jsonSample/small.json"));
}

TEST(StaticFilesTest, Fail2Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail2.json"));
}

TEST(StaticFilesTest, Fail3Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail3.json"));
}

TEST(StaticFilesTest, Fail4Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail4.json"));
}

TEST(StaticFilesTest, Fail5Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail5.json"));
}

TEST(StaticFilesTest, Fail6Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail6.json"));
}

TEST(StaticFilesTest, Fail7Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail7.json"));
}

TEST(StaticFilesTest, Fail8Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail8.json"));
}

TEST(StaticFilesTest, Fail9Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail9.json"));
}

TEST(StaticFilesTest, Fail10Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail10.json"));
}

TEST(StaticFilesTest, Fail11Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail11.json"));
}

TEST(StaticFilesTest, Fail12Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail12.json"));
}

TEST(StaticFilesTest, Fail13Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail13.json"));
}

TEST(StaticFilesTest, Fail14Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail14.json"));
}

TEST(StaticFilesTest, Fail15Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail15.json"));
}

TEST(StaticFilesTest, Fail16Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail16.json"));
}

TEST(StaticFilesTest, Fail17Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail17.json"));
}

TEST(StaticFilesTest, Fail19Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail19.json"));
}

TEST(StaticFilesTest, Fail20Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail20.json"));
}

TEST(StaticFilesTest, Fail21Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail21.json"));
}

TEST(StaticFilesTest, Fail22Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail22.json"));
}

TEST(StaticFilesTest, Fail23Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail23.json"));
}

TEST(StaticFilesTest, Fail24Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail24.json"));
}

TEST(StaticFilesTest, Fail25Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail25.json"));
}

TEST(StaticFilesTest, Fail26Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail26.json"));
}

TEST(StaticFilesTest, Fail27Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail27.json"));
}

TEST(StaticFilesTest, Fail28Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail28.json"));
}

TEST(StaticFilesTest, Fail29Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail29.json"));
}

TEST(StaticFilesTest, Fail30Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail30.json"));
}

TEST(StaticFilesTest, Fail31Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail31.json"));
}

TEST(StaticFilesTest, Fail32Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail32.json"));
}

TEST(StaticFilesTest, Fail33Json) {
  ASSERT_FALSE(parseFile("jsonSample/fail33.json"));
}

int main (int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

