#include <iostream>
#include <JasonBuilder.h>

using namespace arangodb::jason;

int main (int argc, char* argv[]) {
  JasonBuilder b;
  b.add(Jason(JasonType::Object));
  b.add("b", Jason(12));
  b.add("a", Jason(true));
  b.add("l", Jason(JasonType::Array));
  b.add(Jason(1));
  b.add(Jason(2));
  b.add(Jason(3));
  b.close();
  b.add("name", Jason("Gustav"));
  b.close();

  std::cout << "Resulting Jason:\n";
  uint8_t* p = b.start();
  JasonLength len = b.size();
  std::cout << std::hex;
  for (size_t i = 0; i < len; i++) {
    std::cout << (int) p[i] << " ";
  }
  std::cout << std::endl;
}
