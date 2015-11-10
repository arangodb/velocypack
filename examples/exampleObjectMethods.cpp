#include <iostream>
#include "velocypack/vpack.h"

using namespace arangodb::velocypack;

int main (int, char*[]) {
  // create an object with a few members
  Builder b;

  b(Value(ValueType::Object));
  b.add("foo", Value(42)); 
  b.add("bar", Value("some string value")); 
  b.add("baz", Value(ValueType::Object));
  b.add("qux", Value(true));
  b.add("bart", Value("this is a string"));
  b.close();
  b.add("quux", Value(12345));
  b.close();

  // a Slice is a lightweight accessor for a VPack value
  Slice s(b.start());

  // get all object keys. returns a vector of strings
  for (auto& it : Collection::keys(s)) {
    // print key
    std::cout << "Object has key '" << it << "'" << std::endl;
  }

  // get all object values. returns a Builder object with an Array inside
  Builder values = Collection::values(s);
  for (auto it : ArrayIterator(values.slice())) {
    std::cout << "Object value is: " << it << ", as JSON: " << it.toJson() << std::endl;
  }
}
