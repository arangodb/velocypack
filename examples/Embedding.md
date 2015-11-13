Embedder info for the VPack library
===================================

Minimal example
---------------

Let's start with a small example program *test.cpp* that uses the VPack library.
It does nothing yet, the only goal is to make this minimal example compile and
link:

```cpp
#include <velocypack/vpack.h>
#include <iostream>

using namespace arangodb::velocypack;

int main () {
  std::cout << valueTypeName(ValueType::Object) << std::endl;
}
```

To make the VPack classes available in your project, add the VPack headers
to the list of include directories. How exactly this works is compiler-specific.
For example, when using g++ or clang, include directories can be added using 
the `-I` compiler option.

When compiling the program, please make sure the compiler can understand C++11
syntax. In g++ and clang, this can be controlled via the `-std=c++11` option.

Additionally, the velocypack library must be linked to the example program.
In g++ this works by specifying the libary path with the `-L` option and specifying
the library's name.

The full instruction to compile and link the test program with g++ is:

```bash
g++ -std=c++11 -I/usr/local/include -L/usr/local/lib  main.cpp -lvelocypack -o test
```

With clang, it is:

```bash
clang++ -std=c++11 -I/usr/local/include -L/usr/local/lib  main.cpp -lvelocypack -o test
```

The test program can afterwards be run with

```bash
./test
```

With a working infrastructure for compiling and linking the VPack library,
you can now go ahead and adjust the example program so it does something useful. 
There are some working usage examples in this directory.


Exceptions and error reporting
------------------------------

The VPack library's way of signaling errors is to throw exceptions. Thus VPack
library users need to make sure they handle exceptions properly.

The VPack library will mostly throw exceptions of type `arangodb::vpack::Exception`. 
The library's `Exception` class is derived from `std::exception` and provides the 
`what` method to retrieve the error message from the exception.

Additionally, `Exception` provides the `errorCode` method for retrieving a
numeric error code from an exception. This error code can be used to check for
specific error types programmatically.

```cpp
Builder b;
b.add(Value(ValueType::Object));
try {
  // will fail as we should rather add a key/value pair here
  b.add(Value(ValueType::Null));
}
catch (Exception const& ex) {
 std::cout << "caught exception w/ code " << ex.errorCode()
           << ", msg: " << ex.what() << std::endl;
}
```

Additionally, the VPack library may throw standard exceptions such as
`std::bad_alloc` when appropriate. 

If no special treatment of VPack exceptions is needed by the client 
application, it is sufficient to only catch exceptions of type `std::exception`,
as the VPack `Exception` class is derived from it:

Builder b;
b.add(Value(ValueType::Object));
try {
  // will fail as we should rather add a key/value pair here
  b.add(Value(ValueType::Null));
}
catch (std::exception const& ex) {
 std::cout << "caught exception: " << ex.what() << std::endl;
}


Thread safety
-------------

Thread-safety was no design goal for VPack, so objects in the VPack library
are not thread-safe. VPack objects can be passed between threads though,
but if the same object is accessed concurrently, client applications need
to make sure they employ an appropriate locking mechanism on top.


Memory ownership and memory leaks
---------------------------------

In most cases there is no need to deal with raw memory when working with
VPack. By default, all VPack objects manage their own memory. This will also
avoid memory leaks.

It is encouraged to construct VPack objects on the stack rather than using
`new`/`delete`. This will greatly help avoiding memory leaks in client
code that uses the VPack library.

Special care must be taken for `Slice` objects: a `Slice` object contains a
pointer to memory where a VPack value is stored, and the client code needs
to make sure this memory is still valid when the `Slice` is accessed.

Here is a valid example for using a `Slice`:

```cpp
{
  Builder b;
  b.add(Value("this is a test"));

  // this Slice object is referencing memory owned by the Builder b
  // this works here as b is still available
  Slice s(b.start());

  // do something with Slice s in this scope...
}
```

Here is an invalid usage example, returning a `Slice` object that will
point to deallocated memory:

```cpp
Slice getSlice () {
  Builder b;
  b.add(Value("this is a test"));

  // this Slice object is referencing memory owned by the Builder b
  // this works here as b is still available
  Slice s(b.start());

  // the following return statement will make the Builder b go out of
  // scope. this will deallocate the memory owned by b, and accessing the
  // Slice s that points to b's memory will result in undefined behavior

  return s; // will return a Slice pointing to deallocated memory !!
}
```

In the latter case it would have been better to return the `Builder`
object from the function and not the `Slice`.


VPack `Buffer` objects also manage their own memory. When a `Buffer`
object goes out of scope, it will deallocate any dynamic memory it has
allocated. Client-code must not access the `Buffer` object's memory
after that.


Name clashes and class aliases
------------------------------

The default way of making the VPack library's classes available to a client
program is to include the header `velocypack/vpack.h`. This will load the
classes definitions from namespace `arangodb::velocypack`.

To avoid full name qualification in client programs, it may be convenient to
make all classes from this namespace available without extra qualification.
The following line will do that:

```cpp
using namespace arangodb::velocypack`
```

However, this can lead to name clashes in the client application. For example,
the VPack library contains classes named `Buffer`, `Exception`, `Parser` -
class names which are not uncommon in many projects.

If for this reason importing the whole `arangodb::velocypack` namespace is 
not an option, an alternative is to use the class name aliases that are
defined in the header file `velocypack/velocypack-aliases.h`. 

This header file makes the most common VPack classes available under alternative 
(hopefully unambiguous) class names with the prefix *VPack*:

```cpp
using VPackArrayIterator      = arangodb::velocypack::ArrayIterator;
using VPackBuilder            = arangodb::velocypack::Builder;
using VPackCharBuffer         = arangodb::velocypack::CharBuffer;
using VPackCharBufferSink     = arangodb::velocypack::CharBufferSink;
using VPackCollection         = arangodb::velocypack::Collection;
using VPackDumper             = arangodb::velocypack::Dumper;
using VPackException          = arangodb::velocypack::Exception;
using VPackHexDump            = arangodb::velocypack::HexDump;
using VPackObjectIterator     = arangodb::velocypack::ObjectIterator;
using VPackOptions            = arangodb::velocypack::Options;
using VPackParser             = arangodb::velocypack::Parser;
using VPackSink               = arangodb::velocypack::Sink;
using VPackSlice              = arangodb::velocypack::Slice;
using VPackStringSink         = arangodb::velocypack::StringSink;
using VPackStringStreamSink   = arangodb::velocypack::StringStreamSink;
using VPackValue              = arangodb::velocypack::Value;
using VPackValueLength        = arangodb::velocypack::ValueLength;
using VPackValueType          = arangodb::velocypack::ValueType;
using VPackVersion            = arangodb::velocypack::Version;
```
