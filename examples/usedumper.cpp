#include <iostream>
#include "velocypack/velocypack-common.h"
#include "velocypack/Builder.h"
#include "velocypack/Dump.h"
#include "velocypack/Slice.h"
#include "velocypack/Value.h"
#include "velocypack/ValueType.h"

using namespace arangodb::velocypack;

int main (int argc, char* argv[]) {
  Builder b;
  b.options.sortAttributeNames = false;
  b(Value(ValueType::Object))
   ("b", Value(12))
   ("a", Value(true))
   ("l", Value(ValueType::Array))
     (Value(1)) (Value(2)) (Value(3)) ()
   ("name", Value("Gustav")) ();

  Slice s(b.start());

  CharBuffer buffer;
  BufferDumper dumper(buffer, BufferDumper::StrategyFail);
  dumper.dump(s);
  std::string output(buffer.data(), buffer.size());

  std::cout << "Resulting VPack:\n" << output << std::endl;
}
