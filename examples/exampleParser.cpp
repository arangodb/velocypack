#include <iostream>
#include <iomanip>
#include "velocypack/vpack.h"

using namespace arangodb::velocypack;

int main (int, char*[]) {
  // this is the JSON string we are going to parse
  std::string const json = "{\"a\":12}";
  
  Parser parser;
  try {
    size_t nr = parser.parse(json);
    std::cout << "Number of values: " << nr << std::endl;
  }
  catch (std::bad_alloc const&) {
    std::cout << "Out of memory!" << std::endl;
    throw;
  }
  catch (Exception const& e) {
    std::cout << "Parse error: " << e.what() << std::endl;
    std::cout << "Position of error: " << parser.errorPos() << std::endl;
    throw;
  }

  // get a pointer to the start of the data
  Builder b = parser.steal();
 
  // now dump the resulting VPack value
  std::cout << "Resulting VPack:" << std::endl;
  std::cout << HexDump(b.slice());
}
