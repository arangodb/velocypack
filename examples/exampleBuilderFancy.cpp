#include <iostream>
#include <iomanip>
#include "velocypack/vpack.h"

using namespace arangodb::velocypack;

int main (int, char*[]) {
  // create an object with attributes "b", "a", "l" and "name"
  // note that the attribute names will be sorted in the target VPack object!
  Builder b;

  b(Value(ValueType::Object))
   ("b", Value(12))
   ("a", Value(true))
   ("l", Value(ValueType::Array))
     (Value(1)) (Value(2)) (Value(3)) ()
   ("name", Value("Gustav")) ();

  // get pointer to the start of the VPack data
  uint8_t* p = b.start();
  ValueLength len = b.size();
  
  // now dump the resulting VPack value
  std::cout << "Resulting VPack:" << std::endl;

  std::cout << std::hex;
  // and dump it to stdout
  for (size_t i = 0; i < len; i++) {
    std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int) p[i] << " ";
  }
  std::cout << std::endl;
}
