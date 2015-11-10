
# options
option(BuildExamples "Build examples" ON)
message(STATUS "Building examples: ${BuildExamples}")

# build examples
if(BuildExamples)
  foreach (exampleName
           exampleArrayHandling
           exampleArrayIterator
           exampleBuilder
           exampleBuilderFancy
           exampleCustom
           exampleDumper
           exampleDumperFile
           exampleDumperPretty
           exampleObjectIterator
           exampleObjectLookup
           exampleObjectMethods
           exampleParser
           exampleSlice)

    add_executable(
      ${exampleName}
      ${exampleName}.cpp
    )
  
    target_link_libraries(
      ${exampleName}
      velocypack
    )

  endforeach()
endif()

