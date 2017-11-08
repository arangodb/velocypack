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

#ifndef VELOCYPACK_PARSER_H
#define VELOCYPACK_PARSER_H 1

#include <string>
#include <cmath>
#include <memory>
#include <utility>

#include "velocypack/velocypack-common.h"
#include "velocypack/Json.h"
#include "velocypack/Exception.h"

namespace arangodb {
namespace velocypack {

   // Parser class with the same interface as before
   // the integration of taocpp/json.

class Parser {

   EventsToBuilder _e;

 public:
  Options const* options;

  Parser(Parser const&) = delete;
  Parser(Parser&&) = delete;
  Parser& operator=(Parser const&) = delete;
  Parser& operator=(Parser&&) = delete;
  ~Parser() = default;

  explicit Parser(Options const* options = &Options::Defaults)
      : options(options) {
    if (options == nullptr) {
      throw Exception(Exception::InternalError, "Options cannot be a nullptr");
    }
    _e.builder->options = options;
  }

  explicit Parser(std::shared_ptr<Builder>& builder,
                  Options const* options = &Options::Defaults)
      : _e(builder),
        options(options) {
    if (options == nullptr) {
      throw Exception(Exception::InternalError, "Options cannot be a nullptr");
    }
  }

  // This method produces a parser that does not own the builder
  explicit Parser(Builder& builder,
                  Options const* options = &Options::Defaults)
      : _e( std::shared_ptr< Builder >( &builder, BuilderNonDeleter() ) ),
        options(options) {
    if (options == nullptr) {
      throw Exception(Exception::InternalError, "Options cannot be a nullptr");
    }
  }

  Builder const& builder() const { return *_e.builder; }

  static std::shared_ptr<Builder> fromJson(
      std::string const& json,
      Options const* options = &Options::Defaults) {
    Parser parser(options);
    parser.parse(json);
    return parser.steal();
  }

  static std::shared_ptr<Builder> fromJson(
      char const* start, size_t size,
      Options const* options = &Options::Defaults) {
    Parser parser(options);
    parser.parse(start, size);
    return parser.steal();
  }

  static std::shared_ptr<Builder> fromJson(
      uint8_t const* start, size_t size,
      Options const* options = &Options::Defaults) {
    Parser parser(options);
    parser.parse(start, size);
    return parser.steal();
  }

  ValueLength parse(std::string const& json, bool multi = false) {
    return parse(json.data(), json.size(),multi);
  }

  ValueLength parse(void const* json, const ValueLength size, const bool multi = false );

  std::shared_ptr<Builder> steal() {
    // Parser object is broken after a steal()
    return std::move( _e.builder );
  }

  // Beware, only valid as long as you do not parse more, use steal
  // to move the data out!
  uint8_t const* start() { return _e.builder->start(); }

  // Returns the position at the time when the just reported error
  // occurred, only use when handling an exception.
  size_t errorPos() const { return 0; }  // TODO!

  void clear() { _e.builder->clear(); }
};

}  // namespace arangodb::velocypack
}  // namespace arangodb

#endif
