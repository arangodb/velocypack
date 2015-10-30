# -*- mode: CMAKE; -*-

cmake_minimum_required(VERSION 2.6)

project(jason)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")
set(CMAKE_BINARY_DIR "${CMAKE_SOURCE_DIR}/build")

message(STATUS "path: ${CMAKE_MODULE_PATH}")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE string "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

message(STATUS "Build mode: ${CMAKE_BUILD_TYPE}")

set(JASON_VERSION "0.0.1" CACHE path "Jason version")

set(JASON_DISPLAY_NAME "Jason")
set(JASON_URL_INFO_ABOUT "http://www.arangodb.org")
set(JASON_CONTACT "hackers@arangodb.org")

set(JASON_FRIENDLY_STRING "Jason - Just Another SerializatiON")

# test for SSE4.2 support
include(TargetArch)
target_architecture(ARCH)
find_package(SSE)

# compiler options
if(CMAKE_COMPILER_IS_GNUCXX)
  message(STATUS "Compiler type GNU: ${CMAKE_CXX_COMPILER}")
  set(BASE_COMPILER_OPTIONS "-std=c++11 -Wall -Wextra")
  set(CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS} ${BASE_COMPILER_OPTIONS}")
  set(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG} ${BASE_COMPILER_OPTIONS} -O0 -g -DJASON_DEBUG=1")
  set(CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL} ${BASE_COMPILER_OPTIONS} -Os")
  set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE} ${BASE_COMPILER_OPTIONS} -O3 -fomit-frame-pointer")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${BASE_COMPILER_OPTIONS} -O3 -g")

  set(CAN_USE_SSE4_2 ${SSE4_2_FOUND} CACHE BOOL "Use SSE4.2 optimizations")

elseif(CMAKE_COMPILER_IS_CLANGCXX)
  message(STATUS "Compiler type CLANG: ${CMAKE_CXX_COMPILER}")
  set(BASE_COMPILER_OPTIONS "-std=c++11 -Wall -Wextra")
  set(CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS} ${BASE_COMPILER_COMPTIONS}")
  set(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG} ${BASE_COMPILER_OPTIONS} -O0 -g -DJASON_DEBUG=1")
  set(CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL} ${BASE_COMPILER_OPTIONS} -Os")
  set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE} ${BASE_COMPILER_OPTIONS} -O3 -fomit-frame-pointer")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${BASE_COMPILER_OPTIONS} -O3 -g")
  
  set(CAN_USE_SSE4_2 ${SSE4_2_FOUND} CACHE BOOL "Use SSE4.2 optimizations")

elseif(MSVC) 
  message(STATUS "Compiler type MSVC: ${CMAKE_CXX_COMPILER}")
  set(${CMAKE_CXX_FLAGS} "${CMAKE_CXX_FLAGS} /MT")
  set(${CMAKE_CXX_FLAGS_DEBUG} "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
  
  set(CAN_USE_SSE4_2 True CACHE BOOL "Use SSE4.2 optimizations")

else()
  # unknown compiler
  message(STATUS "Compiler type UNKNOWN: ${CMAKE_CXX_COMPILER}")
  set(BASE_COMPILER_OPTIONS "-std=c++11 -Wall -Wextra")
  set(CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS} ${BASE_COMPILER_COMPTIONS}")
  set(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG} ${BASE_COMPILER_OPTIONS} -O0 -g -DJASON_DEBUG=1")
  set(CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL} ${BASE_COMPILER_OPTIONS} -Os")
  set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE} ${BASE_COMPILER_OPTIONS} -O3 -fomit-frame-pointer")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${BASE_COMPILER_OPTIONS} -O3 -g")

endif()

# options
option(BuildBench "Build bench performance test suite" OFF)
message(STATUS "Building bench performance test suite: ${BuildBench}")

option(BuildTests "Build test suite" OFF)
message(STATUS "Building test suite: ${BuildTests}")
  
if(CAN_USE_SSE4_2)
  option(EnableSSE "Build using SSE4.2 optimizations" ON)
else()
  option(EnableSSE "Build using SSE4.2 optimizations" OFF)
endif()
  
message(STATUS "Building with SSE 4.2 optimizations enabled: ${EnableSSE}")
if(EnableSSE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2")
endif()


include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src)

find_package(Threads)

# build libjason
add_library(
  jason
  STATIC
  src/Jason.cpp
  src/JasonAsm.cpp
  src/JasonBuilder.cpp
  src/JasonParser.cpp
  src/JasonSlice.cpp
  src/JasonType.cpp
  src/fpconv.cpp
)

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
    jason
  )

  if(EnableSSE)
    set_target_properties(bench PROPERTIES COMPILE_DEFINITIONS RAPIDJSON_SSE42)
  endif()
endif()

# build tests.cpp
if(BuildTests)
  add_subdirectory(googletest/googletest)

  include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

  add_executable(
    tests
    src/tests.cpp
  )

  target_link_libraries(
    tests
    gtest
    jason
    ${CMAKE_THREAD_LIBS_INIT}
  )
endif()

## Local Variables:
## mode: outline-minor
## outline-regexp: "### @brief\\|## --SECTION--\\|# -\\*- "
## End:
