
# options
option(BuildExamples "Build examples" ON)
message(STATUS "Building examples: ${BuildExamples}")

# build examples
if(BuildExamples)
  foreach (exampleName
           exampleAliases
           exampleArrayHandling
           exampleArrayIterator
           exampleAttributeTranslation
           exampleBuilder
           exampleBuilderFancy
           exampleCustom
           exampleDumper
           exampleDumperFile
           exampleDumperPretty
           exampleObjectHandling
           exampleObjectIterator
           exampleObjectLookup
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

    add_dependencies(${exampleName} velocypack) 
  endforeach()
endif()

