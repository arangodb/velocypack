# -*- mode: CMAKE; -*-

# build libvelocypack
add_library(
  velocypack
  STATIC
  velocypack-common.cpp
  Builder.cpp
  Collection.cpp
  Exception.cpp
  HexDump.cpp
  Options.cpp
  Parser.cpp
  Slice.cpp
  ValueType.cpp
  asm-functions.cpp
  fasthash.cpp
  fpconv.cpp
)

install(TARGETS velocypack DESTINATION lib)
