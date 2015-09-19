#ifndef JASON_DUMPER_H
#define JASON_DUMPER_H 1

#include "JasonBuffer.h"
#include "JasonSlice.h"
#include "JasonType.h"
#include "Jason.h"

namespace triagens {
  namespace basics {

    // Dumps Jason into a JSON output string
    class JasonDumper {

        struct JasonDumperError : std::exception {
          private:
            std::string _msg;
          public:
            JasonDumperError (std::string const& msg) : _msg(msg) {
            }
            char const* what() const noexcept {
              return _msg.c_str();
            }
        };

      public:

        enum UnsupportedTypeStrategy {
          STRATEGY_SUPPRESS,
          STRATEGY_FAIL
        };

        JasonDumper (JasonDumper const&) = delete;
        JasonDumper& operator= (JasonDumper const&) = delete;

        JasonDumper (JasonSlice slice, JasonBuffer* buffer, UnsupportedTypeStrategy strategy) 
          : _slice(slice), _buffer(buffer), _strategy(strategy) {
        }
        
        JasonDumper (JasonSlice slice, JasonBuffer& buffer, UnsupportedTypeStrategy strategy) 
          : _slice(slice), _buffer(&buffer), _strategy(strategy) {
        }

        ~JasonDumper () {
        }

        void dump () {
          internalDump(_slice);
        }

      private:

        void internalDump (JasonSlice);

        void dumpInteger (JasonSlice);

        void dumpString (char const*, JasonLength);
        void dumpEscapedCharacter (uint32_t);
        void dumpHexCharacter (uint16_t);

        void handleUnsupportedType (JasonSlice /*slice*/) {
          if (_strategy == STRATEGY_SUPPRESS) {
            return;
          }

          throw JasonDumperError("unsupported type - cannot convert to JSON");
        }

      private:

        JasonSlice const _slice;

        JasonBuffer* _buffer;

        UnsupportedTypeStrategy _strategy;


    };

  }  // namespace triagens::basics
}  // namespace triagens

#endif
