
#include "JasonParser.h"

using JasonParser = triagens::basics::JasonParser;
        
bool JasonParser::WhiteSpaceTable[256];

void JasonParser::Initialize () {
  for (int i = 0; i < 256; ++i) {
    WhiteSpaceTable[i] = (i == ' ' || i == '\t' || i == '\n' || i == '\r' || i == '\f' || i == '\b');
  }
}

