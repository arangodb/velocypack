VelocyPack (VPack) - a fast and compact format for serialization and storage
============================================================================

TravisCI: [![Build Status](https://secure.travis-ci.org/arangodb/velocypack.png?branch=master)](http://travis-ci.org/arangodb/velocypack)   AppVeyor: [![Build status](https://ci.appveyor.com/api/projects/status/pkbl4t7vey88bqud?svg=true)](https://ci.appveyor.com/project/jsteemann/velocypack)

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

  - is self-contained, schemaless and platform independent
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
    from its beginning

All this gives us the possibility to use *the same byte sequence of
data* for **transport**, **storage** and (read-only) **work**.

The other popular formats we looked at have all some deficiency with
respect to the above list. To name but a few:

  - JSON itself lacks some data types (dates and binary data) and does
    not provide quick subvalue access without parsing
  - XML is not compact and is not good with binary data, it also lacks
    quick subvalue access
  - BSON is relatively compact, gets quite a lot right with respect to
    data types, but is seriously lacking w.r.t. subvalue access
  - Apache Thrift and Google's Protocol Buffers are not schemaless and 
    self-contained and the transport format is a serialization that is
    not good for rapid subvalue access
  - MessagePack is probably the closest to our shopping list, it is
    quite compact, has decent data types but again no quick subvalue
    access
  - Our own shaped JSON (used in ArangoDB as internal storage format)
    has very quick subvalue access, but the shape data is kept outside
    the actual data, so the data markers are not self-contained.
    Furthermore, we have run into scalability issues on multi-core
    because of the shared data structures for the shapes.

Any new data format must be backed by good C++ classes to allow

  - easy and fast parsing from JSON
  - easy and convenient buildup without too many memory allocations
  - fast access of subobjects (arrays and objects)
  - flexible memory management
  - fast dumping to JSON

The VelocyPack format is an attempt to achieve all this and our first
experiments and usage attempts are very encouraging..

This repository contains a C++ library for building, manipulating and
serializing VPack data. It is the *reference implementation for the VPack
format*.


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
compile and work on other platforms for which a recent version of *cmake* and
a working C++11-enabled compiler are available.

See the file [Install.md](Install.md) for compilation and installation
instructions.


Using the VPack library
-----------------------

Please consult the file [examples/API.md](examples/API.md) for usage examples, 
and the file [examples/Embedding.md](examples/Embedding.md) for information
about how to embed the library into client applications.

