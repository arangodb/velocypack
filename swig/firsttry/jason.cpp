#include <iostream>
#include "jason.h"

#include "JasonParser.h"
#include "JasonSlice.h"

using namespace arangodb::jason;

char* JsonToJason (std::string s) {
  JasonParser p;
  try {
    p.parse(s);
  } 
  catch (std::bad_alloc const& e) {
    std::cout << "Out of memory!" << std::endl;
  }
  catch (JasonException const& e) {
    return nullptr;
  }
  JasonBuilder b = p.steal();
  char* res = static_cast<char*>(malloc(b.size()));
  memcpy(res, b.start(), b.size());
  return res;
}

unsigned long int ByteLength(char* p) {
  JasonSlice s(p);
  return s.byteSize();
}
