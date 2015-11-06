#include <iostream>
#include <iomanip>
#include "velocypack/vpack.h"

using namespace arangodb::velocypack;

int main (int, char*[]) {
  // this is the JSON string we are going to parse
  std::string const json = "{\"a\":12}";
  
  Parser p;
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

  // get a pointer to the start of the data
  Builder b = p.steal();
  uint8_t* pp = b.start();
  ValueLength len = b.size();
 
  // now dump the resulting VPack value
  std::cout << "Resulting VPack:" << std::endl;
  for (size_t i = 0; i < len; i++) {
    std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int) pp[i] << " ";
  }
  std::cout << std::endl;
}
