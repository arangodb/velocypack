////////////////////////////////////////////////////////////////////////////////
/// @brief Library to build up VPack documents.
///
/// DISCLAIMER
///
/// Copyright 2022 ArangoDB GmbH, Cologne, Germany
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
/// @author Julia Puget
/// @author Copyright 2022, ArangoDB GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>

#include "velocypack/vpack.h"
#include "velocypack/velocypack-exception-macros.h"
#include <random>
#include <thread>

using namespace arangodb::velocypack;

enum class Format {
  VPACK, JSON
};

enum RandomBuilderAdditions {ADD_ARRAY=0, ADD_OBJECT, ADD_BOOLEAN, ADD_STRING, ADD_NULL, ADD_UINT64, ADD_INT64, ADD_DOUBLE};

struct RandomGenerator {
  RandomGenerator() : mt64{rd()} {}

  std::random_device rd;
  std::mt19937_64 mt64;
};

static void usage(char* argv[]) {
  std::cout << "Usage: " << argv[0] << " [OPTIONS] [ITERATIONS]"
            << std::endl;
  std::cout << "This program creates random VPack or JSON structures and validates them."
            << std::endl;
  std::cout << "The amout of times it does this is supplied by <iterations>."
            << std::endl;
  std::cout << "The paralelization is supplied by <threads>."
            << std::endl;
  std::cout << "Available options are:" << std::endl;
  std::cout << " --vpack       create VPack."
            << std::endl;
  std::cout << " --json        create JSON."
            << std::endl;
  std::cout << " <iterations>  number of iterations. Default: 1"
            << std::endl;
  std::cout << " <threads>  number of threads. Default: 1"
            << std::endl;
}

static inline bool isOption(char const* arg, char const* expected) {
  return (strcmp(arg, expected) == 0);
}

static void addString(Builder& builder, RandomGenerator& randomGenerator) {
  static auto &availableChars = "0123456789"
                                "abcdefghijklmnopqrstuvwxyz"
                                "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  size_t length = randomGenerator.mt64() % 1000;
  std::string s;
  s.reserve(length);
  while (length--) {
    s += availableChars[randomGenerator.mt64() % (sizeof(availableChars) - 1)];
  }
  builder.add(Value(s));
}

static void generateVelocypack(Builder& builder, size_t depth, RandomGenerator& randomGenerator) {
  RandomBuilderAdditions randomBuilderAdds;
  while (true) {
    randomBuilderAdds = static_cast<RandomBuilderAdditions>(randomGenerator.mt64() % 8);
    if (depth > 10 && randomBuilderAdds < 2) {
      continue;
    }
    switch (randomBuilderAdds) {
      case ADD_ARRAY: {
        builder.openArray(randomGenerator.mt64() % 2 ? true : false);
        size_t numMembers = randomGenerator.mt64() % 10;
        for (size_t i = 0; i < numMembers; ++i) {
          generateVelocypack(builder, depth + 1, randomGenerator);
        }
        builder.close();
        break;
      }
      case ADD_OBJECT: {
        builder.openObject(randomGenerator.mt64() % 2 ? true : false);
        size_t numMembers = randomGenerator.mt64() % 10;
        for (size_t i = 0; i < numMembers; ++i) {
          std::string key = "test" + std::to_string(i);
          builder.add(Value(key));
          generateVelocypack(builder, depth + 1, randomGenerator);
        }
        builder.close();
        break;
      }
      case ADD_BOOLEAN:
        builder.add(Value(randomGenerator.mt64() % 2 ? true : false));
        break;
      case ADD_STRING:
        addString(builder, randomGenerator);
        break;
      case ADD_NULL:
        builder.add(Value(ValueType::Null));
        break;
      case ADD_UINT64:
        builder.add(Value(randomGenerator.mt64()));
        break;
      case ADD_INT64: {
        uint64_t uintValue = randomGenerator.mt64();
        int64_t intValue;
        memcpy(&intValue, &uintValue, sizeof(uintValue));
        builder.add(Value(intValue));
        break;
      }
      case ADD_DOUBLE: {
        uint64_t uintValue = randomGenerator.mt64();
        double doubleValue;
        memcpy(&doubleValue, &uintValue, sizeof(uintValue));
        builder.add(Value(doubleValue));
      }
      default:
        break;
    }
    break;
  }
}

int main(int argc, char* argv[]) {
  VELOCYPACK_GLOBAL_EXCEPTION_TRY
    std::vector<std::thread> threads;
    bool isTypeAssigned = false;
    size_t numIterations = 1;
    size_t numThreads = 1;
    Format format;

    int i = 1;
    while (i < argc) {
      char const *p = argv[i];
      if (isOption(p, "--help")) {
        usage(argv);
        return EXIT_SUCCESS;
      } else if (isOption(p, "--vpack") && !isTypeAssigned) {
        isTypeAssigned = true;
        format = Format::VPACK;
      } else if (isOption(p, "--json") && !isTypeAssigned) {
        isTypeAssigned = true;
        format = Format::JSON;
      } else if (int value = atoi(p); value >= 1) {
        if (i == 3) { // as threads and iterations are optional arguments, threads must be preceeded by iterations
                      // in the command line, hence, argument 3
          numThreads = value;
        } else {
          numIterations = value;
        }
      } else {
        usage(argv);
        return EXIT_FAILURE;
      }
      ++i;
    }

    size_t itsPerThread = numIterations / numThreads;
    size_t leftoverIts = numIterations % numThreads;

    auto threadCallback = [format] (size_t iterations) {
      RandomGenerator randomGenerator;
      while (iterations-- > 0) {
        Builder builder;
        generateVelocypack(builder, 0, randomGenerator);

        if (format == Format::JSON) {
          Parser parser;
          parser.parse(builder.slice().toJson());
        } else {
          Validator validator;
          validator.validate(builder.slice().start(), builder.slice().byteSize());
        }
      }
    };

    for (size_t i = 0; i < numThreads; ++i) {
      size_t iterations = itsPerThread;
      if (i == numThreads - 1) {
        iterations += leftoverIts;
      }
      threads.emplace_back(std::thread(threadCallback, iterations));
    }

    for (auto &thread: threads) {
      if (thread.joinable()) {
        thread.join();
      }
    }
  VELOCYPACK_GLOBAL_EXCEPTION_CATCH
}
