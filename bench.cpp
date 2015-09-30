#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "Jason.h"
#include "JasonBuilder.h"
#include "JasonParser.h"
#include "JasonSlice.h"
#include "JasonType.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using Jason            = triagens::basics::Jason;
using JasonBuilder     = triagens::basics::JasonBuilder;
using JasonLength      = triagens::basics::JasonLength;
using JasonPair        = triagens::basics::JasonPair;
using JasonParser      = triagens::basics::JasonParser;
using JasonSlice       = triagens::basics::JasonSlice;
using JasonType        = triagens::basics::JasonType;
  
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
  string s;
  int fd = open(argv[1], O_RDONLY);
  char buffer[4096];
  int len;
  while (true) {
    len = read(fd, buffer, 4096);
    if (len <= 0) {
      break;
    }
    s.append(buffer, len);
  }
  close(fd);

  inputs.push_back(s);
  outputs.push_back(new JasonParser());

  for (size_t i = 1; i < copies; i++) {
    // Make an explicit copy:
    s.clear();
    s.insert(s.begin(), inputs[0].begin(), inputs[0].end());
    inputs.push_back(s);
    outputs.push_back(new JasonParser());
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
  return EXIT_SUCCESS;
}
