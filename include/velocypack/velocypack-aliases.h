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

#ifndef VELOCYPACK_ALIASES_H
#define VELOCYPACK_ALIASES_H 1

#include "velocitypack/vpack.h"

using VPackBufferDumper       = arangodb::velocitypack::BufferDumper;
using VPackBuilder            = arangodb::velocitypack::Builder;
using VPackCharBuffer         = arangodb::velocitypack::CharBuffer;
using VPackException          = arangodb::velocitypack::Exception;
using VPackDumper             = arangodb::velocitypack::Dumper;
using VPackOptions            = arangodb::velocitypack::Options;
using VPackParser             = arangodb::velocitypack::Parser;
using VPackStringDumper       = arangodb::velocitypack::StringDumper;
using VPackStringPrettyDumper = arangodb::velocitypack::StringPrettyDumper;
using VPackSlice              = arangodb::velocitypack::Slice;
using VPackValue              = arangodb::velocitypack::Value;
using VPackValueType          = arangodb::velocitypack::ValueType;

#endif
