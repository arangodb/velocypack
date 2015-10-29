#include <iostream>
#include <JasonBuilder.h>

using namespace arangodb::jason;

int main (int argc, char* argv[]) {
  JasonBuilder b;
  b(Jason(JasonType::Object))
   ("b", Jason(12))
   ("a", Jason(true))
   ("l", Jason(JasonType::Array))
     (Jason(1)) (Jason(2)) (Jason(3)) ()
   ("name", Jason("Gustav")) ();

  std::cout << "Resulting Jason:\n";
  uint8_t* p = b.start();
  JasonLength len = b.size();
  std::cout << std::hex;
  for (size_t i = 0; i < len; i++) {
    std::cout << (int) p[i] << " ";
  }
  std::cout << std::endl;
}
