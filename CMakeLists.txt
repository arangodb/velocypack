# -*- mode: CMAKE; -*-

cmake_minimum_required(VERSION 2.6)

project(velocypack)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")
set(CMAKE_BINARY_DIR "${CMAKE_SOURCE_DIR}/build")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE string "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

message(STATUS "Build mode: ${CMAKE_BUILD_TYPE}")

set(VELOCYPACK_VERSION "0.0.1" CACHE path "VelocyPack version")

set(VELOCYPACK_DISPLAY_NAME "Velocypack")
set(VELOCYPACK_URL_INFO_ABOUT "https://github.com/arangodb/velocypack")
set(VELOCYPACK_CONTACT "hackers@arangodb.org")

set(VELOCYPACK_FRIENDLY_STRING "VelocyPack - A fast & compact serialization format")

# test for SSE4.2 support
include(TargetArch)
target_architecture(ARCH)
find_package(SSE)

# compiler options
if(CMAKE_COMPILER_IS_GNUCXX)
  message(STATUS "Compiler type GNU: ${CMAKE_CXX_COMPILER}")
  set(BASE_COMPILER_OPTIONS "-std=c++11 -Wall -Wextra")
  set(CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS} ${BASE_COMPILER_OPTIONS}")
  set(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG} ${BASE_COMPILER_OPTIONS} -O0 -g -DVELOCYPACK_DEBUG=1")
  set(CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL} ${BASE_COMPILER_OPTIONS} -Os")
  set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE} ${BASE_COMPILER_OPTIONS} -O3 -fomit-frame-pointer")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${BASE_COMPILER_OPTIONS} -O3 -g")

  set(CAN_USE_SSE4_2 ${SSE4_2_FOUND} CACHE BOOL "Use SSE4.2 optimizations")

elseif(CMAKE_COMPILER_IS_CLANGCXX)
  message(STATUS "Compiler type CLANG: ${CMAKE_CXX_COMPILER}")
  set(BASE_COMPILER_OPTIONS "-std=c++11 -Wall -Wextra")
  set(CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS} ${BASE_COMPILER_COMPTIONS}")
  set(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG} ${BASE_COMPILER_OPTIONS} -O0 -g -DVELOCYPACK_DEBUG=1")
  set(CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL} ${BASE_COMPILER_OPTIONS} -Os")
  set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE} ${BASE_COMPILER_OPTIONS} -O3 -fomit-frame-pointer")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${BASE_COMPILER_OPTIONS} -O3 -g")
  
  set(CAN_USE_SSE4_2 ${SSE4_2_FOUND} CACHE BOOL "Use SSE4.2 optimizations")

elseif(MSVC) 
  message(STATUS "Compiler type MSVC: ${CMAKE_CXX_COMPILER}")

  set(DEBUG_MODE False)
  add_definitions("-D_CRT_SECURE_NO_WARNINGS=1")

  foreach (flag_var
           CMAKE_CXX_FLAGS
           CMAKE_CXX_FLAGS_DEBUG
           CMAKE_CXX_FLAGS_RELEASE
           CMAKE_CXX_FLAGS_MINSIZEREL
           CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if (${flag_var} MATCHES "/MD")
      string(REGEX REPLACE "/MDd?" "" ${flag_var} "${${flag_var}}")
    endif()

    if (flag_var MATCHES "DEBUG")
      set(${flag_var} "${${flag_var}} /MTd")
      set(DEBUG_MODE True)
    else ()
      set(${flag_var} "${${flag_var}} /MT")
    endif ()
  endforeach()

  # https://msdn.microsoft.com/en-us/library/aa267384%28VS.60%29.aspx
  if(DEBUG_MODE)
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE /LTCG /ignore:4099 /NODEFAULTLIB:libc.lib /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:libcd.lib /NODEFAULTLIB:msvcrtd.lib")
  else()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE /LTCG /ignore:4099 /NODEFAULTLIB:libc.lib /NODEFAULTLIB:libcmtd.lib /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:libcd.lib /NODEFAULTLIB:msvcrtd.lib")
  endif()

  set(CAN_USE_SSE4_2 True CACHE BOOL "Use SSE4.2 optimizations")
else()
  # unknown compiler
  message(STATUS "Compiler type UNKNOWN: ${CMAKE_CXX_COMPILER}")
  set(BASE_COMPILER_OPTIONS "-std=c++11 -Wall -Wextra")
  set(CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS} ${BASE_COMPILER_COMPTIONS}")
  set(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG} ${BASE_COMPILER_OPTIONS} -O0 -g -DVELOCYPACK_DEBUG=1")
  set(CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL} ${BASE_COMPILER_OPTIONS} -Os")
  set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE} ${BASE_COMPILER_OPTIONS} -O3 -fomit-frame-pointer")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${BASE_COMPILER_OPTIONS} -O3 -g")

endif()

# options
option(BuildBench "Build bench performance test suite" OFF)
message(STATUS "Building bench performance test suite: ${BuildBench}")

if(CAN_USE_SSE4_2)
  option(EnableSSE "Build using SSE4.2 optimizations" ON)
else()
  option(EnableSSE "Build using SSE4.2 optimizations" OFF)
endif()
  
message(STATUS "Building with SSE 4.2 optimizations enabled: ${EnableSSE}")
if(EnableSSE AND NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2")
endif()


include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src)

find_package(Threads)

# build libvelocypack
add_library(
  velocypack
  STATIC
  src/velocypack-common.cpp
  src/Builder.cpp
  src/Collection.cpp
  src/Exception.cpp
  src/Parser.cpp
  src/Slice.cpp
  src/ValueType.cpp
  src/asm-functions.cpp
  src/fpconv.cpp
)

install(TARGETS velocypack DESTINATION lib)
install(FILES include/velocypack/velocypack-aliases.h DESTINATION include)
install(FILES include/velocypack/velocypack-common.h DESTINATION include)
install(FILES include/velocypack/Buffer.h DESTINATION include)
install(FILES include/velocypack/Builder.h DESTINATION include)
install(FILES include/velocypack/Collection.h DESTINATION include)
install(FILES include/velocypack/Dump.h DESTINATION include)
install(FILES include/velocypack/Dumper.h DESTINATION include)
install(FILES include/velocypack/Exception.h DESTINATION include)
install(FILES include/velocypack/Iterator.h DESTINATION include)
install(FILES include/velocypack/Options.h DESTINATION include)
install(FILES include/velocypack/Parser.h DESTINATION include)
install(FILES include/velocypack/Slice.h DESTINATION include)
install(FILES include/velocypack/Value.h DESTINATION include)
install(FILES include/velocypack/ValueType.h DESTINATION include)

# build json-to-vpack.cpp
add_executable(
  "json-to-vpack"
  src/json-to-vpack.cpp
)

target_link_libraries(
  "json-to-vpack"
  velocypack
)

# build vpack-to-json.cpp
add_executable(
  "vpack-to-json"
  src/vpack-to-json.cpp
)

target_link_libraries(
  "vpack-to-json"
  velocypack
)

add_subdirectory(examples)

# build bench.cpp
if(BuildBench)
  if(NOT IS_DIRECTORY "${PROJECT_SOURCE_DIR}/rapidjson")
    message(FATAL_ERROR "rapidjson library not found in subdirectory 'rapidjson'. Please run ./download-rapidjson.sh")
  endif()

  include_directories(${PROJECT_SOURCE_DIR}/rapidjson/include)

  add_executable(
    bench
    src/bench.cpp
  )

  target_link_libraries(
    bench
    velocypack
  )

  if(EnableSSE)
    set_target_properties(bench PROPERTIES COMPILE_DEFINITIONS RAPIDJSON_SSE42)
  endif()
endif()

# build tests
add_subdirectory(tests)

