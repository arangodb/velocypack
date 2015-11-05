#include <iostream>
#include "velocypack/velocypack-common.h"
#include "velocypack/Builder.h"
#include "velocypack/Value.h"
#include "velocypack/ValueType.h"

using namespace arangodb::velocypack;

int main (int argc, char* argv[]) {
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

  std::cout << "Resulting VPack:\n";
  uint8_t* p = b.start();
  ValueLength len = b.size();
  std::cout << std::hex;
  for (size_t i = 0; i < len; i++) {
    std::cout << (int) p[i] << " ";
  }
  std::cout << std::endl;
}
