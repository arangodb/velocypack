# -*- mode: CMAKE; -*-

cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH "cmake")

project(jason)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE string "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

include(TargetArch)
target_architecture(ARCH)
find_package(SSE)

set(JASON_VERSION "0.0.1" CACHE path "Jason version")

set(JASON_DISPLAY_NAME "Jason")
set(JASON_URL_INFO_ABOUT "http://www.arangodb.org")
set(JASON_CONTACT "hackers@arangodb.org")

set(JASON_FRIENDLY_STRING "Jason - Just Another SerializatiON")

# compiler options
if(CMAKE_COMPILER_IS_GNUCXX)
  set(BASE_COMPILER_OPTIONS "-std=c++11 -Wall -Wextra")
  set(CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS} ${BASE_COMPILER_COMPTIONS} -O3")
  set(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG} ${BASE_COMPILER_OPTIONS} -O -g -DJASON_DEBUG=1")
  set(CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL} ${BASE_COMPILER_OPTIONS} -Os")
  set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE} ${BASE_COMPILER_OPTIONS} -O3")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${BASE_COMPILER_OPTIONS} -O3 -g")

  set(USE_SSE4_2 ${SSE4_2_FOUND} CACHE BOOL "Use SSE4.2 optimizations")
  if(USE_SSE4_2)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2 -DRAPIDJSON_SSE42")
  endif()

elseif(CMAKE_COMPILER_IS_CLANGXX)
  set(BASE_COMPILER_OPTIONS "-std=c++11 -Wall -Wextra")
  set(CMAKE_CXX_FLAGS                "${CMAKE_CXX_FLAGS} ${BASE_COMPILER_COMPTIONS} -O3")
  set(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG} ${BASE_COMPILER_OPTIONS} -O -g -DJASON_DEBUG=1")
  set(CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL} ${BASE_COMPILER_OPTIONS} -Os")
  set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE} ${BASE_COMPILER_OPTIONS} -O3")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${BASE_COMPILER_OPTIONS} -O3 -g")
  
  set(USE_SSE4_2 ${SSE4_2_FOUND} CACHE BOOL "Use SSE4.2 optimizations")
  if(USE_SSE4_2)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2 -DRAPIDJSON_SSE42")
  endif()

elseif(MSVC) 
  set(${CMAKE_CXX_FLAGS} "${CMAKE_CXX_FLAGS} /MT")
  set(${CMAKE_CXX_FLAGS_DEBUG} "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

option(BuildBench "Build bench performance test suite" OFF)
option(BuildTests "Build test suite" OFF)

include_directories(${PROJECT_SOURCE_DIR}/include)

find_package(Threads)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
  
add_library(
  jason
  STATIC
  Jason.cpp
  JasonAsm.cpp
  JasonBuilder.cpp
  JasonParser.cpp
  JasonSlice.cpp
  JasonType.cpp
  fpconv.cpp
)

if(BuildBench)
  include_directories(${PROJECT_SOURCE_DIR}/rapidjson/include)

  add_executable(
    bench
    bench.cpp
  )

  target_link_libraries(
    bench
    jason
  )
endif()

if(BuildTests)
  include_directories(${PROJECT_SOURCE_DIR}/googletest/googletest/include)

  find_library(GTEST_LIBRARY NAMES gtest gtest.a libgtest libgtest.a PATHS googletest/googletest)
  message(STATUS "Gtest library found at: ${GTEST_LIBRARY}")


  add_executable(
    tests
    tests.cpp
  )

  target_link_libraries(
    tests
    ${GTEST_LIBRARY}
    jason
  )
endif()

## Local Variables:
## mode: outline-minor
## outline-regexp: "### @brief\\|## --SECTION--\\|# -\\*- "
## End:
