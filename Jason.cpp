
#include "Jason.h"

using Jason       = triagens::basics::Jason;
using JasonLength = triagens::basics::JasonLength;
        
// maximum String length (all longer strings must be of type StringLong) 
JasonLength const Jason::MaxLengthString = 127;

