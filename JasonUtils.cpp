
#include "JasonUtils.h"

using JasonException = triagens::basics::JasonException;
using JasonLength    = triagens::basics::JasonLength;
using JasonUtils     = triagens::basics::JasonUtils;

#ifndef JASON_64BIT
void JasonUtils::CheckSize (JasonLength length) {
  // check if the length is beyond the size of a SIZE_MAX on this platform
  if (length > static_cast<JasonLength>(SIZE_MAX)) {
    throw JasonException("JasonLength out of bounds.");
  } 
}
#endif

