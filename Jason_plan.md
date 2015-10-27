Jason
=====

Jason - Just Another SerializatiON

More crazy ideas for names:

Jextson - Json EXTended SerializatiON
ExtJSON - EXTended JSON
JSONExt - JSON EXTended
Jetson - Json ExTended Serialization    ( Jetson ist schon ein der Name 
                                          eines Development Boards von nVidia)
IndexJSON - Indexed JSON
JSONPack
PackedJSON
PackJSON
Packson = acknops = sponkca = spankoc = conkspa


## Generalities

Jason is (unsigned) byte oriented, so Jason values are simply sequences
of bytes and are completely platform independent. Values are not
necessarily aligned, so all access to larger subvalues must be properly
organised to avoid alignment assumptions.

We describe a single Jason value, which is recursive in nature, but
resides (with two exceptions, see below) in a single contiguous block of
memory. Assume that the value starts at address A, the first byte V 
indicates the type (and often the length) of the Jason value at hand:

## Value types

We first give an overview with a brief but accurate description for
reference, for arrays and objects see below for details:

  - 0x00      : none - this indicates absence of any type and value,
                this is not allowed in Jason values
  - 0x01      : null
  - 0x02      : false
  - 0x03      : true
  - 0x04      : array without index table (all subitems have the same byte length)
  - 0x05      : array with 2-byte index table entries
  - 0x06      : array with 4-byte index table entries
  - 0x07      : array with 8-byte index table entries
  - 0x08      : object with 2-byte index table entries, sorted by attribute name
  - 0x09      : object with 4-byte index table entries, sorted by attribute name
  - 0x0a      : object with 8-byte index table entries, sorted by attribute name
  - 0x0b      : object with 2-byte index table entries, not sorted by attribute name
  - 0x0c      : object with 4-byte index table entries, not sorted by attribute name
  - 0x0d      : object with 8-byte index table entries, not sorted by attribute name
  - 0x0e      : double IEEE-754, 8 bytes follow, stored as little 
                endian uint64 equivalent
  - 0x0f      : UTC-date in milliseconds since the epoch, stored as 8 byte
                signed int, little endian, 2s complement
  - 0x10      : external (only in memory): a char* pointing to the actual
                place in memory, where another Jason item resides, not
                allowed in Jason values on disk or on the network
  - 0x11      : minKey, nonsensical value that compares < than all other values
  - 0x12      : maxKey, nonsensical value that compares > than all other values
  - 0x13-0x1f : reserved
  - 0x20-0x27 : signed int, little endian, 1 to 8 bytes, number is V-0x1f, 
                2s complement
  - 0x28-0x2f : uint, little endian, 1 to 8 bytes, number is V - 0x27
  - 0x30-0x39 : small integers 0, 1, ... 9
  - 0x3a-0x3f : small negative integers -6, -5, ..., -1
  - 0x40-0xbe : UTF-8-string, using V-0x40 bytes (not Unicode-Characters!), 
                length 0 is possible, so 0x40 is the empty string,
                maximal length is 127, note that strings here are not
                zero-terminated
  - 0xbf      : long UTF-8-string, next 8 bytes are length of string in
                bytes (not Unicode chars) as little endian unsigned
                integer, note that long strings are not zero-terminated
                and may contain zero bytes
  - 0xc0-0xc7 : binary blob, next V-0xbf bytes are length of blob in bytes,
                note that binary blobs are not zero-terminated
  - 0xc8-0xcf : positive long packed BCD-encoded float, V-0xc7 bytes follow
                that encode in a little-endian way the length of the
                mantissa in bytes. Directly after that follow 4 bytes
                encoding the (power of 10) exponent, by which the mantissa
                is to be multiplied, stored as little endian 2s
                complement signed 32-bit integer. After that, as many 
                bytes follow as the length information at the beginning
                has specified,
                each byte encodes two digits in big-endian packed BCD
                Example: 12345 decimal can be encoded as
                         0xc8 0x03 0x00 0x00 0x00 0x00 0x01 0x23 0x45
                      or 0xc8 0x03 0xff 0xff 0xff 0xff 0x12 0x34 0x50
  - 0xd0-0xd7 : negative long packed BCD-encoded float, V-0xcf bytes
                follow that encode in a little-endian way the length of
                the mantissa in bytes. After that, same as positive long
                packed BCD-encoded float above.
  - 0xd8-0xef : reserved
  - 0xf0-0xff : custom types


## Null and boolean values

These three values use a single byte to store the corresponding JSON
values.


## Arrays

Arrays look like this:

  0x04 or 0x05 or 0x06 or 0x07
  BYTELENGTH (one or 9 bytes)
  sub Jason values
  ...
  optional INDEXTABLE
  NRITEMS

The INDEXTABLE consists of: 
  - 2-byte sequences (little endian unsigned) for type 0x04
  - 4-byte sequences (little endian unsigned) for type 0x05
  - 8-byte sequences (little endian unsigned) for type 0x06
  - not existent for type 0x04, then it is guaranteed that all items
    have the same byte length.

NRITEMS is 1, 9 bytes as follows: The last byte is either
  0x00 to indicate that the 8 preceding bytes are the length 
       (little endian unsigned integer)
  0x01-0xff to directly store the length


Arrays have a small header including their byte length, then all the
subvalues and an index table containing offsets to the subvalues and
finally the number of subvalues. To find the index table, find the end,
then the number of subvalues and from that the base of the index table,
considering how long its entries are. There are two variants for this
byte length, a one byte variant with values between 0x02 and 0xff, or an 8
byte integer. The small values are specified by one byte following the
type byte with values 0x02 to 0xff. If this length byte is 0x00, then
the next 8 bytes are the length as little endian unsigned integer. Thus,
the first entry is either at adress A+2 or at address A+10, depending
on whether the byte at address A+1 is non-zero or zero. The index table
resides at the end of the space occupied by the value, just before the
number of subvalues information. As a special case the empty array has
A[1] set to 2 and no length information is needed.

The number of subvalues is either stored as a single byte (last byte in
item, possible values 0x01 to 0xff) containing the number N of entries,
or, if that last byte is 0x00, in the preceding 8 bytes as a little
endian unsigned int.

The index table resides before this number of items information and
its format depends on the type byte. For type 0x07 there is no index
table but it is guaranteed that all items have the same length (which
can either be determined by looking at the first subitem or by dividing
the byte length of the subitem area by the number of subitems. For type
0x04 the entries of the index table are 2-byte sequences (little endian
unsigned int), for type 0x05, the entries are 4-byte sequences, and for
type 0x06 the entries are 8-byte sequences. All offsets are measured
from base A. Recall that for N=0, the byte length is 0x02 and there is
no space used for N.


*Example*:

`[1,2,3]` has the hex dump 

    07 06 31 32 33 03 

in the most compact representation, but the following are equally
possible:

*Examples*:

    04 0c 31 32 33 02 00 03 00 04 00 03

    05 12 31 32 33 02 00 00 00 03 00 00 00 04 00 00 00 03 

    06 1e 31 32 33
    02 00 00 00 00 00 00 00 00
    03 00 00 00 00 00 00 00 00
    04 00 00 00 00 00 00 00 00 03

A long array (V=0x06) is very similar, except that the number of entries
and the offsets are 8-byte little endian unsigned integers.

Note that it is not recommended to encode short arrays in too long a
format.

Furthermore note that there is no direct correlation between the byte
length format (1 or 8 bytes) and the array type. It is in particular
valid to have a small array with an 8-byte byte length. This can be
convenient (and indeed sometimes necessary!) when building the array 
in one linear go.


## Objects

Objects look like this:

  one of 0x08 - 0x0d, indicating structure of the index
  BYTELENGTH (one or 9 bytes)
  sub Jason values as pairs of attribute and value
  ...
  optional INDEXTABLE
  NRITEMS

The INDEXTABLE consists of: 
  - 2-byte sequences (little endian unsigned) for types 0x08 and 0x0b
  - 4-byte sequences (little endian unsigned) for types 0x09 and 0x0c
  - 8-byte sequences (little endian unsigned) for types 0x0a and 0x0d

NRITEMS is 1, 9 bytes as follows: The last byte is either
  0x00 to indicate that the 8 preceding bytes are the length 
       (little endian unsigned integer)
  0x01-0xff to directly store the length

Objects can be stored sorted or unsorted. The sorted object variants need
to store key/value pair in order, sorted by bytewise comparions of the
keys on each nesting level. Sorting has some overhead but will allow
looking up keys in logarithmic time later. For the unsorted object variants,
keys can be stored in arbitrary order, so key lookup later will require
a linear search. 

Objects have a small header including their byte length, then all the
subvalues and an index table containing offsets to the subvalues and
finally the number of subvalues. To find the index table, find the end,
then the number of subvalues and from that the base of the index table,
considering how long its entries are. There are two variants for this
byte length, a one byte variant with values between 0x02 and 0xff, or an 8
byte integer. The small values are specified by one byte following the
type byte with values 0x02 to 0xff. If this length byte is 0x00, then
the next 8 bytes are the length as little endian unsigned integer. Thus,
the first entry is either at adress A+2 or at address A+10, depending
on whether the byte at address A+1 is non-zero or zero. The index table
resides at the end of the space occupied by the value, just before the
number of subvalues information. As a special case the empty object has
A[1] set to 2 and no length information is needed.

The number of subvalues is either stored as a single byte (last byte in
item, possible values 0x01 to 0xff) containing the number N of entries,
or, if that last byte is 0x00, in the preceding 8 bytes as a little
endian unsigned int.

The index table resides before this number of items information and its
format depends on the type byte. For types 0x08 and 0x0b the entries of
the index table are 2-byte sequences (little endian unsigned int), for
types 0x09 and 0x0c, the entries are 4-byte sequences, and for types
0x0a and 0x0d the entries are 8-byte sequences. All offsets are measured
from base A. Recall that for N=0, the byte length is 0x02 and there is
no space used for N.

Each entry consists of two parts, the key and the value, they are
encoded as normal Jason values as above, the first is always a short or
long UTF-8 string starting with a byte 0x40-0xbf as before. The second
is any other Jason value.

There is one extension: For the key it is possible to use the values
0x00-0x27 as indexes into an outside-given table of attribute names, or
the values 0x28-0x2f to store the index in a uint as above. These are
convenient when only very few attribute names occur or some are repeated
very often. The standard way to encode such an attribute name table is
as a JSON-array as specified here.

Example: the object `{"a": 12, "b": true, "c": "xyz"}` can have the hexdump:

    08 
    16
    41 62 03 
    41 61 28 0c 
    41 63 43 78 79 7a
    05 00 02 00 09 00 03

The same object could have been done with an index table with longer
entries, as in this example:

    09 
    1c
    41 62 03 
    41 61 28 0c 
    41 63 43 78 79 7a
    05 00 00 00 02 00 00 00 09 00 00 00 03

Similarly with type 0x0a and 8-byte offsets. Furthermore, it could be
stored unsorted like in:

    0a 
    16
    41 62 03 
    41 61 28 0c 
    41 63 43 78 79 7a
    02 00 05 00 09 00 03

Note that it is not recommended to encode short arrays with too long
index tables.

Furthermore note that there is no direct correlation between the byte
length format (1 or 8 bytes) and the object size. It is in particular
valid to have an object with a 2-byte entry index table and an 8-byte
byte length. This can be convenient (and indeed sometimes necessary!)
when building the object in one linear go.


## Doubles

Type 0x0e indicates a double IEEE-754 value using the 8 bytes following
the type byte. To guarantee platform-independentness the details of the
byte order ar as follows. Encoding is done by using memcpy to copy the
internal double value to an uint64\_t. This 64-bit unsigned integer is
then stored as little endian 8 byte integer in the Jason value. Decoding
works in the opposite direction. This should sort out the undetermined byte
order in IEEE-754 in practice.


## Dates

Type 0x0f indicates a signed 64-int integer stored in 8 bytes little
endian twos complement notation directly after the type. The value means
a universal UTC-time measured in milliseconds since the epoch, which is
00:00 on 1 January 1970 UTC.


## External Jason values

This type is only for use within memory, not for data exchange over disk
or network. Therefore, we only need to specify that the following k
bytes are the memcpy of a char* on the current architecture. That char*
points to the actual Jason value elsewhere in memory.


## Artifical minimal and maximal keys

These values of types 0x11 and 0x12 have no meaning other than comparing
smaller or greater respectively than any other Jason value. The idea is
that these can be used in systems that define a total order on all Jason
values to specify left or right ends of infinite intervals.


## Integer types

There are different ways to specify integers. For small values -6 to 9
inclusively there are specific type bytes in the range 0x30 to 0x3f to
allow for storage in a single byte. After that there are signed and
unsigned integer types that can code in the type byte the number of
bytes used (ranges 0x20-0x27 for signed and 0x28-0x2f for unsigned).

## Strings

...

## Binary data

...

## Packed BCD long floats

...

## Custom types

Note that custom types should not be used for data exchange but
only internally in systems. The C++ library classes have pluggable
methods for them.

So far, the following user-defined types have been suggested for use
in ArangoDB:

  - 0xf0      : ID, to be specified, contains a collection ID and a
                string key, for example as a uint followed by a string,
                or as 8 bytes little endian unsigned int followed by a
                string
  - 0xf1      : the value of ArangoDB's _id attribute, it is generated
                out of the collection name, "/" and the value of the
                _key attribute when JSON is generated



