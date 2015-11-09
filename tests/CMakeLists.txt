# -*- mode: CMAKE; -*-

# options
option(BuildTests "Build test suite" OFF)
message(STATUS "Building test suite: ${BuildTests}")

option(BuildLargeTests "Build large tests" OFF)
message(STATUS "Building large tests: ${BuildLargeTests}")
  
option(BuildAsmTest "Build assembler test code" OFF)
message(STATUS "Building assembler test suite: ${BuildAsmTest}")

# build tests.cpp
if(BuildTests)
  include(CTest)

  enable_testing()
  add_subdirectory(googletest/googletest)

  include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
  foreach(testName
          testsBuilder
          testsCollection
          testsDumper
          testsFiles
          testsHexDump
          testsIterator
          testsLookup
          testsParser
          testsSlice
          testsType)
    add_executable(
      ${testName}
      ${testName}.cpp
    )
  
    target_link_libraries(
      ${testName}
      gtest
      velocypack
      ${CMAKE_THREAD_LIBS_INIT}
    )

    add_test(${testName} ${testName})
  endforeach()

  if (BuildLargeTests)
    add_executable(
      testsLarge
      testsLarge.cpp
    )
  
    target_link_libraries(
      testsLarge
      gtest
      velocypack
      ${CMAKE_THREAD_LIBS_INIT}
    )
    add_test(testsLarge testsLarge)
  endif()
endif()

# build AsmTest
if (BuildAsmTest)
  add_executable(
    AsmTest
    src/asm-functions.cpp
  )

  target_link_libraries(
    AsmTest
    ${CMAKE_THREAD_LIBS_INIT}
  )

  set_target_properties(AsmTest PROPERTIES COMPILE_FLAGS "-DCOMPILE_VELOCYPACK_ASM_UNITTESTS")

endif()


