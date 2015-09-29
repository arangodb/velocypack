Jason
=====

Version 0.9999

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

  - 0x00      : none - this indicates absence of any type and value
  - 0x01      : null
  - 0x02      : false
  - 0x03      : true
  - 0x04      : double, 8 bytes IEEE follow, little endian
  - 0x05      : short array (< 256 entries, < 65536 bytes in length)
  - 0x06      : long array (< 2^64 entries, < 2^64 bytes in length)
  - 0x07      : short object (< 256 entries, < 65536 bytes in length)
  - 0x08      : long object (< 2^64 entries, < 2^64 bytes in length)
  - 0x09      : external (only in memory): a char* pointing to the actual
                place in memory, where another Jason item resides
  - 0x0a      : ID, to be specified, contains a collection ID and a
                string key, for example as a uint followed by a string,
                or as 8 bytes little endian unsigned int followed by a
                string
  - 0x0b      : the value of ArangoDB's _id attribute, it is generated
                out of the collection name, "/" and the value of the
                _key attribute when JSON is generated
  - 0x0c      : long UTF-8-string, next 8 bytes are length of string in
                bytes (not Unicode chars) as little endian unsigned
                integer, note that long strings are not zero-terminated
                and may contain zero bytes
  - 0x0d-0x0f : reserved
  - 0x10-0x17 : UTC-date in milliseconds since the epoch, stored as uint
                as below number of bytes used is V-0x0f
  - 0x18-0x1f : positive int, little endian, 1-8 bytes, number is V-0x17
  - 0x20-0x27 : negative int, absolute value is stored as uint, 1-8
                bytes, number is V-0x1f
  - 0x28-0x2f : uint, little endian, 1 to 8 bytes, number is V - 0x27
  - 0x30-0x3f : small integers -8, -7, ... 0, 1, ... 7 in twos complement, 
                that is, 0 is 0x30 and 0x37 is 7 and 0x38 is -8, etc. 
  - 0x40-0xbf : UTF-8-string, using V-0x40 bytes (not Unicode-Characters!), 
                length 0 is possible, so 0x40 is the empty string,
                maximal length is 127, note that strings here are not
                zero-terminated
  - 0xc0-0xc7 : binary blob, next V-0xbf bytes are length of blob in bytes,
                note that binary blobs are not zero-terminated
  - 0xc8-0xcf : positive long packed BCD-encoded integer, V-0xc7 bytes follow
                that encode in a little-endian way the length of the
                long int in bytes. After that, that many bytes follow,
                each byte encodes two digits in little-endian packed BCD
                Example: 12345 decimal is encoded as
                         0xc8 0x03 0x45 0x23 0x01
  - 0xd0-0xd7 : negative long packed BCD-encoded integer, V-0xcf bytes
                follow that encode in a little-endian way the length of
                the long int in bytes. After that, that many bytes
                follow, each byte encodes two digits in little-endian
                packed BCD representation.
  - 0xd8-0xff : reserved


### Arrays

Arrays look like this:

  0x05 or 0x06 for the type (short or long array)
  BYTELENGTH (one or 9 bytes)
  sub Jason values
  ...
  INDEX-TABLE (2-byte offsets (short) or 8-byte offsets (long)
  NUMBER N OF SUBVALUES (1 byte (short) or 8-bytes (long))

Arrays have a small header including their byte length, then all the
subvalues and then an index table containing offsets to the subvalues
and finally the number of subvalues. To find the index table, find the
end, then the number of subvalues and from that the base of the index
table, considering whether the array is short or long. There are two
variants for this byte length, a one byte variant with values between 2
and 0xff, or an 8 byte integer. The small values are specified by one
byte following the type byte with values 0x02 to 0xff. If this length
byte is 0x00, then the next 8 bytes are the length as little endian
unsigned integer. Thus, the first entry is either at adress A+2 or at
address A+10, depending on whether the byte at address A+1 is non-zero
or zero. The index table resides at the end of the space occupied by the
value, just before the number of subvalue information. As a special case
the empty array has A[1] set to 2 and no length information is needed.

For a small array (V=0x05), the number of subvalues is stored in a
single byte containing the number N of entries and, before that, has N
byte pairs, containing the offsets of the subvalues at indices 0, 1,
2, ... N-1, at ascending memory addresses. Recall that for N=0, the
byte length is 0x02 and there is no space used for N. The byte pairs
are unsigned little endian values for the offsets and all offsets are
measured from base A.


*Example*:

`[1,2,3]` has the hex dump 

    05 0b 31 32 33 02 00 03 00 04 00 03 

A long array (V=0x06) is very similar, except that the number of entries
and the offsets are 8-byte little endian unsigned integers.

*Example*:

`[1,2,3]` as long-array has the hex dump

     06 
     25
     31 32 33
     02 00 00 00 00 00 00 00
     03 00 00 00 00 00 00 00
     04 00 00 00 00 00 00 00
     03 00 00 00 00 00 00 00

Note that it is not recommended to encode short arrays in the long format.

Furthermore note that there is no direct correlation between the byte
length format (1 or 8 bytes) and the array size. It is in particular
valid to have a small array with an 8-byte byte length. This can be
convenient (and indeed sometimes necessary!) when building the array i
in one linear go. It is however never necessary to have a 1-byte byte
length in a long array.


### Objects

Objects look like this:

  0x07 or 0x08 for the type (short or long object)
  BYTELENGTH (one or 8 bytes)
  sub Jason values as pairs of a string and another Jason value
  ...
  INDEX-TABLE (2-byte offsets (short) or 8-byte offsets (long)
  NUMBER N OF SUBVALUES (1 byte (short) or 8-bytes (long))

Objects have a small header including their byte length, then all the
subvalues and then an index table containing offsets to the subvalues
and finally the number of subvalues. To find the index table, find the
end, then the number of subvalues and from that the base of the index
table, considering whether the array is short or long. There are two
variants for this byte length, a one byte variant with values between 2
and 0xff, or an 8 byte integer. The small values are specified by one
byte following the type byte with values 0x02 to 0xff. If this length
byte is 0x00, then the next 8 bytes are the length as little endian
unsigned integer. Thus, the first entry is either at adress A+2 or at
address A+10, depending on whether the byte at address A+1 is non-zero
or zero. The index table resides at the end of the space occupied by the
value, just before the number of subvalue information. As a special case
the empty object has A[1] set to 2 and no length information is needed.

For a small object (V=0x07), the number of subvalues is stored in a
single byte containing the number N of entries and, before that, has N
byte pairs, containing the offsets of the subvalues at indices 0, 1,
2, ... N-1, at ascending memory addresses. Recall that for N=0, the
byte length is 0x02 and there is no space used for N. The byte pairs
are unsigned little endian values for the offsets and all offsets are
measured from base A.

Each entry consists of two parts, the key and the value, they are
encoded as normal Jason values as above, the first is always a short or
long UTF-8 string starting with a byte 0x40-0xcf as before. The second
is any other Jason value.

There is one extension: For the key it is possible to use the values
0x00-0x27 as indexes into an outside-given table of attribute names, or
the values 0x28-0x2f to store the index in a uint as above. These are
convenient when only very few attribute names occur or some are repeated
very often. The standard way to encode such an attribute name table is
as a JSON-array as specified here.

Example, the object `{"a": 12, "b": true, "c": "xyz"}` can have the hexdump:

    07 
    16
    41 62 03 
    41 61 18 0c 
    41 63 43 78 79 7a
    05 00 02 00 09 00 03

Large objects are the same, only the number of entries is an 8-byte
little-endian unsigned integer and the offsets are 8-byte little-endian 
integers as well, the above object as a long object:

    07 
    2f
    41 62 03 
    41 61 18 0c 
    41 63 43 78 79 7a
    05 00 00 00 00 00 00 00
    02 00 00 00 00 00 00 00
    09 00 00 00 00 00 00 00
    03 00 00 00 00 00 00 00

Note that it is not recommended to encode short arrays in the long format.

Furthermore note that there is no direct correlation between the byte
length format (1 or 8 bytes) and the object size. It is in particular
valid to have a small object with an 8-byte byte length. This can be
convenient (and indeed sometimes necessary!) when building the object 
in one linear go. It is however never necessary to have a 1-byte byte
length in a long object.

C++-Classes to handle Jason
--------------------------

class JasonSlice;   // Represents a sub-JSON as above, read-only

class JasonBuilder; // A way to build up JSON objects in memory

class Jason;       // Helper for convenient notation

enum JasonType;



