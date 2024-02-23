////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2024 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Business Source License 1.1 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     https://github.com/arangodb/arangodb/blob/devel/LICENSE
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
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <new>

namespace arangodb::velocypack {

// classes from Basics.h are for internal use only and are not exposed here

// prevent copying
class NonCopyable {
 public:
  NonCopyable() = default;
  ~NonCopyable() = default;

 private:
  NonCopyable(NonCopyable const&) = delete;
  NonCopyable& operator=(NonCopyable const&) = delete;
};

#ifdef _WIN32
// turn off warnings about unimplemented exception specifications
#pragma warning(push)
#pragma warning(disable : 4290)
#endif

// prevent heap allocation
struct NonHeapAllocatable {
  void* operator new(std::size_t) = delete;
  void operator delete(void*) noexcept = delete;
  void* operator new[](std::size_t) = delete;
  void operator delete[](void*) noexcept = delete;
};

#ifdef _WIN32
#pragma warning(pop)
#endif

}  // namespace arangodb::velocypack
