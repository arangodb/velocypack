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

#pragma once

#include <cstdint>

extern std::size_t (*JSONStringCopy)(uint8_t*, uint8_t const*, std::size_t);

// Now a version which also stops at high bit set bytes:
extern std::size_t (*JSONStringCopyCheckUtf8)(uint8_t*, uint8_t const*,
                                              std::size_t);

// White space skipping:
extern std::size_t (*JSONSkipWhiteSpace)(uint8_t const*, std::size_t);

// check string for invalid utf-8 sequences
extern bool (*ValidateUtf8String)(uint8_t const*, std::size_t);

namespace arangodb::velocypack {

void enableNativeStringFunctions();
void enableBuiltinStringFunctions();

}  // namespace arangodb::velocypack
