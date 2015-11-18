{
  "targets": [
    {
      "target_name": "velocypack",
      "sources": [ "../../src/asm-functions.cpp", 
                   "../../src/AttributeTranslator.cpp",
                   "../../src/Builder.cpp",
                   "../../src/Collection.cpp",
                   "../../src/Dumper.cpp",
                   "../../src/Exception.cpp",
                   "../../src/fasthash.cpp",
                   "../../src/fpconv.cpp",
                   "../../src/HexDump.cpp",
                   "../../src/Iterator.cpp",
                   "../../src/Options.cpp",
                   "../../src/Parser.cpp",
                   "../../src/Slice.cpp",
                   "../../src/ValueType.cpp",
                   "../../src/velocypack-common.cpp",
                   "../../src/Version.cpp",
                   "velocypack_wrap.cxx" ],
      "include_dirs": [ "../../include", "../../src", "/usr/local/node-v5.0.0-linux-x64/include/node" ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "cflags": [ "-std=c++11 -DSWIG=1" ]
    }
  ]
}
