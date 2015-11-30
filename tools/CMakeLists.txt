# -*- mode: CMAKE; -*-

# options
option(BuildBench "Build bench performance test suite" OFF)
message(STATUS "Building bench performance test suite: ${BuildBench}")

option(BuildTools "Build support programs and tools" ON)
message(STATUS "Building support programs and tools: ${BuildTools}")

# build json-to-vpack.cpp
if(BuildTools)
  add_executable(
    "json-to-vpack"
    json-to-vpack.cpp
  )

  target_link_libraries(
    "json-to-vpack"
    velocypack
  )

  add_dependencies("json-to-vpack" velocypack)
  install(TARGETS "json-to-vpack" DESTINATION bin)

  # build vpack-to-json.cpp
  add_executable(
    "vpack-to-json"
    vpack-to-json.cpp
  )

  target_link_libraries(
    "vpack-to-json"
    velocypack
  )

  add_dependencies("vpack-to-json" velocypack)
  install(TARGETS "vpack-to-json" DESTINATION bin)
endif()

# build bench.cpp
if(BuildBench)
  if(NOT IS_DIRECTORY "${PROJECT_SOURCE_DIR}/rapidjson")
    message(FATAL_ERROR "rapidjson library not found in subdirectory 'rapidjson'. Please run ./download-rapidjson.sh")
  endif()

  include_directories(${PROJECT_SOURCE_DIR}/rapidjson/include)

  add_executable(
    bench
    bench.cpp
  )

  target_link_libraries(
    bench
    velocypack
  )
  add_dependencies(bench velocypack)

  if(EnableSSE)
    set_target_properties(bench PROPERTIES COMPILE_DEFINITIONS RAPIDJSON_SSE42)
  endif()
endif()

