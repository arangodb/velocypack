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
  - covers all of JSON plus dates, integers and binary data
  - can be used in a database kernel to access subdocuments for
    example for indexes, so it must be possible to access subdocuments
    (array and object members) efficiently
  - can be transferred to JSON and from JSON rapidly
  - avoids too many memory allocations
  - gives flexibility to assemble objects, such that subobjects reside
    in the database in an unchanged way
  - allows to use an external table for frequently used attribute names
  - it is quick to read off the type and length of a given object

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
to other formats like JSON itself, MsgPack and BSON. We look at file
sizes and parsing and conversion performance.


Installation of the Jason library
---------------------------------

Installation is straightforward, simply do

    ./configure
    make
    sudo make install

For a standard installation.


Running the tests and the benchmark suite
-----------------------------------------

...


Using the Jason library
-----------------------

Here are some examples of how the Jason library is used in C++ code.
For a detailed documentation of the C++ classes see [the API
documentation](API.md).

...

