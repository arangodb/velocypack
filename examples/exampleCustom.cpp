#include <iostream>
#include "velocypack/vpack.h"

using namespace arangodb::velocypack;
  
// this is our handler for custom types 
struct MyCustomTypeHandler : public CustomTypeHandler {
  // serialize a custom type into JSON
  static int const myMagicNumber = 42;

  void toJson (Slice const& value, Dumper* dumper, Slice const&) {
    Sink* sink = dumper->sink();

    if (value.head() == 0xf0) {
      sink->append(std::to_string(myMagicNumber));
      return;
    }
    if (value.head() == 0xf1) {
      char const* start = value.startAs<char const>();
      // read string length
      uint8_t length = *(value.start() + 1);
      // append string (don't care about escaping here...)
      sink->push_back('"');
      sink->append(start + 2, static_cast<ValueLength>(length));
      sink->push_back('"');
      return;
    }
    throw "unknown type!";
  }

  ValueLength byteSize (Slice const& value) {
    if (value.head() == 0xf0) {
      // this custom type uses 1 byte only
      return 1; 
    }
    if (value.head() == 0xf1) {
      uint8_t const* start = value.start();
      // read string length
      uint8_t length = *(start + 1);
      // 1 byte for the type, 1 byte for the length info + x bytes for the actual string
      return static_cast<ValueLength>(1 + 1 + length); 
    }
    throw "unknown type!";
  }
};


int main (int, char*[]) {
  MyCustomTypeHandler handler;
  Options options;
  options.customTypeHandler = &handler;
  
  Builder b(&options);

  b(Value(ValueType::Object));

  uint8_t* p;
  // create an attribute "custom1" of type 0xf0 with bytesize 1
  // this type will be serialized into the value of 42
  p = b.add("custom1", ValuePair(1ULL, ValueType::Custom));
  *p = 0xf0;
  
  // create an attribute "custom2" of type 0xf1 with bytesize 8
  // this type will be contain a user-defined string type, with 
  // one byte of string length information following the Slice's
  // head byte, and the string bytes are it.
  p = b.add("custom2", ValuePair(8ULL, ValueType::Custom));
  *p++ = 0xf1;
  // fill it with something useful...
  *p++ = static_cast<uint8_t>(6); // length of following string
  *p++ = 'f'; *p++ = 'o'; *p++ = 'o'; *p++ = 'b'; *p++ = 'a'; *p++ = 'r';
 
  // another 0xf1 value 
  p = b.add("custom3", ValuePair(5ULL, ValueType::Custom));
  *p++ = 0xf1;
  *p++ = static_cast<uint8_t>(3); // length of following string
  *p++ = 'q'; *p++ = 'u'; *p++ = 'x';

  b.close();

  Slice s = b.slice();

  // now print 'custom1':
  std::cout << "'custom1': byteSize: " << s.get("custom1").byteSize() 
            << ", JSON: " << s.get("custom1").toJson() << std::endl; 

  // and 'custom2':
  std::cout << "'custom2': byteSize: " << s.get("custom2").byteSize() 
            << ", JSON: " << s.get("custom2").toJson() << std::endl; 
  
  // and 'custom3':
  std::cout << "'custom3': byteSize: " << s.get("custom3").byteSize() 
            << ", JSON: " << s.get("custom3").toJson() << std::endl; 
}
