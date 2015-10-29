#include <iostream>
#include <JasonParser.h>

using namespace arangodb::jason;

int main (int argc, char* argv[]) {
  JasonParser p;
  std::string json = "{\"a\":12}";
  try {
    size_t nr = p.parse(json);
    std::cout << "Number of values: " << nr << std::endl;
  }
  catch (std::bad_alloc const& e) {
    std::cout << "Out of memory!" << std::endl;
  }
  catch (JasonException const& e) {
    std::cout << "Parse error: " << e.what() << std::endl;
    std::cout << "Position of error: " << p.errorPos() << std::endl;
  }
  JasonBuilder b = p.steal();

  std::cout << "Resulting Jason:\n";
  uint8_t* pp = b.start();
  JasonLength len = b.size();
  std::cout << std::hex;
  for (size_t i = 0; i < len; i++) {
    std::cout << (int) pp[i] << " ";
  }
  std::cout << std::endl;
}
