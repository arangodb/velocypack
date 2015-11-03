Jason
=====

Jason - Just Another SerializatiON


## Generalities

Jason is (unsigned) byte oriented, so Jason values are simply sequences
of bytes and are platform independent. Values are not necessarily
aligned, so all access to larger subvalues must be properly organised to
avoid alignment assumptions of the CPU.


## Value types

We describe a single Jason value, which is recursive in nature, but
resides (with two exceptions, see below) in a single contiguous block of
memory. Assume that the value starts at address A, the first byte V 
indicates the type (and often the length) of the Jason value at hand:

We first give an overview with a brief but accurate description for
reference, for arrays and objects see below for details:

  - 0x00      : none - this indicates absence of any type and value,
                this is not allowed in Jason values
  - 0x01      : empty array
  - 0x02      : array without index table (all subitems have the same 
                byte length), 1-byte byte length
  - 0x03      : array without index table (all subitems have the same 
                byte length), 2-byte byte length
  - 0x04      : array without index table (all subitems have the same 
                byte length), 4-byte byte length
  - 0x05      : array without index table (all subitems have the same 
                byte length), 8-byte byte length
  - 0x06      : array with 1-byte index table offsets, bytelen and # subvals
  - 0x07      : array with 2-byte index table offsets, bytelen and # subvals
  - 0x08      : array with 4-byte index table offsets, bytelen and # subvals
  - 0x09      : array with 8-byte index table offsets, bytelen and # subvals
  - 0x0a      : empty object
  - 0x0b      : object with 1-byte index table offsets, sorted by 
                attribute name, 1-byte bytelen and # subvals
  - 0x0c      : object with 2-byte index table offsets, sorted by 
                attribute name, 2-byte bytelen and # subvals
  - 0x0d      : object with 4-byte index table offsets, sorted by 
                attribute name, 4-byte bytelen and # subvals
  - 0x0e      : object with 8-byte index table offsets, sorted by 
                attribute name, 8-byte bytelen and # subvals
  - 0x0f      : object with 1-byte index table offsets, not sorted by 
                attribute name, 1-byte bytelen and # subvals
  - 0x10      : object with 2-byte index table offsets, not sorted by 
                attribute name, 2-byte bytelen and # subvals
  - 0x11      : object with 4-byte index table offsets, not sorted by 
                attribute name, 4-byte bytelen and # subvals
  - 0x12      : object with 8-byte index table offsets, not sorted by 
                attribute name, 8-byte bytelen and # subvals
  - 0x13-0x17 : reserved
  - 0x18      : null
  - 0x19      : false
  - 0x1a      : true
  - 0x1b      : double IEEE-754, 8 bytes follow, stored as little 
                endian uint64 equivalent
  - 0x1c      : UTC-date in milliseconds since the epoch, stored as 8 byte
                signed int, little endian, two's complement
  - 0x1d      : external (only in memory): a char* pointing to the actual
                place in memory, where another Jason item resides, not
                allowed in Jason values on disk or on the network
  - 0x1e      : minKey, nonsensical value that compares < than all other values
  - 0x1f      : maxKey, nonsensical value that compares > than all other values
  - 0x20-0x27 : signed int, little endian, 1 to 8 bytes, number is V - 0x1f, 
                two's complement
  - 0x28-0x2f : uint, little endian, 1 to 8 bytes, number is V - 0x27
  - 0x30-0x39 : small integers 0, 1, ... 9
  - 0x3a-0x3f : small negative integers -6, -5, ..., -1
  - 0x40-0xbe : UTF-8-string, using V - 0x40 bytes (not Unicode characters!), 
                length 0 is possible, so 0x40 is the empty string,
                maximal length is 126, note that strings here are not
                zero-terminated
  - 0xbf      : long UTF-8-string, next 8 bytes are length of string in
                bytes (not Unicode characters) as little endian unsigned
                integer, note that long strings are not zero-terminated
                and may contain zero bytes
  - 0xc0-0xc7 : binary blob, next V - 0xbf bytes are the length of blob in 
                bytes, note that binary blobs are not zero-terminated
  - 0xc8-0xcf : positive long packed BCD-encoded float, V - 0xc7 bytes follow
                that encode in a little endian way the length of the
                mantissa in bytes. Directly after that follow 4 bytes
                encoding the (power of 10) exponent, by which the mantissa
                is to be multiplied, stored as little endian two's
                complement signed 32-bit integer. After that, as many 
                bytes follow as the length information at the beginning
                has specified, each byte encodes two digits in
                big-endian packed BCD
                Example: 12345 decimal can be encoded as
                         0xc8 0x03 0x00 0x00 0x00 0x00 0x01 0x23 0x45
                      or 0xc8 0x03 0xff 0xff 0xff 0xff 0x12 0x34 0x50
  - 0xd0-0xd7 : negative long packed BCD-encoded float, V - 0xcf bytes
                follow that encode in a little endian way the length of
                the mantissa in bytes. After that, same as positive long
                packed BCD-encoded float above.
  - 0xd8-0xef : reserved
  - 0xf0-0xff : custom types


## Arrays

Empty arrays are simply a single byte 0x01.

Nonempty arrays look like this:

  one of 0x02 to 0x09
  BYTELENGTH
  sub Jason values
  optional INDEXTABLE
  optional NRITEMS

Numbers (for byte length, number of subvalues and offsets in the
INDEXTABLE) are little endian unsigned integers, using 1 byte for
types 0x02 and 0x06, 2 bytes for types 0x03 and 0x07, 4 bytes for types
0x04 and 0x08, and 8 bytes for types 0x05 and 0x09.

The INDEXTABLE consists of: 
  - not existent for types 0x02-0x05, then it is guaranteed that all 
    items have the same byte length, one of these types is always 
    taken for arrays with 1 element.
  - for types 0x06-0x09 an array of offsets (unaligned, in the number
    format described above) earlier offsets reside at lower addresses.
    Offsets are measured from the start of the Jason value.

NRITEMS is a single number as described above.


Nonempty arrays have a small header including their byte length, then
all the subvalues and an index table containing offsets to the subvalues
and finally the number of subvalues. To find the index table, find the
end, then the number of subvalues and from that the base of the index
table, considering how wide its entries are.

For types 0x02 to 0x05 there is no offset table and no number of items.
The first item begins at address A+2, A+3, A+5 or respectively A+9 and
one can determine the number by finding the byte length of the first
subvalue and dividing the amount of available space by it.

For types 0x06 to 0x09 the offset table describes where the subvalues
reside. It is not necessary for the subvalues to start immediately after
the byte length field. For performance reasons when building the value,
it could be desirable to reserve 8 bytes for the byte length and not
fill the gap, even though it turns out later that offsets and thus the
byte length only uses 2 bytes, say. The number of subvalues is stored
after the offset table as another number in the format described above.

All offsets are measured from base A.


*Example*:

`[1,2,3]` has the hex dump 

    02 05 31 32 33

in the most compact representation, but the following are equally
possible, though not necessarily advised to use:

*Examples*:

    03 06 00 31 32 33

    04 08 00 00 00 31 32 33

    05 0c 00 00 00 00 00 00 00 31 32 33

    06 09 31 32 33 02 03 04 03

    07 0e 00 31 32 33 03 00 05 00 07 00 03 00

    08 18 00 00 00 31 32 33 05 00 00 00 06 00 00 00 07 00 00 00 03 00 00 00

    09 
    2c 00 00 00 00 00 00 00 00
    31 32 33
    09 00 00 00 00 00 00 00 00
    0a 00 00 00 00 00 00 00 00
    0b 00 00 00 00 00 00 00 00 
    03 00 00 00 00 00 00 00 00

Note that it is not recommended to encode short arrays in too long a
format.


## Objects

Empty objects are simply a single byte 0x0a.

Nonempty objects look like this:

  one of 0x0b - 0x12
  BYTELENGTH
  sub Jason values as pairs of attribute and value
  optional INDEXTABLE
  NRITEMS

Numbers (for byte length, number of subvalues and offsets in the
INDEXTABLE) are little endian unsigned integers, using 1 byte for
types 0x0b and 0x0f, 2 bytes for types 0x0c and 0x10, 4 bytes for types
0x0d and 0x11, and 8 bytes for types 0x0e and 0x12.

The INDEXTABLE consists of: 
  - an array of offsets (unaligned, in the number format described
    above) earlier offsets reside at lower addresses.
    Offsets are measured from the beginning of the Jason value.

NRITEMS is a single number as described above.

Nonempty objects have a small header including their byte length, then all the
subvalues and an index table containing offsets to the subvalues and
finally the number of subvalues. To find the index table, find the end,
then the number of subvalues and from that the base of the index table,
considering how wide its entries are. 

For all types the offset table describes where the subvalues reside.
It is not necessary for the subvalues to start immediately after the
byte length field. For performance reasons when building the value, it
could be desirable to reserve 8 bytes for the byte length and not fill
the gap, even though it turns out later that offsets and thus the byte
length only uses 2 bytes, say. The number of subvalues is stored after
the offset table as another number in the format described above.

There are two special cases: the empty object is simply stored as the
single byte 0x0a and objects with exactly one element have no offset
table at all.

All offsets are measured from base A.

Each entry consists of two parts, the key and the value, they are
encoded as normal Jason values as above, the first is always a short or
long UTF-8 string starting with a byte 0x40-0xbf as described below. The
second is any other Jason value.

There is one extension: For the key it is possible to use the values
0x00-0x27 as indexes into an outside-given table of attribute names, or
the values 0x28-0x2f to store the index in a uint as above. These are
convenient when only very few attribute names occur or some are repeated
very often. The standard way to encode such an attribute name table is
as a Jason array of strings as specified here.

Objects can be stored sorted or unsorted. The sorted object variants
need to store key/value pairs in order, sorted by bytewise comparions
of the keys on each nesting level. Sorting has some overhead but will
allow looking up keys in logarithmic time later. For the unsorted object
variants, keys can be stored in arbitrary order, so key lookup later
will require a linear search. Note that only the index table needs to
be sorted, it is not required that the offsets in these tables are
increasing. Since the index table resides after the actual subvalues,
one can build up a complex Jason value by writing linearly.

Example: the object `{"a": 12, "b": true, "c": "xyz"}` can have the hexdump:

    0b 
    13
    41 62 1a 
    41 61 28 0c 
    41 63 43 78 79 7a
    05 02 09 03

The same object could have been done with an index table with longer
entries, as in this example:

    0d 
    22 00 00 00
    41 62 03 
    41 61 28 0c 
    41 63 43 78 79 7a
    08 00 00 00 05 00 00 00 0c 00 00 00 03 00 00 00

Similarly with type 0x0c and 2-byte offsets, byte length and number of
subvalues, or with type 0x09 and 8-byte numbers. Furthermore, it could
be stored unsorted like in:

    0f 
    13
    41 62 03 
    41 61 28 0c 
    41 63 43 78 79 7a
    02 05 09 03

Note that it is not recommended to encode short arrays with too long
index tables.


## Doubles

Type 0x1b indicates a double IEEE-754 value using the 8 bytes following
the type byte. To guarantee platform-independentness the details of the
byte order are as follows. Encoding is done by using memcpy to copy the
internal double value to an uint64\_t. This 64-bit unsigned integer is
then stored as little endian 8 byte integer in the Jason value. Decoding
works in the opposite direction. This should sort out the undetermined
byte order in IEEE-754 in practice.


## Dates

Type 0x1c indicates a signed 64-int integer stored in 8 bytes little
endian two's complement notation directly after the type. The value means
a universal UTC-time measured in milliseconds since the epoch, which is
00:00 on 1 January 1970 UTC.


## External Jason values

This type is only for use within memory, not for data exchange over disk
or network. Therefore, we only need to specify that the following k
bytes are the memcpy of a char* on the current architecture. That char*
points to the actual Jason value elsewhere in memory.


## Artifical minimal and maximal keys

These values of types 0x1e and 0x1f have no meaning other than comparing
smaller or greater respectively than any other Jason value. The idea is
that these can be used in systems that define a total order on all Jason
values to specify left or right ends of infinite intervals.


## Integer types

There are different ways to specify integers. For small values -6 to 9
inclusively there are specific type bytes in the range 0x30 to 0x3f to
allow for storage in a single byte. After that there are signed and
unsigned integer types that can code in the type byte the number of
bytes used (ranges 0x20-0x27 for signed and 0x28-0x2f for unsigned).


## Null and boolean values

These three values use a single byte to store the corresponding JSON
values.


## Strings

Strings are stored as UTF-8 encoded byte sequences. There are two
variants, a short one and a long one. In the short one, the byte length
(not the number of UTF-8 characters) is directly encoded in the type,
and this works up to and including byte length 126. Types 0x40 to 0xbe
are used for this and the byte length is V - 0x3f, if V is the type
byte. For strings longer than 126 bytes, the type byte is 0xbf and the
byte length of the string is stored in the first 8 bytes after the type
byte, using a little endian unsigned integer representation. The actual
string follows after these 8 bytes. There is no terminating zero byte in
either case and the string may contain zero bytes.


## Binary data

The type bytes 0xc0 to 0xc7 allow to store arbitrary binary byte
sequences as a Jason value. The format is as follows: If V is the type
byte, then V - 0xbf bytes follow it to make a little endian unsigned
integer representing the length of the binary data, which directly
follows these length bytes. No alignment is guaranteed. The content is
entirely up to the user.


## Packed BCD long floats

These types are used to represent arbitrary precision decimal numbers.
There are different types for positive and negative numbers. The overall
format of these values is:

    one of 0xc8 - 0xcf (positive) or of 0xd0 - 0xd7 (negative)
    LENGTH OF MANTISSA in bytes
    EXPONENT (as 4-byte little endian signed two's complement integer)
    MANTISSA (as packed BCD-encoded integer, big-endian)

The type byte describes the sign of the number as well as the number of
bytes used to specify the byte length of the mantissa. As usual, if V is
the type byte, then V - 0xc7 (in the positive case) or V - 0xcf (in the
negative case) bytes are used for the length of the mantissa, stored as
little endian unsigned integer directly after the byte length. After
this follow exactly 4 bytes (little endian signed two's complement 
integer) to specify the exponent. After the exponent, the actual
mantissa bytes follow.

Packed BCD is used, so that each byte stores exactly 2 decimal digits as
in 0x34 for the decimal digits 34. Therefore, the mantissa always has an
even number of decimal digits. Note that the mantissa is stored in big
endian form, to make parsing and dumping efficient. This leads to the
"unholy nibble problem": When a JSON parser sees the beginning of a
longish number, it does not know whether an even or odd number of digits
follow. However, for efficiency reasons it wants to start writing bytes
to the output as it reads the input. This is, where the exponent comes
to the rescue, which is illustrated by the following example:

    12345 decimal can be encoded as:

    0xc8 0x03 0x00 0x00 0x00 0x00 0x01 0x23 0x45
    0xc8 0x03 0xff 0xff 0xff 0xff 0x12 0x34 0x50

The former encoding puts a leading 0 in the first byte and uses exponent
0, the latter encoding directly starts putting two decimal digits in one
byte and then in the end has to "erase" the trailing 0 by using exponent
-1, encoded by the 4 byte sequence 0xff 0xff 0xff 0xff.

There for the unholy nibble problem is solved and parsing (and indeed
dumping) can be efficient.


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



