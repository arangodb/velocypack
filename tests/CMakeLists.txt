# -*- mode: CMAKE; -*-

# options
option(BuildTests "Build test suite" OFF)
message(STATUS "Building test suite: ${BuildTests}")

option(BuildLargeTests "Build large tests" OFF)
message(STATUS "Building large tests: ${BuildLargeTests}")

option(BuildAsmTest "Build assembler test code" OFF)
message(STATUS "Building assembler test suite: ${BuildAsmTest}")

set(Tests
    testsAliases
    testsBuffer
    testsBuilder
    testsCollection
    testsCommon
    testsCompare
    testsDumper
    testsException
    testsFiles
    testsHexDump
    testsIterator
    testsLookup
    testsParser
    testsSerializable
    testsSlice
    testsSliceContainer
    testsStringRef
    testsType
    testsValidator
    testsVersion
)

macro(standard_test test_name)
    set(testName ${test_name})
    if("${ARGV1}" STREQUAL "")
        set(testCpp ${testName}.cpp)
    else()
        set(testCpp ${ARGV1})
    endif()

    include_directories(${testName} PRIVATE ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
    add_executable(${testName} ${testCpp})
    target_link_libraries(${testName} gtest velocypack ${CMAKE_THREAD_LIBS_INIT})
    add_test(${testName} ${testName})
endmacro()

# build tests.cpp
if(BuildTests)
    include(CTest)
    enable_testing()
    find_package(Threads REQUIRED)
    add_subdirectory(googletest REQUIRED)

    foreach(testName ${Tests})
        standard_test(${testName})
    endforeach()
    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/jsonSample" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")

    if(BuildLargeTests)
        standard_test(testsLarge)
    endif()
endif()

# build AsmTest
if (BuildAsmTest)
    add_executable(AsmTest "${CMAKE_SOURCE_DIR}/src/asm-functions.cpp")
    target_compile_definitions(AsmTest PRIVATE COMPILE_VELOCYPACK_ASM_UNITTESTS)
    target_link_libraries(AsmTest velocypack ${CMAKE_THREAD_LIBS_INIT})
    add_test(AsmTest AsmTest 300 10000000 100)
endif()
