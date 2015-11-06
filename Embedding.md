Embedder info for the VPack library
===================================

Exceptions and error reporting
------------------------------

The VPack library way of signaling errors is to throw exceptions. Thus VPack
library users need to make sure they handle exceptions properly.

The VPack library will mostly throw exceptions of type `Exception`. The library's
`Exception` class is derived from `std::exception` and provides the `what` method
to retrieve the error message from the exception.

Additionally, `Exception` provides the `errorCode` method for retrieving a
numeric error code from an exception. This error code can be used to check for
specific errors programmatically.

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
`new`/`delete`. This will greatly help avoiding memory leaks in the client
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

Here is an invalid usage example:

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

In the latter case, it would have been better to return the `Builder`
object from the function and not the `Slice`.


VPack `Buffer` objects also manage their own memory. When a `Buffer`
object goes out of scope, it will deallocate any dynamic memory it has
allocated. Client-code must not access the `Buffer` objects memory
after that.


Name clashes and class aliases
------------------------------

The default way of making the VPack library's classes available to a client
program is to include the header `velocypack/vpack.h`. This will load the
classes definitions from namespace `arangodb::velocypack`.

To avoid full name qualification in client programs, it may be convenient to
make all classes from this namespace available without extra qualification:

```cpp
using namespace arangodb::velocypack`
```

This however can lead to name clashes in client application. For example,
the VPack library contains classes named `Buffer`, `Exception`, `Parser` -
class names which are not uncommon in many projects.

When importing the `arangodb::velocypack` namespace is not an option, an
alternative to use the class name aliases for VPack classes as defined in
header `velocypack/velocypack-aliases.h`. This header makes the most common
VPack classes available under alternative (hopefully unambiguous) class
names with prefix `VPack`:

```cpp
using VPackBufferDumper       = arangodb::velocypack::BufferDumper;
using VPackBuilder            = arangodb::velocypack::Builder;
using VPackCharBuffer         = arangodb::velocypack::CharBuffer;
using VPackCollection         = arangodb::velocypack::Collection;
using VPackException          = arangodb::velocypack::Exception;
using VPackOptions            = arangodb::velocypack::Options;
using VPackParser             = arangodb::velocypack::Parser;
using VPackStringDumper       = arangodb::velocypack::StringDumper;
using VPackStringPrettyDumper = arangodb::velocypack::StringPrettyDumper;
using VPackSlice              = arangodb::velocypack::Slice;
using VPackValue              = arangodb::velocypack::Value;
using VPackValueType          = arangodb::velocypack::ValueType;
```
