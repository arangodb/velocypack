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
#include <memory>

#include "velocypack/vpack.h"
#include "velocypack/velocypack-exception-macros.h"
#include <random>

using namespace arangodb::velocypack;

enum class Format {
  VPACK, JSON
};

enum RandomBuilderAdditions {ADD_ARRAY=0, ADD_OBJECT, ADD_BOOLEAN, ADD_STRING, ADD_NULL, ADD_UINT64, ADD_INT64, ADD_DOUBLE};

struct RandomGenerator {
  RandomGenerator() : mt{rd()}, mt64{rd()} {}

  std::random_device rd;
  std::mt19937 mt;
  std::mt19937_64 mt64;
};

RandomGenerator randomGenerator;

static void usage(char* argv[]) {
  std::cout << "Usage: " << argv[0] << " [OPTIONS] [ITERATIONS]"
            << std::endl;
  std::cout << "This program creates random VPack or JSON structures and validates them."
            << std::endl;
  std::cout << "The amout of times it does this is supplied by <iterations>."
            << std::endl;
  std::cout << "Available options are:" << std::endl;
  std::cout << " --vpack       create VPack."
            << std::endl;
  std::cout << " --json        create JSON."
            << std::endl;
  std::cout << " <iterations>  number of iterations. Default: 1"
            << std::endl;
}

static inline bool isOption(char const* arg, char const* expected) {
  return (strcmp(arg, expected) == 0);
}

static void addString(Builder& builder) {
  static auto &availableChars = "0123456789"
                                "abcdefghijklmnopqrstuvwxyz"
                                "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  size_t length = randomGenerator.mt() % 1000;
  std::string s;
  s.reserve(length);
  while (length--) {
    s += availableChars[randomGenerator.mt() % (sizeof(availableChars) - 1)];
  }
  builder.add(Value(s));
}

static void generateVelocypack(Builder& builder, size_t depth) {
  RandomBuilderAdditions randomBuilderAdds;
  while (true) {
    randomBuilderAdds = static_cast<RandomBuilderAdditions>(randomGenerator.mt64() % 9);
    if (depth > 10 && randomBuilderAdds < 2) {
      continue;
    }
    switch (randomBuilderAdds) {
      case ADD_ARRAY: {
        builder.openArray(randomGenerator.mt() % 2 ? true : false);
        size_t numMembers = randomGenerator.mt() % 10;
        for (size_t i = 0; i < numMembers; ++i) {
          generateVelocypack(builder, depth + 1);
        }
        builder.close();
        break;
      }
      case ADD_OBJECT: {
        builder.openObject(randomGenerator.mt() % 2 ? true : false);
        size_t numMembers = randomGenerator.mt() % 10;
        for (size_t i = 0; i < numMembers; ++i) {
          std::string key = "test" + std::to_string(i);
          builder.add(Value(key));
          generateVelocypack(builder, depth + 1);
        }
        builder.close();
        break;
      }
      case ADD_BOOLEAN:
        builder.add(Value(randomGenerator.mt64() % 2 ? true : false));
        break;
      case ADD_STRING:
        addString(builder);
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
        if (uintValue <= std::numeric_limits<uint64_t>::max() ) {
          intValue = uintValue;
        } else {
          intValue = 1; // to be changed
        }
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
    bool isTypeAssigned = false;
    size_t iterations = 1;
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
        iterations = value;
      } else {
        usage(argv);
        return EXIT_FAILURE;
      }
      ++i;
    }

    while (iterations-- > 0) {
      Builder builder;
      generateVelocypack(builder, 0);

      if (format == Format::JSON) {
        Parser parser;
        parser.parse(builder.slice().toJson());
      } else {
        Validator validator;
        validator.validate(builder.slice().start(), builder.slice().byteSize());
      }
    }
  VELOCYPACK_GLOBAL_EXCEPTION_CATCH
}
