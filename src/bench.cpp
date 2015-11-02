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
#include <vector>
#include <fstream>
#include <chrono>
#include <thread>

#include "Jason.h"
#include "JasonBuilder.h"
#include "JasonParser.h"
#include "JasonSlice.h"
#include "JasonType.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using Jason            = arangodb::jason::Jason;
using JasonBuilder     = arangodb::jason::JasonBuilder;
using JasonLength      = arangodb::jason::JasonLength;
using JasonPair        = arangodb::jason::JasonPair;
using JasonParser      = arangodb::jason::JasonParser;
using JasonSlice       = arangodb::jason::JasonSlice;
using JasonType        = arangodb::jason::JasonType;
  
using namespace std;

static void usage () {
  cout << "Usage: FILENAME.json RUNTIME_IN_SECONDS COPIES TYPE" << endl;
  cout << "This program reads the file into a string, makes COPIES copies" << endl;
  cout << "and then parses the copies in a round-robin fashion to Jason." << endl;
  cout << "1 copy means its running in cache, more copies make it run" << endl;
  cout << "out of cache. The target areas are also in a different memory" << endl;
  cout << "area for each copy." << endl;
  cout << "TYPE must be either 'jason' or 'rapidjson'." << endl;
}

int main (int argc, char* argv[]) {
  if (argc < 5) {
    usage();
    return EXIT_FAILURE;
  }

  bool useJason;
  if (strcmp(argv[4], "jason") == 0) {
    useJason = true;
  }
  else if (strcmp(argv[4], "rapidjson") == 0) {
    useJason = false;
  }
  else {
    usage();
    return EXIT_FAILURE;
  }
    

  size_t copies = stoul(argv[3]);
  int runTime = stoi(argv[2]);
  vector<string> inputs;
  vector<JasonParser*> outputs;

  // read input file
  std::string s;
  std::ifstream ifs(argv[1], std::ifstream::in);

  if (! ifs.is_open()) {
    std::cerr << "Cannot open input file" << std::endl;
    return EXIT_FAILURE;
  }
  
  char buffer[4096];
  while (ifs.good()) {
    ifs.read(&buffer[0], sizeof(buffer));
    s.append(buffer, ifs.gcount());
  }
  ifs.close();

  inputs.push_back(s);
  outputs.push_back(new JasonParser());
  outputs.back()->options.sortAttributeNames = false;

  for (size_t i = 1; i < copies; i++) {
    // Make an explicit copy:
    s.clear();
    s.insert(s.begin(), inputs[0].begin(), inputs[0].end());
    inputs.push_back(s);
    outputs.push_back(new JasonParser());
    outputs.back()->options.sortAttributeNames = false;
  }

  size_t count = 0;
  size_t total = 0;
  auto start = chrono::high_resolution_clock::now();
  decltype(start) now;

  do {
    for (int i = 0; i < 2; i++) {
      if (useJason) {
        outputs[count]->clear();
        outputs[count]->parse(inputs[count]);
      }
      else {
        rapidjson::Document d;
        d.Parse(inputs[count].c_str());
      }
      count++;
      if (count >= copies) {
        count = 0;
      }
      total++;
    }
    now = chrono::high_resolution_clock::now();
  } while (chrono::duration_cast<chrono::duration<int>>(now - start).count() < runTime);

  chrono::duration<double> totalTime 
      = chrono::duration_cast<chrono::duration<double>>(now - start);
  cout << "Total runtime: " << totalTime.count() << " s" << endl;
  cout << "Have parsed " << total << " times with " << argv[4] << " using " << copies
       << " copies of JSON data, each of size " << inputs[0].size() << "."
       << endl;
  cout << "Parsed " << inputs[0].size() * total << " bytes in total." << endl;
  cout << "This is " << static_cast<double>(inputs[0].size() * total) /
                        totalTime.count() << " bytes/s"
       << " or " << total / totalTime.count() 
       << " JSON docs per second." << endl;

  for (auto& it : outputs) {
    delete it;
  }
  return EXIT_SUCCESS;
}
