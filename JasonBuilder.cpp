
#include "JasonBuilder.h"

using Jason        = triagens::basics::Jason;
using JasonBuilder = triagens::basics::JasonBuilder;
using JasonLength  = triagens::basics::JasonLength;
       
// thread local vector for sorting small object attributes
thread_local std::vector<JasonBuilder::SortEntrySmall> JasonBuilder::SortObjectSmallEntries;

// thread local vector for sorting large object attributes
thread_local std::vector<JasonBuilder::SortEntryLarge> JasonBuilder::SortObjectLargeEntries;
           
