
#include "JasonBuilder.h"
#include "JasonDumper.h"
#include "JasonSlice.h"

using Jason             = triagens::basics::Jason;
using JasonBuilder      = triagens::basics::JasonBuilder;
using JasonLength       = triagens::basics::JasonLength;
using JasonPrettyDumper = triagens::basics::JasonStringPrettyDumper;
using JasonSlice        = triagens::basics::JasonSlice;
       
// thread local vector for sorting small object attributes
thread_local std::vector<JasonBuilder::SortEntrySmall> JasonBuilder::SortObjectSmallEntries;

// thread local vector for sorting large object attributes
thread_local std::vector<JasonBuilder::SortEntryLarge> JasonBuilder::SortObjectLargeEntries;

std::string JasonSlice::toString () const {
  return JasonPrettyDumper::Dump(this);
}

