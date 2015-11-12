Performance
===========

to be written


Data size comparison
====================

The directory *tests/jsonSample* contains several example JSON files
which can be converted to VPack values. The following sections show
how much can be gained by converting these JSON data into VPack values.

The resulting VPack sizes are compared against some other popular schema-free 
serialization formats such as [MessagePack / MsgPack](http://msgpack.org/)
and [BSON](http://bsonspec.org/).

The following table contains the size comparisons for the beforementioned 
files and formats. All values in the table are byte sizes. Smaller values are
better. 

The columns in the table have the following meanings:

* *original JSON*: size of the original JSON data (potentially including whitespace)
* *net JSON*: size of the remaining JSON data after stripping whitespace
* *BSON*: size after converting JSON input to BSON
* *MsgPack*: size after converting JSON input to MsgPack
* *VPack*: size after converting JSON input to VPack
* *VPack-c*: size after converting JSON input to VPack, using *compact* option in VPack

```
test case        original JSON    net JSON     MsgPack        BSON       VPack     VPack-c
------------------------------------------------------------------------------------------ 
api-docs.json        1,205,964   1,054,928   1,003,337   1,012,938   1,004,012     994,160
commits.json            25,216      25,216      23,091      30,907      22,935      20,789 
countries.json       1,134,029   1,134,029   1,161,461   1,367,765   1,061,291     956,786 
directory-tree.json    297,695     297,695     306,534     368,518     276,862     244,716 
doubles.json         1,187,062   1,187,062     899,981   1,488,895   1,299,984     899,982 
doubles-small.json     158,706     108,705      89,995     138,895      89,998     130,001
file-list.json         151,317     151,316     139,979     195,268     150,177     133,536
object.json            157,781     157,781     118,519     178,895     158,633     118,630
pass1.json               1,441         961         844       1,181         916         804
pass2.json                  52          52          32         169          51          51
pass3.json                 148         115         124         123         110         108
random1.json             9,672       7,577       7,007       8,209       7,310       6,836
random2.json             8,239       6,443       5,960       6,992       6,222       5,815
random3.json            72,953      57,112      52,780      61,903      55,066      51,515
sample.json            687,491     168,089     150,697     155,462     162,113     153,187
small.json                  82          58          35          79          35          30
```

