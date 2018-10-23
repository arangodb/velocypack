# -*- mode: CMAKE; -*-

cmake_minimum_required(VERSION 2.8)
project(velocypack CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

option(BuildVelocyPackExamples "Build examples" ON)
option(Maintainer "Build maintainer tools" OFF)
option(Coverage "Build with coverage" OFF)
option(HashType "Hash type (fasthash, xxhash)" OFF)

#Default to xxhash
if(HashType STREQUAL "OFF")
    set(HashType "xxhash")
endif()

# Set Build Type
if(NOT CMAKE_BUILD_TYPE)
    #why don't we use release as default?
    set(CMAKE_BUILD_TYPE Release CACHE string "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()
if(Coverage)
    set(CMAKE_BUILD_TYPE Debug)
endif()


message(STATUS "Build mode: ${CMAKE_BUILD_TYPE}")
message(STATUS "Install base directory: ${CMAKE_INSTALL_PREFIX}")
set(VELOCYPACK_VERSION "0.0.1" CACHE path "VelocyPack version")
set(VELOCYPACK_DISPLAY_NAME "Velocypack")
set(VELOCYPACK_URL_INFO_ABOUT "https://github.com/arangodb/velocypack")
set(VELOCYPACK_CONTACT "hackers@arangodb.org")
set(VELOCYPACK_FRIENDLY_STRING "VelocyPack - A fast & compact serialization format")


#Compiler Setting
include(AR_CompilerSettings)

# build version number generator - NICE!
set(VELOCY_SOURCE
    src/velocypack-common.cpp
    src/AttributeTranslator.cpp
    src/Builder.cpp
    src/Collection.cpp
    src/Dumper.cpp
    src/Exception.cpp
    src/HexDump.cpp
    src/Iterator.cpp
    src/Options.cpp
    src/Parser.cpp
    src/Slice.cpp
    src/SliceStaticData.cpp
    src/StringRef.cpp
    src/Utf8Helper.cpp
    src/Validator.cpp
    src/ValueType.cpp
    src/Version.cpp
    src/asm-functions.cpp
    src/asm-utf8check.cpp
    src/fpconv.cpp
)

#Use xxhash or fasthash?
if(HashType STREQUAL "xxhash")
    list(APPEND VELOCY_SOURCE src/xxhash.cpp)
    add_definitions("-DVELOCYPACK_XXHASH=1")
elseif(HashType STREQUAL "fasthash")
    list(APPEND VELOCY_SOURCE src/fasthash.cpp)
    add_definitions("-DVELOCYPACK_FASTHASH=1")
else()
    message(FATAL_ERROR "invalid HashType value. supported values: xxhash or fasthash")
endif()
message(STATUS "Building with hash type: ${HashType}")

add_library(velocypack STATIC ${VELOCY_SOURCE})
target_include_directories(velocypack PRIVATE src)
target_include_directories(velocypack PUBLIC include)

if(Maintainer)
    add_executable(buildVersion scripts/build-version.cpp)
    add_custom_target(buildVersionNumber
        COMMAND ${PROJECT_BINARY_DIR}/scripts/buildVersion
        ${PROJECT_SOURCE_DIR}/include/velocypack/velocypack-version-number.h
        DEPENDS ${PROJECT_BINARY_DIR}/scripts/buildVersion
        VERBATIM
    )
endif()

add_subdirectory(tools)
add_subdirectory(examples)
add_subdirectory(tests)

install(TARGETS velocypack DESTINATION lib)
install(
    DIRECTORY   "include/velocypack"
    DESTINATION include
)
include(CPack)
