#ifndef JASON_UTILS_H
#define JASON_UTILS_H 1

#include "Jason.h"

namespace triagens {
  namespace basics {

    struct JasonUtils {

#ifndef JASON_64BIT
      // check if the length is beyond the size of a SIZE_MAX on this platform
      static void CheckSize (JasonLength length) {
        if (length > static_cast<JasonLength>(SIZE_MAX)) {
          throw JasonException("JasonLength out of bounds.");
        }  
      }
#else
      static inline void CheckSize (JasonLength) { 
        // do nothing on a 64 bit platform 
      }
#endif
    };

  }  // namespace triagens::basics
}  // namespace triagens

#endif
