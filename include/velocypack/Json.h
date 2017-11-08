// Author: Dr. Colin Hirsch

#ifndef ARANGODB_VELOCYPACK_JSON_H
#define ARANGODB_VELOCYPACK_JSON_H

#include <string>
#include <vector>

#include "tao/json.hpp"

#include "velocypack/Builder.h"
#include "velocypack/Iterator.h"
#include "velocypack/Options.h"
#include "velocypack/Slice.h"
#include "velocypack/Value.h"
#include "velocypack/ValueType.h"

namespace arangodb
{
   namespace velocypack
   {
      // Class that consumes taocpp/json Events, i.e.
      // implements the standard taocpp/json Events
      // API, and feeds them into a Builder.

      class EventsToBuilder
      {
      private:
         std::string m_key;
         bool m_member = false;

         void add( const Value& v )
         {
            if ( m_member ) {
               builder->add( m_key, v );
               m_member = false;
            }
            else {
               builder->add( v );
            }
         }

         void add( const ValuePair& v )
         {
            if ( m_member ) {
               builder->add( m_key, v );
               m_member = false;
            }
            else {
               builder->add( v );
            }
         }

      public:
         std::shared_ptr< Builder > builder;

         EventsToBuilder()
            : builder( std::make_shared< Builder >() )
         {
         }

         EventsToBuilder( std::shared_ptr< Builder > && builder )
            : builder( std::move( builder ) )
         {
         }

         EventsToBuilder( const std::shared_ptr< Builder > & builder )
            : builder( builder )
         {
         }

         void null()
         {
            add( Value( ValueType::Null ) );
         }

         void boolean( const bool v )
         {
            add( Value( v ) );
         }

         void number( const std::int64_t v )
         {
            add( Value( v ) );
         }

         void number( const std::uint64_t v )
         {
            add( Value( v ) );
         }

         void number( const double v )
         {
            add( Value( v ) );
         }

         void string( const tao::string_view v )
         {
            add( ValuePair( reinterpret_cast< const uint8_t* >( v.data() ), v.size(), ValueType::String ) );
         }

         void binary( const tao::byte_view v )
         {
            add( ValuePair( reinterpret_cast< const uint8_t* >( v.data() ), v.size(), ValueType::Binary ) );
         }

         void begin_array( const std::size_t = 0 )
         {
            add( Value( ValueType::Array ) );
         }

         void element()
         {
         }

         void end_array( const std::size_t = 0 )
         {
            builder->close();
         }

         void begin_object( const std::size_t = 0 )
         {
            add( Value( ValueType::Object ) );
         }

         void key( const std::string& v )
         {
            m_key = v;
            m_member = true;
         }

         void key( std::string&& v )
         {
            m_key = std::move( v );
            m_member = true;
         }

         void member()
         {
         }

         void end_object( const std::size_t = 0 )
         {
            builder->close();
         }
      };

      // Function that produces taocpp/json Events from
      // a given Slice (or Builder); the consumer must
      // implement the taocpp/json Events API methods.

      template< typename Consumer >
      void sliceToEvents( Consumer& consumer, const Slice& slice )
      {
         switch( slice.type() ) {
            case ValueType::None:
            case ValueType::Illegal:
               // TOOD: Decide what to do; for now fall through.
            case ValueType::Null:
               consumer.null();
               break;
            case ValueType::Bool:
               consumer.boolean( slice.getBool() );
               break;
            case ValueType::Array: {
               const auto size = slice.length();
               consumer.begin_array( size );
               for ( const auto& s : ArrayIterator( slice ) ) {
                  sliceToEvents( consumer, s );
                  consumer.element();
               }
               consumer.end_array( size );
            }  break;
            case ValueType::Object: {
               const auto size = slice.length();
               consumer.begin_object( size );
               for ( const auto& s : ObjectIterator( slice ) ) {
                  ValueLength size;
                  const char * data = s.key.getString( size );
                  consumer.key( tao::string_view( data, size ) );
                  sliceToEvents( consumer, s.value );
                  consumer.member();
               }
               consumer.end_object( size );
            }  break;
            case ValueType::Double:
               consumer.number( slice.getDouble() );
               break;
            case ValueType::UTCDate:
               // TODO: Implement when taocpp/json datetime support is finished.
               break;
            case ValueType::External:
               // TODO: What?
               break;
            case ValueType::MinKey:
            case ValueType::MaxKey:
               // TODO: What?
               break;
            case ValueType::Int:
               consumer.number( slice.getInt() );
               break;
            case ValueType::UInt:
               consumer.number( slice.getUInt() );
               break;
            case ValueType::SmallInt:
               consumer.number( slice.getSmallInt() );
               break;
            case ValueType::String: {
               ValueLength size;
               const char * data = slice.getString( size );
               consumer.string( tao::string_view( data, size ) );
            }  break;
            case ValueType::Binary: {
               ValueLength size;
               const auto * data = slice.getBinary( size );
               consumer.binary( tao::byte_view( reinterpret_cast< const tao::byte* >( data ), size ) );
            }  break;
            case ValueType::BCD:
               // TODO: Convert to other number format or string?
               break;
            case ValueType::Custom:
               // TODO: What?
               break;
         }
      }

      template< typename Consumer >
      void builderToEvents( Consumer& consumer, const Builder& builder )
      {
         sliceToEvents( consumer, Slice( builder.start() ) );
      }

      inline void builderToJsonStream( std::ostream& stream, const Builder& builder )
      {
         tao::json::events::to_stream events( stream );
         builderToEvents( events, builder );
      }

      inline void builderToPrettyJsonStream( std::ostream& stream, const Builder& builder, const std::size_t indent = 3 )
      {
         tao::json::events::to_pretty_stream events( stream, indent );
         builderToEvents( events, builder );
      }

      inline std::string builderToJsonString( const Builder& builder )
      {
         std::ostringstream stream;
         builderToJsonStream( stream, builder );
         return stream.str();
      }

      inline std::string builderToPrettyJsonString( const Builder& builder, const std::size_t indent = 3 )
      {
         std::ostringstream stream;
         builderToPrettyJsonStream( stream, builder, indent );
         return stream.str();
      }

   } // velocypack

} // namespace arangodb

#endif
