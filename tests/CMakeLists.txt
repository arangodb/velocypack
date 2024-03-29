# -*- mode: CMAKE; -*-

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION False)

# options
option(BuildTests "Build test suite" OFF)
message(STATUS "VelocyPack building test suite: ${BuildTests}")

option(BuildLargeTests "Build large tests" OFF)
message(STATUS "VelocyPack building large tests: ${BuildLargeTests}")

option(BuildAsmTest "Build assembler test code" OFF)
message(STATUS "VelocyPack building assembler test suite: ${BuildAsmTest}")

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
    testsHashedStringRef
    testsHexDump
    testsIterator
    testsLookup
    testsParser
    testsSerializable
    testsSharedSlice
    testsSink
    testsSlice
    testsSliceContainer
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
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
      set_source_files_properties(${testCpp} PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations")
    endif()
    target_link_libraries(${testName} gtest velocypack ${CMAKE_THREAD_LIBS_INIT})
    add_test(${testName} ${testName})
endmacro()

# build tests.cpp
if(BuildTests)
    include(CTest)
    enable_testing()
    find_package(Threads REQUIRED)
    if (NOT TARGET googletest)
      if (MSVC)
        # Configure google to match selected CRT runtime.
        # We check only Release variables as GTest itself does not support
        # using different runtimes for Debug and Release.
        foreach (flag_var
                CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_RELEASE)
          string(FIND "${${flag_var}}" "/MD" MD_POS)
          if(${MD_POS} GREATER_EQUAL 0)
            message("Dynamic runtime detected! Setting GTest to use dynamic runtime for static build!")
            SET(gtest_force_shared_crt ON CACHE BOOL 
                "Use shared (DLL) run-time lib even when Google Test is built as static lib." FORCE)
            break()
          endif()
        endforeach()
      endif()
      add_subdirectory(googletest REQUIRED)
    endif()
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
