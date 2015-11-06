# -*- mode: CMAKE; -*-

# options
option(BuildTests "Build test suite" OFF)
message(STATUS "Building test suite: ${BuildTests}")
  
option(BuildAsmTest "Build assembler test code" OFF)
message(STATUS "Building assembler test suite: ${BuildAsmTest}")

# build tests.cpp
if(BuildTests)
  add_subdirectory(googletest/googletest)

  include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
  foreach (testName
           testsBuilder
           testsCollection
           testsDumper
           testsFiles
           testsIterator
           testsLarge
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

  endforeach()
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


