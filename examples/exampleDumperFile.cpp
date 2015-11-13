#include <iostream>
#include <fstream>
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
 
  // now dump the Slice into an outfile
  Options dumperOptions;
  dumperOptions.prettyPrint = true;

  // this is our output file
  try {
    std::ofstream ofs("prettified.json", std::ofstream::out);

    OutputFileStreamSink sink(&ofs);
    Dumper::dump(s, &sink, &options);
    std::cout << "successfully wrote JSON to outfile 'prettified.json'" << std::endl;
  }
  catch (std::exception const& ex) {
    std::cout << "could not write outfile 'prettified.json': " << ex.what() << std::endl;
  }
}
