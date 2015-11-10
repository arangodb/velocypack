Performance
===========

to be written


Data sizes
==========

The directory *tests/jsonSample* contains several example JSON files
which can be converted to VPack values. The following sections show
how much can be gained by converting these JSON data into VPack values.
The VPack sizes are compared against other popular schema-free serialization
formats.

### tests/jsonSample/small.json

The [original JSON file](tests/jsonSample/small.json) has a size of 82 
bytes and contains indentation and newline whitespace. Stripping all 
the whitespace from the JSON file will result in a JSON filesize of 58 bytes. 

When converting the JSON into a VPack value, the VPack value will only
need 35 bytes. MessagePack is also at 35 bytes, while BSON will need
79 bytes to store the same value. 

### tests/jsonSample/commits.json

The [original JSON file](tests/jsonSample/commits.json) has a size of 
25,216 bytes and does not contain any whitespace. When converting it to 
VPack, it will need 22,935 bytes of storage. MessagePack will use 23,091, 
and BSON will need 30,907 bytes.

### tests/jsonSample/pass1.json

The [original JSON file](tests/jsonSample/pass1.json) is 1,441 bytes long 
and contains whitespace. Without whitespace, the JSON is 961 bytes long. 
Storing the data as VPack value will need 916 bytes. Storing it in 
MessagePack will need 844 bytes, and storing it in BSON will use 1,181 bytes.

### tests/jsonSample/pass2.json

The [original JSON file](tests/jsonSample/pass2.json) is 52 bytes long and 
contains no whitespace. The same values is 51 bytes long in VPack, 32 bytes
in MessagePack, and 169 bytes in BSON.

### tests/jsonSample/pass3.json

The [input JSON file](tests/jsonSample/pass3.json) is 148 bytes long 
including whitespace. It is 115 bytes long without whitespace. Storing 
the data as VPack value will require 110 bytes. Using MessagePack will 
require 124, and using BSON will require 123 bytes.

### tests/jsonSample/random1.json

The [input JSON file](tests/jsonSample/random1.json) is 9,672 bytes long 
including whitespace. It is 7,577 bytes long without whitespace. Storing 
the data as VPack value will require 7,310 bytes. Using MessagePack will 
require 7,007, and using BSON will require 8,209 bytes.

### tests/jsonSample/random2.json

The [JSON file](tests/jsonSample/random2.json) is 8,239 bytes long 
including whitespace. It is 6,443 bytes long without whitespace. Storing 
the data as VPack value will require 6,222 bytes. Using MessagePack will 
require 5,960, and using BSON will require 6,992 bytes.

### tests/jsonSample/random3.json

The [JSON file](tests/jsonSample/random3.json) is 72,953 bytes long 
including whitespace. It is 57,112 bytes long without whitespace. Storing 
the data as VPack value will require 55,066 bytes. Using MessagePack will 
require 52,780, and using BSON will require 61,903 bytes.

### tests/jsonSample/sample.json

The [original JSON file](tests/jsonSample/sample.json) is 687,491 bytes long. 
Without whitespace, it is 168,089 bytes long. Storing the data as VPack value
will require 162,113 bytes. Using MessagePack requires 150,697 and using BSON
will require 155,462 bytes.

### Overview

The following table contains a size summary for the beforementioned files: 

```
test case        original JSON   net JSON      VPack    MsgPack       BSON 
-------------------------------------------------------------------------- 
small.json                  82         58         35         35         79
commits.json            25,216     25,216     22,935     23,091     30,907
pass1.json               1,441        961        916        844      1,181
pass2.json                  52         52         51         32        169
pass3.json                 148        115        110        124        123
random1.json             9,672      7,577      7,310      7,007      8,209
random2.json             8,239      6,443      6,222      5,960      6,992
random3.json            72,953     57,112     55,066     52,780     61,903
sample.json            687,491    168,089    162,113    150,697    155,462
```
