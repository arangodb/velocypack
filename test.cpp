#include "JasonBuilder.h"
#include "JasonParser.h"
#include "JasonSlice.h"
#include "JasonType.h"

#include <iostream>

using JasonSlice = triagens::basics::JasonSlice;
using JasonType = triagens::basics::JasonType;
  
static char Buffer[4096];

static void TestNull () {
  Buffer[0] = 0x0;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::Null);
  assert(slice.isNull());
  assert(slice.byteSize() == 0);
}

static void TestFalse () {
  Buffer[0] = 0x1;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::Bool);
  assert(slice.isBool());
  assert(slice.byteSize() == 0);
  assert(slice.getBool() == false);
}

static void TestTrue () {
  Buffer[0] = 0x2;

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::Bool);
  assert(slice.isBool());
  assert(slice.byteSize() == 0);
  assert(slice.getBool() == true);
}

static void TestDouble () {
  Buffer[0] = 0x3;
  double value = 23.5;
  memcpy(&Buffer[1], (void*) &value, sizeof(value));

  JasonSlice slice(reinterpret_cast<uint8_t const*>(&Buffer[0]));

  assert(slice.type() == JasonType::Double);
  assert(slice.isDouble());
  assert(slice.byteSize() == 8);
  assert(slice.getDouble() == value);
}

int main (int argc, char* argv[]) {
  JasonSlice::Initialize();

  TestNull();
  TestFalse();
  TestTrue();
  TestDouble();
 
  std::cout << "ye olde tests passeth.\n";
  return 0;
}
