VelocyPack (VPack) - a fast and compact serialization format
============================================================

Master: [![Build Status](https://secure.travis-ci.org/arangodb/velocypack.png?branch=master)](http://travis-ci.org/arangodb/velocypack)

Motivation
----------

These days, JSON (JavaScript Object Notation, see [ECMA-404]
(http://www.ecma-international.org/publications/files/ECMA-ST/ECMA-404.pdf))
is used in many cases where data has to be exchanged.
Lots of protocols between different services use it, databases store
JSON (document stores naturally, but others increasingly as well). It
is popular, because it is simple, human-readable, and yet surprisingly
versatile, despite its limitations.

At the same time there are a plethora of alternatives ranging from XML
over Universal Binary JSON, MongoDB's BSON, MessagePack, BJSON (binary
JSON), Apache Thrift till Google's protocol buffers and ArangoDB's
shaped JSON.

When looking into this, we were surprised to find that none of these
formats manages to combine compactness, platform independence, fast
access to subobjects and rapid conversion from and to JSON. 

We have invented VPack because we need a binary format that

  - is compact
  - covers all of JSON plus dates, integers, binary data and arbitrary 
    precision numbers
  - can be used in a database kernel to access subdocuments for
    example for indexes, so it must be possible to access subdocuments
    (array and object members) efficiently
  - can be transferred to JSON and from JSON rapidly
  - avoids too many memory allocations
  - gives flexibility to assemble objects, such that subobjects reside
    in the database in an unchanged way
  - allows to use an external table for frequently used attribute names
  - quickly allows to read off the type and length of a given object

This data format must be backed by good C++ classes to allow

  - easy and fast parsing from JSON
  - easy and convenient buildup without too many memory allocations
  - fast access of subobjects (arrays and objects)
  - flexible memory management
  - fast dumping to JSON

The VPack format is an attempt to achieve all this.


Specification
-------------

See [the file VelocyPack.md](VelocyPack.md) for a detailed description.


Performance
-----------

See [the file Performance.md](Performance.md) for a thorough comparison
to other formats like JSON itself, MessagePack and BSON. We look at file
sizes and parsing and conversion performance.


Building the VPack library
--------------------------

Building the VPack library is straightforward. Simply execute the 
following commands:

```bash
cmake . 
make
```

This will build a static library `libvelocypack.a` in the current directory in
release mode.


Running the tests and the benchmark suite
-----------------------------------------

Building VPack's own test suite requires the [googletest framework](https://github.com/google/googletest)
to be built. To build the tests, run cmake with the option `-DBuildTests=ON`:

```bash
cmake -DBuildTests=ON . 
make
```

Afterwards, you can run the tests via:

```bash
./tests
```

The benchmark suite compares VPack against [RapidJson](https://github.com/miloyip/rapidjson).
RapidJson is not shipped with VPack, but it can be downloaded into the
local subdirectory `rapidjson` with the following command:

```bash
./download-rapidjson.sh
```

Afterwards, you are ready to build the benchmark suite:

```bash
cmake . -DBuildBench=ON -DBuildTests=ON
make
```

and then run the benchmark suite via

```bash
./runBench.sh

```

Build Options
-------------

The following options can be set for building the VPack library:

* `-DCMAKE_BUILD_TYPE=Release`: builds the VPack library in release mode. This
  does not build debug symbols and turns on all optimizations. Use this mode for 
  production.
* `-DCMAKE_BUILD_TYPE=Debug`: builds the VPack library in debug mode. This
  adds debug symbols and turns off optimizations. Use this mode for development,
  but not for production or performance testing.
* `-DBuildBench`: controls whether the benchmark suite should be built. The 
  default is `OFF`, meaning the suite will not be built. Set the option to `ON` to
  build it. Building the benchmark suite requires the subdirectory *rapidjson* to 
  be present (see below). 
* `-DBuildTests`: controls whether VPack's own test suite should be built. The
  default is `OFF`. Set the option to `ON` for building the tests. This requires 
  the subdirectory *googletest* to be present (see below). 
* `-DEnableSSE`: controls whether SSE4.2 optimizations are compiled into the
  library. The default is either `ON` or `OFF`, depending on the detected SSE4.2
  support of the host platform.


Using the VPack library
-----------------------

Here are some examples of how the VPack library is used in C++ code.
For a detailed documentation of the C++ classes see [the API
documentation](API.md).

### Building up VPack objects from scratch

If you want to build up a VPack object corresponding to this JSON
object:

```json
{
  "b": 12,
  "a": true,
  "l": [1, 2, 3],
  "name": "Gustav"
}
```

then you would use the `Builder` class as follows:

```cpp
#include <velocyPack/vpack.h>

using namespace arangodb::velocypack;

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
```

The resulting VPack object can now be found at the memory location that
the `uint8_t*` 

```cpp
b.start()
```

points to, and its byte length can be determined via

```cpp
b.size()
```

One can see that adding objects and arrays essentially "opens" them such
that further additions go into the subvalue until one calls the
`close()` method. The `Value` class is a slim wrapper class to use the
C++ type system for compact notation.

If you fancy syntactic sugar using C++ callable objects, you could also
write:

```cpp
#include <velocyPack/vpack.h>

using namespace arangodb::velocypack;

Builder b;

b(Value(ValueType::Object))
 ("b", Value(12))
 ("a", Value(true))
 ("l", Value(ValueType::Array))
   (Value(1)) (Value(2)) (Value(3)) ()
 ("name", Value("Gustav")) ();
```

### Parsing JSON into VPack in memory

To create a VPack object from a JSON string, use the JSON parser shipped
with VPack as follows:

```cpp
#include <velocypack/vpack.h>

using namespace arangodb::velocypack;

Parser p;
std::string const json = "{\"a\":12}";
try {
  size_t nr = p.parse(json);
}
catch (std::bad_alloc const& e) {
  std::cout << "Out of memory!" << std::endl;
}
catch (Exception const& e) {
  std::cout << "Parse error: " << e.what() << std::endl;
  std::cout << "Position of error: " << p.errorPos() << std::endl;
}
Builder b = p.steal();
```

The final `steal()` method is very efficient and does not copy the result.
You can access the resulting VPack object as above via `b`.


### Accessing subvalues of a VPack object

The class `Slice` is used for this. It only needs (and stores) the 
starting position of the VPack value in memory and can derive the type
and length from that. Therefore it is very cheap to create and destroy
`Slice`s.

```cpp
#include <velocypack/vpack.h>

using namespace arangodb::velocypack;

Slice s(p);
ValueType t = s.type();    // should be ValueType::Object

if (s.isObject()) {
  Slice ss = s.get("l");   // Now ss points to the subvalue under "l"
  ValueLength l = ss.length();
  Slice ss3 = ss.at(1);
  int i = ss3.getInt();
}

Slice sss = s.get("name");
if (sss.isString()) {
  ValueLength len;
  std::string name = sss.getString(len);
}
```

### Dumping a VPack value to JSON

The template class `Dumper` can be used for this. It in turn uses the 
`Slice` class. Use as follows, if `Slice` object `s` points to
a valid VPack value:

```cpp
#include <velocypack/vpack.h>

CharBuffer buffer;
BufferDumper dumper(buffer);
dumper.dump(s);
std::string output(buffer.data(), buffer.size());
```

