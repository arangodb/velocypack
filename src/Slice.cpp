////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2020 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
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
////////////////////////////////////////////////////////////////////////////////

#include <velocypack/Slice.h>
#include "SliceBase.tpp"

using namespace arangodb::velocypack;

uint8_t const Slice::noneSliceData[] = {0x00};
uint8_t const Slice::illegalSliceData[] = {0x17};
uint8_t const Slice::nullSliceData[] = {0x18};
uint8_t const Slice::falseSliceData[] = {0x19};
uint8_t const Slice::trueSliceData[] = {0x1a};
uint8_t const Slice::zeroSliceData[] = {0x30};
uint8_t const Slice::emptyStringSliceData[] = {0x40};
uint8_t const Slice::emptyArraySliceData[] = {0x01};
uint8_t const Slice::emptyObjectSliceData[] = {0x0a};
uint8_t const Slice::minKeySliceData[] = {0x1e};
uint8_t const Slice::maxKeySliceData[] = {0x1f};

namespace arangodb::velocypack {
INSTANTIATE_TYPE(Slice, Slice)

std::ostream& operator<<(std::ostream& stream, Slice const* slice) {
  stream << "[Slice " << valueTypeName(slice->type()) << " ("
         << slice->hexType() << "), byteSize: " << slice->byteSize() << "]";
  return stream;
}

std::ostream& operator<<(std::ostream& stream, Slice const& slice) {
  return operator<<(stream, &slice);
}
}  // namespace arangodb::velocypack

static_assert(sizeof(arangodb::velocypack::Slice) == sizeof(void*),
              "Slice has an unexpected size");
