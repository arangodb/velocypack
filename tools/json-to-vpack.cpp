////////////////////////////////////////////////////////////////////////////////
/// @brief Library to build up VPack documents.
///
/// DISCLAIMER
///
/// Copyright 2015 ArangoDB GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Max Neunhoeffer
/// @author Jan Steemann
/// @author Copyright 2015, ArangoDB GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <fstream>

#include "velocypack/vpack.h"

using namespace arangodb::velocypack;

static void usage (char* argv[]) {
  std::cout << "Usage: " << argv[0] << " [OPTIONS] INFILE OUTFILE" << std::endl;
  std::cout << "This program reads the JSON INFILE into a string and saves its" << std::endl;
  std::cout << "VPack representation in file OUTFILE. Will work only for input" << std::endl;
  std::cout << "files up to 2 GB size." << std::endl;
  std::cout << "Available options are:" << std::endl;
  std::cout << " --compact       store Array and Object types without index tables" << std::endl;
  std::cout << " --no-compact    store Array and Object types with index tables" << std::endl;
}

int main (int argc, char* argv[]) {
  char const* infileName = nullptr;
  char const* outfileName = nullptr;
  bool compact = true;
  bool resetStream = true;

  int i = 1;
  while (i < argc) {
    char const* p = argv[i];
    if (strncmp("--compact", p, strlen("--compact")) == 0) {
      compact = true;
    }
    else if (strncmp("--no-compact", p, strlen("--no-compact")) == 0) {
      compact = false;
    }
    else if (infileName == nullptr) {
      infileName = p;
    }
    else if (outfileName == nullptr) {
      outfileName = p;
    }
    else {
      usage(argv);
      return EXIT_FAILURE;
    }
    ++i;
  }

  if (infileName == nullptr) {
    usage(argv);
    return EXIT_FAILURE;
  }

#ifdef __linux__
  // treat missing outfile as stdout
  if (outfileName == nullptr) {
    outfileName = "/proc/self/fd/1";
    resetStream = false;
  }
#else 
  if (outfileName == nullptr) {
    usage(argv);
    return EXIT_FAILURE;
  }
#endif

  // treat "-" as stdin
  std::string infile = infileName;
#ifdef __linux__
  if (infile == "-") {
    infile = "/proc/self/fd/0";
  }
#endif

  std::string s;
  std::ifstream ifs(infile, std::ifstream::in);

  if (! ifs.is_open()) {
    std::cerr << "Cannot read infile '" << infile << "'" << std::endl;
    return EXIT_FAILURE;
  }

  char buffer[4096];
  while (ifs.good()) {
    ifs.read(&buffer[0], sizeof(buffer));
    s.append(buffer, ifs.gcount());
  }
  ifs.close();

  Options options;
  options.buildUnindexedArrays = compact;
  options.buildUnindexedObjects = compact;

  Parser parser(&options);
  try {
    parser.parse(s);
  }
  catch (Exception const& ex) {
    std::cerr << "An exception occurred while parsing infile '" << infile << "': " << ex.what() << std::endl;
    std::cerr << "Error position: " << parser.errorPos() << std::endl;
    return EXIT_FAILURE;
  }
  catch (...) {
    std::cerr << "An unknown exception occurred while parsing infile '" << infile << "'" << std::endl;
    return EXIT_FAILURE;
  }
 
  std::ofstream ofs(outfileName, std::ofstream::out);
 
  if (! ofs.is_open()) {
    std::cerr << "Cannot write outfile '" << outfileName << "'" << std::endl;
    return EXIT_FAILURE;
  }

  // reset stream
  if (resetStream) {
    ofs.seekp(0);
  }

  // write into stream
  Builder builder = parser.steal();
  uint8_t const* start = builder.start();
  ofs.write(reinterpret_cast<char const*>(start), builder.size());

  ofs.close();
  
  std::cout << "Successfully converted JSON infile '" << infile << "'" << std::endl;
  std::cout << "JSON Infile size:   " << s.size() << std::endl;
  std::cout << "VPack Outfile size: " << builder.size() << std::endl;
  
  return EXIT_SUCCESS;
}
