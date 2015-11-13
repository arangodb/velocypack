#include <iostream>
#include "velocypack/vpack.h"

using namespace arangodb::velocypack;

int main (int, char*[]) {
  // don't sort the attribute names in the VPack object we construct
  // attribute name sorting is turned on by default, so attributes can
  // be quickly accessed by name. however, sorting adds overhead when
  // constructing VPack objects so it's optional. there may also be
  // cases when the original attribute order needs to be preserved. in
  // this case, turning off sorting will help, too 
  Options options;
  options.sortAttributeNames = false;

  Builder b(&options);
  // build an object with attribute names "b", "a", "l", "name"
  b(Value(ValueType::Object))
   ("b", Value(12))
   ("a", Value(true))
   ("l", Value(ValueType::Array))
     (Value(1)) (Value(2)) (Value(3)) ()
   ("name", Value("Gustav")) ();

  // a Slice is a lightweight accessor for a VPack value
  Slice s(b.start());
 
  // now dump the Slice into an std::string sink 
  std::string buffer;
  StringSink<std::string> sink(&buffer);
  Dumper dumper(&sink);
  dumper.dump(s);

  // and print it
  std::cout << "Resulting VPack:" << std::endl << buffer << std::endl;
}
