
#include "JasonBuilder.h"
#include "JasonDumper.h"
#include "JasonSlice.h"

using Jason             = arangodb::jason::Jason;
using JasonBuilder      = arangodb::jason::JasonBuilder;
using JasonLength       = arangodb::jason::JasonLength;
using JasonPrettyDumper = arangodb::jason::JasonStringPrettyDumper;
using JasonSlice        = arangodb::jason::JasonSlice;
       
std::string JasonSlice::toString () const {
  return JasonPrettyDumper::Dump(this);
}

