#ifndef JASON_UTILS_H
#define JASON_UTILS_H 1

#include "Jason.h"

namespace triagens {
  namespace basics {

    struct JasonUtils {

      // checks if the specified length is within the bounds of the system
      static void CheckSize (JasonLength);

    };

  }  // namespace triagens::basics
}  // namespace triagens

#endif
