
#include "JasonUtils.h"

using JasonException = triagens::basics::JasonException;
using JasonLength    = triagens::basics::JasonLength;
using JasonUtils     = triagens::basics::JasonUtils;

void JasonUtils::CheckSize (JasonLength length) {
#ifndef JASON_64BIT
  // check if the length is beyond the size of a SIZE_MAX on this platform
  if (length > static_cast<JasonLength>(SIZE_MAX)) {
    throw JasonException("JasonLength out of bounds.");
  } 
#endif

  if (length > 281474976710656ULL) {
    // 2 ^ (6 * 8) is the maximum JasonLength value allowed
    throw JasonException("JasonLength out of bounds.");
  }
}
