API documentation for the VPack libraries
=========================================

To make the VPack classes available in your project, use the following
include directive:

```cpp
#include <velocypack/vpack.h>
```

This will make the frequently used classes from namespace `arangodb::velocypack`
available. In order to avoid full class name qualification, you may use a
`using namespace` directive:

```cpp
using namespace arangodb::velocypack`
```

An alternative to this is to make just the required classes from the 
`arangodb::velocypack` namespace available via their original names, e.g.:

```cpp
using Builder   = arangodb::velocypack::Builder;
using Parser    = arangodb::velocypack::Parser;
using Slice     = arangodb::velocypack::Slice;
using Value     = arangodb::velocypack::Value;
using ValueType = arangodb::velocypack::ValueType;
```

A third alternative is to include the header `velocypack/velocypack-aliases.h`,
which makes the classes from the `arangodb::velocypack` namespace
available via alternative names that potentially do not clash with existing
class names in your project. It makes all VPack class available as classes
starting with prefix `VPack`:

```cpp
using VPackBufferDumper       = arangodb::velocypack::BufferDumper;
using VPackBuilder            = arangodb::velocypack::Builder;
using VPackCharBuffer         = arangodb::velocypack::CharBuffer;
using VPackException          = arangodb::velocypack::Exception;
using VPackOptions            = arangodb::velocypack::Options;
using VPackParser             = arangodb::velocypack::Parser;
using VPackStringDumper       = arangodb::velocypack::StringDumper;
using VPackStringPrettyDumper = arangodb::velocypack::StringPrettyDumper;
using VPackSlice              = arangodb::velocypack::Slice;
using VPackValue              = arangodb::velocypack::Value;
using VPackValueType          = arangodb::velocypack::ValueType;
```

## Buffer

## Builder

This class organizes the buildup of a VPack object. It manages
the memory allocation and allows convenience methods to build
the object up recursively.

Use as follows, to build a VPack like on the right hand side:

```cpp
// create an object with attributes "b", "a", "l" and "name"
// note that the attribute names will be sorted in the target VPack object!
Builder b;

b.add(Value(ValueType::Object));
b.add("b", Value(12));
b.add("a", Value(true));
b.add("l", Value(ValueType::Array));
b.add(Value(1));
b.add(Value(2));
b.add(Value(3));
b.close();
b.add("name", Value("Gustav"));
b.close();

// get pointer to the start of the VPack data
uint8_t* p = b.start();
ValueLength len = b.size();

// now dump the resulting VPack value
std::cout << "Resulting VPack:" << std::endl;

std::cout << std::hex;
for (size_t i = 0; i < len; i++) {
  std::cout << (int) p[i] << " ";
}
std::cout << std::endl;
```

Or, if you like fancy syntactic sugar:

```cpp
// create an object with attributes "b", "a", "l" and "name"
// note that the attribute names will be sorted in the target VPack object!
Builder b;

b(Value(ValueType::Object))
  ("b", Value(12))
  ("a", Value(true))
  ("l", Value(ValueType::Array))
    (Value(1)) (Value(2)) (Value(3)) ()
  ("name", Value("Gustav")) ();

// get pointer to the start of the VPack data
uint8_t* p = b.start();
ValueLength len = b.size();

// now dump the resulting VPack value
std::cout << "Resulting VPack:" << std::endl;

std::cout << std::hex;
// and dump it to stdout
for (size_t i = 0; i < len; i++) {
  std::cout << (int) p[i] << " ";
}
std::cout << std::endl;
```

## Slice

```cpp
// create an object with attributes "b", "a", "l" and "name"
// note that the attribute names will be sorted in the target VPack object!
Builder b;

b(Value(ValueType::Object))
  ("b", Value(12))
  ("a", Value(true))
  ("l", Value(ValueType::Array))
    (Value(1)) (Value(2)) (Value(3)) ()
  ("name", Value("Gustav")) ();

// a Slice is a lightweight accessor for a VPack value
Slice s(b.start());

// inspect the outermost value (should be an Object...) 
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
```

## Parser

```cpp
// this is the JSON string we are going to parse
std::string const json = "{\"a\":12}";

Parser p;
try {
  size_t nr = p.parse(json);
  std::cout << "Number of values: " << nr << std::endl;
}
catch (std::bad_alloc const& e) {
  std::cout << "Out of memory!" << std::endl;
}
catch (Exception const& e) {
  std::cout << "Parse error: " << e.what() << std::endl;
  std::cout << "Position of error: " << p.errorPos() << std::endl;
}

// get a pointer to the start of the data
Builder b = p.steal();
uint8_t* pp = b.start();
ValueLength len = b.size();

// now dump the resulting VPack value
std::cout << "Resulting VPack:" << std::endl;
std::cout << std::hex;
for (size_t i = 0; i < len; i++) {
  std::cout << (int) pp[i] << " ";
}
std::cout << std::endl;
```

## Dumper

```cpp
Builder b;
// don't sort the attribute names in the VPack object we construct
// attribute name sorting is turned on by default, so attributes can
// be quickly accessed by name. however, sorting adds overhead when
// constructing VPack objects so it's optional. there may also be
// cases when the original attribute order needs to be preserved. in
// this case, turning off sorting will help, too 
b.options.sortAttributeNames = false;

// build an object with attribute names "b", "a", "l", "name"
b(Value(ValueType::Object))
  ("b", Value(12))
  ("a", Value(true))
  ("l", Value(ValueType::Array))
    (Value(1)) (Value(2)) (Value(3)) ()
  ("name", Value("Gustav")) ();

// a Slice is a lightweight accessor for a VPack value
Slice s(b.start());

// now dump the Slice into a Buffer 
CharBuffer buffer;
BufferDumper dumper(buffer);
dumper.dump(s);
std::string output(buffer.data(), buffer.size());

// and print it
std::cout << "Resulting VPack:" << std::endl << output << std::endl;
```
