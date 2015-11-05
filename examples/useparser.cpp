#include <iostream>
#include "velocypack/velocypack-common.h"
#include "velocypack/Builder.h"
#include "velocypack/Exception.h"
#include "velocypack/Parser.h"

using namespace arangodb::velocypack;

int main (int argc, char* argv[]) {
  Parser p;
  std::string json = "{\"a\":12}";
  try {
    size_t nr = p.parse(json);
    std::cout << "Number of values: " << nr << std::endl;
  }
  catch (std::bad_alloc const& e) {
    std::cout << "Out of memory!" << std::endl;
  }
  catch (Exception const& e) {
    std::cout << "Parse error: " << e.what() << std::endl;
    std::cout << "Position of error: " << p.errorPos() << std::endl;
  }
  Builder b = p.steal();

  std::cout << "Resulting VPack:\n";
  uint8_t* pp = b.start();
  ValueLength len = b.size();
  std::cout << std::hex;
  for (size_t i = 0; i < len; i++) {
    std::cout << (int) pp[i] << " ";
  }
  std::cout << std::endl;
}
