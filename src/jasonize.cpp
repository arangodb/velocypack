////////////////////////////////////////////////////////////////////////////////
/// @brief Library to build up Jason documents.
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
#include <thread>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "Jason.h"
#include "JasonBuilder.h"
#include "JasonException.h"
#include "JasonParser.h"
#include "JasonSlice.h"
#include "JasonType.h"

using Jason            = arangodb::jason::Jason;
using JasonBuilder     = arangodb::jason::JasonBuilder;
using JasonException   = arangodb::jason::JasonException;
using JasonLength      = arangodb::jason::JasonLength;
using JasonParser      = arangodb::jason::JasonParser;
using JasonSlice       = arangodb::jason::JasonSlice;
using JasonType        = arangodb::jason::JasonType;
  
using namespace std;

static void usage () {
  cout << "Usage: INFILE OUTFILE" << endl;
  cout << "This program reads the JSON INFILE into a string and saves its" << endl;
  cout << "Jason representation in file OUTFILE. Will work only for input" << endl;
  cout << "files up to 2 GB size." << endl;
}

int main (int argc, char* argv[]) {
  if (argc < 3) {
    usage();
    return EXIT_FAILURE;
  }

  std::string s;
  int fd = ::open(argv[1], O_RDONLY);
  if (fd < 0) {
    cerr << "Cannot read infile '" << argv[1] << "'" << endl;
    return EXIT_FAILURE;
  }

  char buffer[4096];
  int len;
  while (true) {
    len = ::read(fd, buffer, 4096);
    if (len <= 0) {
      break;
    }
    s.append(buffer, len);
  }
  ::close(fd);
  
  JasonParser parser;
  try {
    parser.parse(s);
  }
  catch (JasonException const& ex) {
    cerr << "An exception while parsing infile '" << argv[1] << "': " << ex.what() << endl;
    return EXIT_FAILURE;
  }
  catch (...) {
    cerr << "An unknown exception occurred while parsing infile '" << argv[1] << "'" << endl;
    return EXIT_FAILURE;
  }
  
  JasonBuilder builder = parser.steal();
  uint8_t const* start = builder.start();
  size_t end = static_cast<size_t>(builder.size());
 
  fd = ::open(argv[2], O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

  if (fd < 0) {
    cerr << "Cannot write outfile '" << argv[2] << "'" << endl;
    return EXIT_FAILURE;
  }
  int res = ::ftruncate(fd, 0);
  if (res < 0) {
    cerr << "Cannot write outfile '" << argv[2] << "'" << endl;
    ::close(fd);
    return EXIT_FAILURE;
  }

  size_t current = 0;
  while (current < end) {
    size_t toWrite = end - current;
    if (toWrite > 16384) {
      toWrite = 16384;
    }
    int len = ::write(fd, start + current, toWrite);
    if (len > 0) {
      current += static_cast<size_t>(len);
    }
    else if (len <= 0) {
      ::close(fd);
      cerr << "Cannot write outfile '" << argv[2] << "'" << endl;
      return EXIT_FAILURE;
    }
  }
  ::close(fd);

  cout << "Successfully converted JSON infile '" << argv[1] << "'" << endl;
  cout << "JSON Infile size:   " << s.size() << endl;
  cout << "Jason Outfile size: " << builder.size() << endl;
  
  return EXIT_SUCCESS;
}
