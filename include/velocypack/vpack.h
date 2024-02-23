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

#include "velocypack/velocypack-common.h"
#include "velocypack/AttributeTranslator.h"
#include "velocypack/Basics.h"
#include "velocypack/Buffer.h"
#include "velocypack/Builder.h"
#include "velocypack/Collection.h"
#include "velocypack/Compare.h"
#include "velocypack/Dumper.h"
#include "velocypack/Exception.h"
#include "velocypack/HashedStringRef.h"
#include "velocypack/HexDump.h"
#include "velocypack/Iterator.h"
#include "velocypack/Options.h"
#include "velocypack/Parser.h"
#include "velocypack/Serializable.h"
#include "velocypack/SharedSlice.h"
#include "velocypack/Sink.h"
#include "velocypack/Slice.h"
#include "velocypack/SliceContainer.h"
#include "velocypack/SmallVector.h"
#include "velocypack/StringRef.h"
#include "velocypack/Utf8Helper.h"
#include "velocypack/Validator.h"
#include "velocypack/Value.h"
#include "velocypack/ValueType.h"
#include "velocypack/Version.h"
