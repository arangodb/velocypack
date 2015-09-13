
#include "JasonUtils.h"

using JasonException = triagens::basics::JasonException;
using JasonLength    = triagens::basics::JasonLength;
using JasonUtils     = triagens::basics::JasonUtils;

void JasonUtils::CheckSize (JasonLength length) {
  if (length > static_cast<JasonLength>(SIZE_MAX)) {
    throw JasonException("JasonLength out of bounds.");
  } 
}

