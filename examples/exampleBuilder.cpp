#include <iostream>
#include <iomanip>
#include "velocypack/vpack.h"

using namespace arangodb::velocypack;

int main (int, char*[]) {
  // create an object with attributes "b", "a", "l" and "name"
  // note that the attribute names will be sorted in the target VPack object!
  Builder b;

  b.add(Value(ValueType::Object));
  b.add("b", Value(12));
  b.add("a", Value(true));
  b.add("l", Value(ValueType::Array));
  b.add(Value(1));
  b.add(Value(2));
  b.add(Value(3));
  b.close();
  b.add("name", Value("Gustav"));
  b.close();

  // get pointer to the start of the VPack data
  uint8_t* p = b.start();
  ValueLength len = b.size();
  
  // now dump the resulting VPack value
  std::cout << "Resulting VPack:" << std::endl;

  std::cout << std::hex;
  for (size_t i = 0; i < len; i++) {
    std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int) p[i] << " ";
  }
  std::cout << std::endl;
}
