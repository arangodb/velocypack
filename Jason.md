Jason
=====

Version 0.5

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
  - 0x08      : external (only in memory): a char* pointing to the actual
                place in memory, where another Jason item resides
  - 0x09      : ID, to be specified, contains a collection ID and a
                string key, for example as a uint followed by a string,
                or as 8 bytes little endian unsigned int followed by a
                string
  - 0x0a      : the value of ArangoDB's _id attribute, it is generated
                out of the collection name, "/" and the value of the
                _key attribute when JSON is generated
  - 0x0b-0x0f : reserved
  - 0x10-0x17 : UTC-date in milliseconds since the epoch, stored as uint
                as below number of bytes used is V-0x0f
  - 0x18-0x1f : reserved
  - 0x20-0x27 : positive int, little endian, 1-8 bytes, number is V-0x1f
  - 0x28-0x2f : negative int, absolute value is stored as uint, 1-8
                bytes, number is V-0x27
  - 0x30-0x37 : uint, little endian, 1 to 8 bytes, number is V - 0x2f
  - 0x38-0x3f : reserved
  - 0x40-0xbf : UTF-8-string, using V-0x40 bytes (not Unicode-Characters!), 
                length 0 is possible, so 0x40 is the empty string,
                maximal length is 127, note that strings here are not
                zero-terminated
  - 0xc0-0xc7 : long UTF-8-string, next V-0xbf bytes are length of string 
                in bytes, note that long strings here are not
                zero-terminated
  - 0xc8-0xcf : reserved
  - 0xd0-0xd7 : binary blob, next V-0xcf bytes are length of blob in bytes,
                note that binary blobs are not zero-terminated
  - 0xd8-0xdf : reserved
  - 0xe0-0xe7 : positive long packed BCD-encoded integer, V-0xdf bytes follow
                that encode in a little-endian way the length of the
                long int in bytes. After that, that many bytes follow,
                each byte encodes two digits in little-endian packed BCD
                Example: 12345 decimal is encoded as
                         0xe0 0x03 0x45 0x23 0x01
  - 0xe8-0xef : negative long packed BCD-encoded integer, V-0xe7 bytes
                follow that encode in a little-endian way the length of
                the long int in bytes. After that, that many bytes
                follow, each byte encodes two digits in little-endian
                packed BCD representation.
  - 0xf0-0xfe : reserved
  - 0xff      : none - this indicates absence of any type and value

### Arrays

A short array (V=0x04) has one byte N following that contains the number
of entries in the array. Then follows a byte pair for the offset of the
end of the last item, that is, the position where the next Jason item
begins, measured from address A. After that, it has N-1 byte pairs for
the offsets of the starts of the entries with index 1 to N-1, again
measured from address A. The byte pairs are unsigned little endian
values for the offsets. The first entry is always at address A+4 +
2*(N-1). If N=0, then there is only one byte pair after the length byte
containing hex bytes 04 00 to indicate a total length of 4 bytes.

*Example*:

`[1,2,3]` has the hex dump 

    04 03 0e 00 0a 00 0c 00 20 01 20 02 20 03

A long array (V=0x05) is very similar, except that the number of entries
is a 7-byte unsigned integer and the offsets are 8-byte unsigned integers.
Thus, the first value is always at address A+16 + 8*(N-1).

*Example*:

`[1,2,3]` as long-array has the hex dump

     05 03 00 00 00 00 00 00 
     26 00 00 00 00 00 00 00
     22 00 00 00 00 00 00 00
     24 00 00 00 00 00 00 00
     20 01 20 02 20 03

Note that it is not recommended to encode short arrays in the long format.


### Objects

A short object (V=0x06) has one byte N following that contains the
number of key/value pairs in the object. Then, it has 1 byte pair for
the offset of the next Jason item, that is, the end of the current one.
Then follow N byte pairs for the offsets of the starts of the entries,
measured from address A. The byte pairs are unsigned little endian
values for the offsets of the starts of the attributes. The table of
offsets is sorted so that they point to the attributes in alphabetical
order to allow for binary search. Note that it is not necessary that the
offsets are monotonically increasing!

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

    06 03 17 00 0d 00 0a 00 11 00
    41 62 02 
    41 61 20 0c 
    41 63 43 78 79 7a

Large objects are the same, only the number of entries is a 7-byte
little-endian integer and the offsets are 8-byte little-endian integers.


C++-Classes to handle Jason
--------------------------

class JasonSlice;   // Represents a sub-JSON as above, read-only

class JasonBuilder; // A way to build up JSON objects in memory

class Jason;       // Helper for convenient notation

enum JasonType;



