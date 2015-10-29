#include <iostream>
#include <JasonBuilder.h>
#include <JasonSlice.h>
#include <JasonDump.h>

using namespace arangodb::jason;

int main (int argc, char* argv[]) {
  JasonBuilder b;
  b.options.sortAttributeNames = false;
  b(Jason(JasonType::Object))
   ("b", Jason(12))
   ("a", Jason(true))
   ("l", Jason(JasonType::Array))
     (Jason(1)) (Jason(2)) (Jason(3)) ()
   ("name", Jason("Gustav")) ();

  JasonSlice s(b.start());

  JasonCharBuffer buffer;
  JasonBufferDumper dumper(buffer, JasonBufferDumper::StrategyFail);
  dumper.dump(s);
  std::string output(buffer.data(), buffer.size());

  std::cout << "Resulting Jason:\n" << output << std::endl;
}
