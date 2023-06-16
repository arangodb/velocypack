////////////////////////////////////////////////////////////////////////////////
/// @brief Library to build up VPack documents.
///
/// DISCLAIMER
///
/// Copyright 2023 ArangoDB GmbH, Cologne, Germany
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
/// @author Lars Maier
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <memory_resource>

#include "tests-common.h"

#include <velocypack/String.h>

using namespace arangodb::velocypack;

TEST(StringTest, pmr_container) {
  std::pmr::vector<pmr::String> vec{std::pmr::new_delete_resource()};
  vec.emplace_back(VPackSlice::emptyArraySlice());
  vec.emplace_back(VPackSlice::emptyArraySlice().getDataPtr());

  auto other = pmr::String {VPackSlice::emptyObjectSlice()};
  vec.emplace_back(other);
  vec.emplace_back(std::move(other));

  for (auto const& e : vec) {
    EXPECT_EQ(e.getUnderlyingString().get_allocator(), vec.get_allocator());
  }
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
