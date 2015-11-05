#include <iostream>
#include "velocypack/velocypack-common.h"
#include "velocypack/Builder.h"
#include "velocypack/Value.h"
#include "velocypack/ValueType.h"

using namespace arangodb::velocypack;

int main (int argc, char* argv[]) {
  Builder b;
  b(Value(ValueType::Object))
   ("b", Value(12))
   ("a", Value(true))
   ("l", Value(ValueType::Array))
     (Value(1)) (Value(2)) (Value(3)) ()
   ("name", Value("Gustav")) ();

  std::cout << "Resulting VPack:\n";
  uint8_t* p = b.start();
  ValueLength len = b.size();
  std::cout << std::hex;
  for (size_t i = 0; i < len; i++) {
    std::cout << (int) p[i] << " ";
  }
  std::cout << std::endl;
}
