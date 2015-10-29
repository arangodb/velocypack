Jason - Just Another SerializatiON
==================================

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

We have invented Jason because we need a binary format that

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

The Jason format is an attempt to achieve all this.


Specification
-------------

See [the file Jason.md](Jason.md) for a detailed description.


Performance
-----------

See [the file Performance.md](Performance.md) for a thorough comparison
to other formats like JSON itself, MessagePack and BSON. We look at file
sizes and parsing and conversion performance.


Installation of the Jason library
---------------------------------

Installation is straightforward, simply do

    ./configure
    make
    sudo make install

For a standard installation.

Note that this will download and compile the googletest suite and the
rapidjson parser, which are only used for the tests and benchmarks.


Running the tests and the benchmark suite
-----------------------------------------

To compile and run the test suite do

    make test
    ./test

To compile and run the benchmarks do

    make bench
    ./runBench.sh


Using the Jason library
-----------------------

Here are some examples of how the Jason library is used in C++ code.
For a detailed documentation of the C++ classes see [the API
documentation](API.md).

### Building up Jason objects from scratch

If you want to build up a Jason object corresponding to this JSON
object:

    {
      "b": 12,
      "a": true,
      "l": [1, 2, 3],
      "name": "Gustav"
    }

then you would use the `JasonBuilder` class as follows:

    #include <JasonBuilder.h>
    using namespace arangodb::jason;
    JasonBuilder b;
    b.add(Jason(JasonType::Object));
    b.add("b", Jason(12));
    b.add("a", Jason(true));
    b.add("l", Jason(JasonType::Array));
    b.add(Jason(1));
    b.add(Jason(2));
    b.add(Jason(3));
    b.close();
    b.add("name", Jason("Gustav"));
    b.close();

The resulting Jason object can now be found at the memory location,
where the `uint8_t*` 

    b.start()

points to and has byte length

    b.size()

One can see that adding objects and arrays essentially "opens" them such
that further additions go into the subvalue until one calls the
`close()` method. The `Jason` class is a slim wrapper class to use the
C++ type system for compact notation.

If you fancy syntactic sugar using C++ callable objects, you could also
write:

    JasonBuilder b;
    b(Jason(JasonType::Object))
     ("b", Jason(12))
     ("a", Jason(true))
     ("l", Jason(JasonType::Array))
       (Jason(1)) (Jason(2)) (Jason(3)) ()
     ("name", Jason("Gustav")) ();


### Parsing JSON to Jason in memory

You use the JSON parser as follows:

    #include <JasonParser.h>
    using namespace arangodb::jason;
    JasonParser p;
    std::string json = "{\"a\":12}";
    try {
      size_t nr = p.parse(json);
    }
    catch (std::bad_alloc const& e) {
      std::cout << "Out of memory!" << std::endl;
    }
    catch (JasonException const& e) {
      std::cout << "Parse error: " << e.what() << std::endl;
      std::cout << "Position of error: " << p.errorPos() << std::endl;
    }
    JasonBuilder b = p.steal();

The final `steal()` method is very efficient and does not copy the result.
You can access the resulting Jason object as above via `b`.


### Accessing subvalues of a Jason object

The class JasonSlice is used for this. It only needs (and stores) the 
starting position of the Jason value in memory and can derive the type
and length from that. Therefore it is very cheap to create and destroy
`JasonSlice`s.

Use as follows, if the `uint8_t*` `jason` points to a Jason value, which
holds `{ "a": 12, "b": true, "l": [1,2,3], "name": "Franz" }`, say:

    #include <JasonSlice.h>
    using namespace arangodb::jason;
    JasonSlice s(p);
    JasonType t = s.type();      // could be JasonType::Object, say
    if (s.isObject()) {
      JasonSlice ss = s.get("l");   // Now ss points to the subvalue under "l"
      JasonLength l = ss.length();
      JasonSlice ss3 = ss.at(1);
      int i = ss3.getInt();
    }
    JasonSlice sss = s.get("name");
    if (sss.isString()) {
      JasonLength len;
      std::string name = sss.getString(len);
    }


### Dumping a Jason value to JSON

The template class JasonDumper is used for this. It in turn uses the 
`JasonSlice` class. Use as follows, if `JasonSlice` `slice` points to
a valid Jason value:

    #include <JasonDump.h>
    #include <JasonSlice.h>
    JasonCharBuffer buffer;
    JasonBufferDumper dumper(buffer, JasonBufferDumper::StrategyFail);
    dumper.dump(s);
    std::string output(buffer.data(), buffer.size());



