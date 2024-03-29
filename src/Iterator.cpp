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

#include <ostream>

#include "velocypack/velocypack-common.h"
#include "velocypack/Iterator.h"

using namespace arangodb::velocypack;

std::ostream& operator<<(std::ostream& stream, ArrayIterator const* it) {
  stream << "[ArrayIterator " << it->index() << " / " << it->size() << "]";
  return stream;
}

std::ostream& operator<<(std::ostream& stream, ArrayIterator const& it) {
  return operator<<(stream, &it);
}

std::ostream& operator<<(std::ostream& stream, ObjectIterator const* it) {
  stream << "[ObjectIterator " << it->index() << " / " << it->size() << "]";
  return stream;
}

std::ostream& operator<<(std::ostream& stream, ObjectIterator const& it) {
  return operator<<(stream, &it);
}
