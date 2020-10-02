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
/// @author Colin Hirsch
/// @author Copyright 2015, ArangoDB GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "velocypack/velocypack-common.h"
#include "velocypack/TaoParser.h"

#include "tao/json/external/pegtl/memory_input.hpp"

#include <cstdlib>

namespace arangodb
{
   namespace velocypack
   {
      ValueLength TaoParser::parse( const void * json, const ValueLength size, const bool multi) {
         if (options->clearBuilderBeforeParse) {
            _e.builder->clear();
         }
         tao::json::pegtl::memory_input< tao::json::pegtl::tracking_mode::lazy, tao::json::pegtl::eol::lf_crlf, const char* > in( reinterpret_cast< const char * >( json ), size, __PRETTY_FUNCTION__ );
         if ( multi ) {
            ValueLength nr = 0;
            do {
               const auto * const t = in.current();
               tao::json::pegtl::parse< tao::json::pegtl::must< tao::json::internal::rules::text >, tao::json::internal::action>( in, _e );
               nr += in.current() - t;
            } while ( !in.empty() );
            return nr;
         }
         const auto * const t = in.current();
         tao::json::pegtl::parse< tao::json::internal::grammar, tao::json::internal::action>( in, _e );
         return in.current() - t;
      }

   } // velocypack

} // arangodb
