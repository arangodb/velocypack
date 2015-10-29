#include <iostream>
#include <JasonBuilder.h>
#include <JasonSlice.h>

using namespace arangodb::jason;

int main (int argc, char* argv[]) {
  JasonBuilder b;
  b(Jason(JasonType::Object))
   ("b", Jason(12))
   ("a", Jason(true))
   ("l", Jason(JasonType::Array))
     (Jason(1)) (Jason(2)) (Jason(3)) ()
   ("name", Jason("Gustav")) ();

  JasonSlice s(b.start());
  
  JasonType t = s.type();
  std::cout << "Type: " << t << std::endl;
  if (s.isObject()) {
    JasonSlice ss = s.get("l");   // Now ss points to the subvalue under "l"
    std::cout << "Length of .l: " << ss.length() << std::endl;
    std::cout << "Second entry of .l:" << ss.at(1).getInt() << std::endl;
  }
  JasonSlice sss = s.get("name");
  if (sss.isString()) {
    JasonLength len;
    char const* st = sss.getString(len);
    std::cout << "Name in .name: " << std::string(st, len) << std::endl;
  }
}
