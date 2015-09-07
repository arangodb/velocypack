Jason
=====

Version 0.3

Just Another SerializatiON

This is a binary version of JSON with the intention to use it in
ArangoDB to replace shaped JSON.

Motivation
----------

We need a binary format that

  - is compact
  - covers all of JSON plus dates, integers and binary data
  - can be used in the database kernel to access subdocuments for
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

The Jason format is an attempt to achieve all this.

Data format
-----------

Jason is (unsigned) byte oriented, values are not necessarily aligned, so
all access to larger subvalues must be properly organised to avoid
alignment assumptions.

We describe a single Jason value, which is recursive in nature, but
resides (with two exceptions, see below) in a single contiguous block of
memory. Assume that the value starts at address A, the first byte V 
indicates the type (and often the length) of the Jason value at hand:

### Value types

  - 0x00      : null
  - 0x01      : false
  - 0x02      : true
  - 0x03      : double, 8 bytes IEEE follow, little endian
  - 0x04      : short array (< 256 entries, < 65536 bytes in length)
  - 0x05      : long array (< 2^56 entries, < 2^64 bytes in length)
  - 0x06      : short object (< 256 entries, < 65536 bytes in length)
  - 0x07      : long object (< 2^56 entries, < 2^64 bytes in length)
  - 0x08      : external (only in memory): a char* and a size_t pointing to the
                actual place in memory
  - 0x09      : ID, to be specified, contains a collection ID and a
                string key, for example as a uint followed by a string,
                or as 8 bytes little endian unsigned int followed by a
                string
  - 0x0a-0x0f : reserved
  - 0x10-0x1f : UTC-date in milliseconds since the epoch, stored as uint
                as below number of bytes used is V - 0x0f
  - 0x20-0x2f : int, little endian, 2s-complement, 1-16 bytes, number is V-0x1f
  - 0x30-0x3f : uint, little endian, 1 to 16 bytes, number is V - 0x2f
  - 0x40-0xbf : UTF-8-string, using V-0x40 bytes (not Unicode-Characters!), 
                length 0 is possible, so 0x40 is the empty string,
                maximal length is 127
  - 0xc0-0xcf : long UTF-8-string, next V-0xbf bytes are length of string 
                in bytes
  - 0xd0-0xdf : binary blob, next V-0xcf bytes are length of blob in bytes
  - 0xe0-0xff : reserved

### Arrays

A short array (V=0x04) has one byte N following that contains the number of
entries in the array. Then, it has N byte pairs for the offsets of the ends
of the entries, measured from address A+2 + 2*N. The byte pairs are 
unsigned little endian values for the offsets. The first entry is always
at address A+2 + 2*N

*Example*:

`[1,2,3]` has the hex dump 

    04 03 02 00 04 00 06 00 20 01 20 02 20 03

A long array (V=0x05) is very similar, except that the number of entries
is a 7-byte unsigned integer and the offsets are 8-byte unsigned integers.
Thus, the first value is always at address A+8 + 8*N.

*Example*:

`[1,2,3]` as long-array has the hex dump

     05 03 00 00 00 00 00 00 
     02 00 00 00 00 00 00 00
     04 00 00 00 00 00 00 00
     06 00 00 00 00 00 00 00
     20 01 20 02 20 03

Note that it is not recommended to encode short arrays in the long format.


### Objects

A short object (V=0x06) has one byte N following that contains the
number of key/value pairs in the object. Then, it has N+1 byte pairs for
the offsets of the ends of the entries, measured from address A+2 + 2*(N+1).
The byte pairs are unsigned little endian values for the offsets of the
starts of the attributes, and one for the offset of the end of the last
attribute. The table of offsets is sorted so that they point to the
attributes in alphabetical order to allow for binary search. Note that
it is not necessary that the offsets are monotonically increasing! The
last offset value (number N+1) always points behind the last attribute
and is thus the largest value.

Each entry consists of two parts, the key and the value, they are
encoded as normal JSON values as above, the first is always a short or
long UTF-8 string starting with a byte 0x40-0xcf as before. The second
is any other JSON value.

There is one extension: For the key it is possible to use the values
0x00-0x2f as indexes into an outside-given table of attribute names, or
the values 0x30-0x3f to store the index in a uint as above. These are
convenient when only very few attribute names occur or some are repeated
very often. The standard way to encode such an attribute name table is
as a JSON-array as specified here.

Example, the object `{"a": 12, "b": true, "c": "xyz"}` can have the hexdump:

    06 03 03 00 00 00 07 00 0d 00
    41 62 02 
    41 61 20 0c 
    41 63 43 78 79 7a


C++-Classes to handle Jason
--------------------------

class JasonSlice;   // Represents a sub-JSON as above, read-only

class JasonBuilder; // A way to build up JSON objects in memory

class Jason;       // Helper for convenient notation

enum JasonType;



