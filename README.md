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

This repository contains a C++ library for building, manipulating and
serializing VPack data. It is the reference implementation for the VPack
format.


Specification
-------------

See the file [VelocyPack.md](VelocyPack.md) for a detailed description of
the VPack format.


Performance
-----------

See the file [Performance.md](Performance.md) for a thorough comparison
to other formats like JSON itself, MessagePack and BSON. We look at file
sizes as well as parsing and conversion performance.


Building the VPack library
--------------------------

The VPack library can be built on Linux, MacOS and Windows. It will likely
compile and work on other platforms for which a recent version of `cmake` and
a working C++11-enabled compiler are available.

See the file [Install.md](Install.md) for compilation and installation
instructions.


Using the VPack library
-----------------------

Please consult the file [API.md](API.md) for usage examples, and the file
[Embedding.md](Embedding.md) for embedding the library into client applications.
