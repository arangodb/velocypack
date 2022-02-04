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

#include <bitset>
#include <charconv>
#include <iostream>
#include <random>
#include <string>
#include <thread>

#include "velocypack/vpack.h"
#include "velocypack/velocypack-exception-macros.h"

using namespace arangodb::velocypack;

enum class Format {
  VPACK, JSON
};

enum RandomBuilderAdditions {
  ADD_ARRAY = 0,
  ADD_OBJECT,
  ADD_BOOLEAN,
  ADD_STRING,
  ADD_NULL,
  ADD_UINT64,
  ADD_INT64,
  ADD_DOUBLE, // here starts values that are only for vpack
  ADD_UTC_DATE,
  ADD_BINARY,
  ADD_EXTERNAL,
  ADD_ILLEGAL,
  ADD_MIN_KEY,
  ADD_MAX_KEY,
  ADD_MAX_VPACK_VALUE
};

struct RandomGenerator {
  RandomGenerator() : mt64{rd()} {}

  std::random_device rd;
  std::mt19937_64 mt64;
};

std::vector<std::jthread> jthreads;


static void finalize() {
  for (auto& jthread : jthreads) {
    jthread.request_stop(); // this is said to be safely called concurrently for the same object
  }
}


static void usage(char* argv[]) {
  std::cout << "Usage: " << argv[0] << " [OPTIONS] [ITERATIONS] [THREADS]"
            << std::endl;
  std::cout << "This program creates <iterations> random VPack or JSON structures and validates them."
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

static void generateVelocypack(Builder& builder, size_t depth, RandomGenerator& randomGenerator, Format const& format) {
  while (true) {
    RandomBuilderAdditions randomBuilderAdds = static_cast<RandomBuilderAdditions>(randomGenerator.mt64() %
                                                                                   (format == Format::JSON
                                                                                    ? ADD_DOUBLE
                                                                                    : ADD_MAX_VPACK_VALUE));
    if (depth > 10 && randomBuilderAdds <= ADD_OBJECT) {
      continue;
    }
    switch (randomBuilderAdds) {
      case ADD_ARRAY: {
        builder.openArray(randomGenerator.mt64() % 2 ? true : false);
        size_t numMembers = randomGenerator.mt64() % 10;
        for (size_t i = 0; i < numMembers; ++i) {
          generateVelocypack(builder, depth + 1, randomGenerator, format);
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
          generateVelocypack(builder, depth + 1, randomGenerator, format);
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
      case ADD_DOUBLE: { //exception was thrown saying JSON doesn't support this type
        uint64_t uintValue = randomGenerator.mt64();
        double doubleValue;
        memcpy(&doubleValue, &uintValue, sizeof(uintValue));
        builder.add(Value(doubleValue));
        break;
      }
      case ADD_UTC_DATE:
        builder.add(Value(randomGenerator.mt64(), ValueType::UTCDate));
        break;
      case ADD_BINARY:
        builder.add(Value(std::bitset<64>(randomGenerator.mt64()).to_string()));
        break;
      case ADD_EXTERNAL: {
        Builder extBuilder;
        extBuilder.add(Value(randomGenerator.mt64()));
        builder.add(Value(static_cast<void const *>(extBuilder.slice().start()), ValueType::External));
        break;
      }
      case ADD_ILLEGAL:
        builder.add(Value(ValueType::Illegal));
        break;
      case ADD_MIN_KEY:
        builder.add(Value(ValueType::MinKey));
        break;
      case ADD_MAX_KEY:
        builder.add(Value(ValueType::MaxKey));
      default:
        break;
    }
    break;
  }
}

int main(int argc, char* argv[]) {
  VELOCYPACK_GLOBAL_EXCEPTION_TRY
    bool isTypeAssigned = false;
    size_t numIterations = 1;
    size_t numThreads = 1;
    Format format = Format::VPACK;

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
      } else {
        int value;
        std::from_chars(p, p + sizeof(p), value);
        if (value >= 1) {
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
      }
      ++i;
    }

    size_t itsPerThread = numIterations / numThreads;
    size_t leftoverIts = numIterations % numThreads;

    auto threadCallback = [format](std::stop_token const& stoken, size_t iterations) {
      try {
        RandomGenerator randomGenerator;
        while (iterations-- > 0) {
          if (stoken.stop_requested()) {
            return;
          }
          Builder builder;
          generateVelocypack(builder, 0, randomGenerator, format);

          if (format == Format::JSON) {
            Parser parser;
            parser.parse(builder.slice().toJson());
          } else {
            Validator validator;
            validator.validate(builder.slice().start(), builder.slice().byteSize());
          }
        }
      } catch (std::exception const& e) {
        std::cerr << "Program encountered exception on thread execution: " << e.what() << std::endl;
        finalize();
        return;
      }
    };

    std::vector<std::thread> threads;
    for (size_t i = 0; i < numThreads; ++i) {
      size_t iterations = itsPerThread;
      if (i == numThreads - 1) {
        iterations += leftoverIts;
      }
      jthreads.emplace_back(std::jthread(threadCallback, iterations));
    }

    for (auto& jthread: jthreads) {
      jthread.join();
    }

  VELOCYPACK_GLOBAL_EXCEPTION_CATCH
}
