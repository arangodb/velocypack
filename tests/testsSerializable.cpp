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
/// @author Copyright 2015, ArangoDB GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include <ostream>
#include <string>
#include <iostream>

#include "tests-common.h"

class SerializableTest : public Serializable {
public:
  void toVelocyPack(Builder &b) const override {
    {
      ObjectBuilder ob(&b);
      b.add("test", Value("serialized!"));
    }
  }
};

TEST(SerializableTest, AddTest) {

  SerializableTest st;

  Builder b;
  b.add(Serialize(st));
  Slice s(b.slice());

  ASSERT_EQ(ValueType::Object, s.type());
  ASSERT_EQ(s.get("test").copyString(), "serialized!");
}

TEST(SerializableTest, AddObjectTest) {

  SerializableTest st;

  Builder b;
  {
    ObjectBuilder ob(&b);
    b.add("key", Serialize(st));
  }
  Slice s(b.slice());
  ASSERT_EQ(ValueType::Object, s.type());
  Slice t(s.get("key"));
  ASSERT_EQ(t.get("test").copyString(), "serialized!");
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
