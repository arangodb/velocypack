#ifndef JASON_UTILS_H
#define JASON_UTILS_H 1

#include "Jason.h"

namespace triagens {
  namespace basics {

    struct JasonUtils {

      // checks if the specified length is within the bounds of the system
#ifdef JASON_64BIT
      // the check is useless in 64 bit environment
      static inline void CheckSize (JasonLength) {
      }
#else
      // only useful in 32 bit-mode
      static void CheckSize (JasonLength);
#endif

    };

  }  // namespace triagens::basics
}  // namespace triagens

#endif
