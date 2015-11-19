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

#include "velocypack/velocypack-common.h"

namespace {

  // unconditional typedefs
  using VPackValueLength             = arangodb::velocypack::ValueLength;
  
  // conditional typedefs, only uses when the respective headers are already included
#ifdef VELOCYPACK_ITERATOR_H
  using VPackArrayIterator           = arangodb::velocypack::ArrayIterator;
  using VPackObjectIterator          = arangodb::velocypack::ObjectIterator;
#endif

#ifdef VELOCYPACK_BUILDER_H
  using VPackBuilder                 = arangodb::velocypack::Builder;
#endif

#ifdef VELOCYPACK_BUILDER_H
  using VPackCharBuffer              = arangodb::velocypack::CharBuffer;
#endif

#ifdef VELOCYPACK_SINK_H
  using VPackSink                    = arangodb::velocypack::Sink;
  using VPackCharBufferSink          = arangodb::velocypack::CharBufferSink;
  using VPackStringSink              = arangodb::velocypack::StringSink;
  using VPackStringStreamSink        = arangodb::velocypack::StringStreamSink;
#endif

#ifdef VELOCYPACK_COLLECTION_H
  using VPackCollection              = arangodb::velocypack::Collection;
#endif

#ifdef VELOCYPACK_ATTRIBUTETRANSLATOR_H
  using VPackAttributeTranslator     = arangodb::velocypack::AttributeTranslator;
#endif

#ifdef VELOCYPACK_DUMPER_H
  using VPackDumper                  = arangodb::velocypack::Dumper;
#endif

#ifdef VELOCYPACK_EXCEPTION_H
  using VPackException               = arangodb::velocypack::Exception;
#endif

#ifdef VELOCYPACK_HEXDUMP_H
  using VPackHexDump                 = arangodb::velocypack::HexDump;
#endif

#ifdef VELOCYPACK_OPTIONS_H
  using VPackOptions                 = arangodb::velocypack::Options;
  using VPackAttributeExcludeHandler = arangodb::velocypack::AttributeExcludeHandler;
  using VPackCustomTypeHandler       = arangodb::velocypack::CustomTypeHandler;
#endif

#ifdef VELOCYPACK_PARSER_H
  using VPackParser                  = arangodb::velocypack::Parser;
#endif

#ifdef VELOCYPACK_SLICE_H
  using VPackSlice                   = arangodb::velocypack::Slice;
#endif

#ifdef VELOCYPACK_VALUE_H
  using VPackValue                   = arangodb::velocypack::Value;
  using VPackValuePair               = arangodb::velocypack::ValuePair;
#endif

#ifdef VELOCYPACK_VALUETYPE_H
  using VPackValueType               = arangodb::velocypack::ValueType;
#endif

#ifdef VELOCYPACK_VERSION_H
  using VPackVersion                 = arangodb::velocypack::Version;
#endif
}

#endif
