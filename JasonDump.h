#ifndef JASON_DUMP_H
#define JASON_DUMP_H 1

#include <string>

#include "JasonBuffer.h"
#include "JasonDumper.h"

namespace arangodb {
  namespace jason {

    // some alias types for easier usage
    typedef JasonDumper<JasonCharBuffer, false> JasonBufferDumper;
    typedef JasonDumper<std::string, false>     JasonStringDumper;
    typedef JasonDumper<std::string, true>      JasonStringPrettyDumper;

  }  // namespace arangodb::jason
}  // namespace arangodb

#endif
