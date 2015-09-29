#ifndef JASON_UTILS_H
#define JASON_UTILS_H 1

#include "Jason.h"

namespace triagens {
  namespace basics {

    struct JasonUtils {

#ifndef JASON_64BIT
      // checks if the specified length is within the bounds of the system
      static void CheckSize (JasonLength);
#else
      static inline void CheckSize (JasonLength) { 
        // do nothing on a 64 bit platform 
      }
#endif
    };

  }  // namespace triagens::basics
}  // namespace triagens

#endif
