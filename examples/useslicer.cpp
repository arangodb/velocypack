#include <iostream>
#include "velocypack/velocypack-common.h"
#include "velocypack/Builder.h"
#include "velocypack/Slice.h"
#include "velocypack/Value.h"
#include "velocypack/ValueType.h"

using namespace arangodb::velocypack;

int main (int argc, char* argv[]) {
  Builder b;
  b(Value(ValueType::Object))
   ("b", Value(12))
   ("a", Value(true))
   ("l", Value(ValueType::Array))
     (Value(1)) (Value(2)) (Value(3)) ()
   ("name", Value("Gustav")) ();

  Slice s(b.start());
  
  ValueType t = s.type();
  std::cout << "Type: " << t << std::endl;
  if (s.isObject()) {
    Slice ss = s.get("l");   // Now ss points to the subvalue under "l"
    std::cout << "Length of .l: " << ss.length() << std::endl;
    std::cout << "Second entry of .l:" << ss.at(1).getInt() << std::endl;
  }
  Slice sss = s.get("name");
  if (sss.isString()) {
    ValueLength len;
    char const* st = sss.getString(len);
    std::cout << "Name in .name: " << std::string(st, len) << std::endl;
  }
}
