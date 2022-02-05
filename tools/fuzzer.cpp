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

#include <charconv>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <thread>

#include "velocypack/vpack.h"
#include "velocypack/velocypack-exception-macros.h"

using namespace arangodb::velocypack;

struct VPackFormat {};

struct JSONFormat {};

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

struct KnownLimitValues {
  static constexpr size_t maxDepth = 10;
  static constexpr size_t utf81ByteFirstLowerBound = 0x00;
  static constexpr size_t utf81ByteFirstUpperBound = 0x1F;
  static constexpr size_t utf82BytesFirstLowerBound = 0xc0;
  static constexpr size_t utf82BytesFirstUpperBound = 0xDF;
  static constexpr size_t utf83BytesFirstLowerBound = 0xE0;
  static constexpr size_t utf83BytesFirstUpperBound = 0xEF;
  static constexpr size_t utf84BytesFirstLowerBound = 0xF0;
  static constexpr size_t utf84BytesFirstUpperBound = 0xF7;
  static constexpr size_t utf8CommonLowerBound = 0x80;
  static constexpr size_t utf8CommonUpperBound = 0xBF;
  static constexpr size_t minRandStringLength = 1;
  static constexpr size_t maxRandStringLength = 1000;

};

struct RandomGenerator {
  RandomGenerator() : mt64{rd()} {}
  RandomGenerator(uint64_t seed) : mt64(seed) {}

  std::random_device rd;
  std::mt19937_64 mt64;
};

Slice nullSlice(Slice::nullSlice());

static void usage(char* argv[]) {
  std::cout << "Usage: " << argv[0] << " [OPTIONS] [ITERATIONS] [THREADS] SEED"
            << std::endl;
  std::cout << "This program creates <iterations> random VPack or JSON structures and validates them."
            << std::endl;
  std::cout << "The paralelization is supplied by <threads>."
            << std::endl;
  std::cout << "The seed value supplied by <seed> is used as seed for random generation."
            << std::endl;
  std::cout << "Available options are:" << std::endl;
  std::cout << " --vpack       create VPack."
            << std::endl;
  std::cout << " --json        create JSON."
            << std::endl;
  std::cout << " --i <number>  number of iterations (number > 0). Default: 1"
            << std::endl;
  std::cout << " --t <number>  number of threads (number > 0). Default: 1"
            << std::endl;
  std::cout
      << " --s <number> number that will be used as seed for random generation (number >= 0). Default: random_device"
      << std::endl;
}

static inline bool isOption(char const* arg, char const* expected) {
  return (strcmp(arg, expected) == 0);
}

uint8_t randWithinRange(size_t min, size_t max, RandomGenerator& randomGenerator) {
  std::uniform_int_distribution<> distr(min, max);
  return distr(randomGenerator.mt64);
}

void appendRandUtf8Char(RandomGenerator& randomGenerator, std::string& utf8Buffer) {
  constexpr KnownLimitValues limits;
  int numBytes = randWithinRange(1, 4, randomGenerator);
  switch (numBytes) {
    case 1: {
      char c[2];
      c[0] = randWithinRange(limits.utf81ByteFirstLowerBound, limits.utf81ByteFirstUpperBound, randomGenerator);
      c[1] = '\0';
      utf8Buffer.append(c);
      break;
    }
    case 2: {
      char c[3];
      c[0] = randWithinRange(limits.utf82BytesFirstLowerBound, limits.utf82BytesFirstUpperBound,
                             randomGenerator);
      c[1] = randWithinRange(limits.utf8CommonLowerBound, limits.utf8CommonUpperBound, randomGenerator);
      c[2] = '\0';
      utf8Buffer.append(c);
      break;
    }
    case 3: {
      char c[4];
      c[0] = randWithinRange(limits.utf83BytesFirstLowerBound, limits.utf83BytesFirstUpperBound,
                             randomGenerator);
      c[1] = randWithinRange(limits.utf8CommonLowerBound, limits.utf8CommonUpperBound, randomGenerator);
      c[2] = randWithinRange(limits.utf8CommonLowerBound, limits.utf8CommonUpperBound, randomGenerator);
      c[3] = '\0';
      utf8Buffer.append(c);
      break;
    }
    case 4: {
      char c[5];
      c[0] = randWithinRange(limits.utf84BytesFirstLowerBound, limits.utf84BytesFirstUpperBound,
                             randomGenerator);
      c[1] = randWithinRange(limits.utf8CommonLowerBound, limits.utf8CommonUpperBound, randomGenerator);
      c[2] = randWithinRange(limits.utf8CommonLowerBound, limits.utf8CommonUpperBound, randomGenerator);
      c[3] = randWithinRange(limits.utf8CommonLowerBound, limits.utf8CommonUpperBound, randomGenerator);
      c[4] = '\0';
      utf8Buffer.append(c);
    }
    default:
      break;
  }
}

static void generateUtf8String(RandomGenerator& randomGenerator, std::string& utf8Str) {
  KnownLimitValues limits;
  std::uniform_int_distribution<> distr(limits.minRandStringLength, limits.maxRandStringLength);
  size_t length = distr(randomGenerator.mt64);
  for (size_t i = 0; i < length; ++i) {
    appendRandUtf8Char(randomGenerator, utf8Str);
  }
}

template <typename Format>
static void generateVelocypack(Builder& builder, size_t depth, RandomGenerator& randomGenerator) {
  constexpr KnownLimitValues limits;
  RandomBuilderAdditions maxValue = RandomBuilderAdditions::ADD_DOUBLE;

  if constexpr (std::is_same_v<Format, VPackFormat>) {
    maxValue = RandomBuilderAdditions::ADD_MAX_VPACK_VALUE;
  }

  while (true) {
    RandomBuilderAdditions randomBuilderAdds = static_cast<RandomBuilderAdditions>(randomGenerator.mt64() %
                                                                                   maxValue);
    if (depth > limits.maxDepth && randomBuilderAdds <= ADD_OBJECT) {
      continue;
    }
    switch (randomBuilderAdds) {
      case ADD_ARRAY: {
        builder.openArray(randomGenerator.mt64() % 2 ? true : false);
        size_t numMembers = randomGenerator.mt64() % 10;
        for (size_t i = 0; i < numMembers; ++i) {
          generateVelocypack<Format>(builder, depth + 1, randomGenerator);
        }
        builder.close();
        break;
      }
      case ADD_OBJECT: {
        builder.openObject(randomGenerator.mt64() % 2 ? true : false);
        size_t numMembers = randomGenerator.mt64() % 10;
        for (size_t i = 0; i < numMembers; ++i) {
          std::string key;
          generateUtf8String(randomGenerator, key);
          //std::string key = "test" + std::to_string(i);
          builder.add(Value(key));
          generateVelocypack<Format>(builder, depth + 1, randomGenerator);
        }
        builder.close();
        break;
      }
      case ADD_BOOLEAN:
        builder.add(Value(randomGenerator.mt64() % 2 ? true : false));
        break;
      case ADD_STRING: {
        std::string key;
        generateUtf8String(randomGenerator, key);
        builder.add(Value(key));
        break;
      }
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
        double doubleValue;
        do {
          uint64_t uintValue = randomGenerator.mt64();
          memcpy(&doubleValue, &uintValue, sizeof(uintValue));
        } while (!std::isfinite(doubleValue));
        builder.add(Value(doubleValue));
        break;
      }
      case ADD_UTC_DATE:
        builder.add(Value(randomGenerator.mt64(), ValueType::UTCDate));
        break;
      case ADD_BINARY: {
        std::string binaries; // stuff
        generateUtf8String(randomGenerator, binaries);
        builder.add(ValuePair(binaries.data(), binaries.size(), ValueType::Binary));
        break;
      }
      case ADD_EXTERNAL: {
        builder.add(Value(static_cast<void const *>(&nullSlice), ValueType::External));
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

static bool isParamValid(char const* p, uint64_t& value) {
  auto result = std::from_chars(p, p + strlen(p), value);
  if (result.ec != std::errc()) {
    std::cerr << "Error: wrong parameter type." << std::endl;
    return false;
  }
  return true;
}

int main(int argc, char* argv[]) {
  VELOCYPACK_GLOBAL_EXCEPTION_TRY
    bool isTypeAssigned = false;
    size_t numIterations = 1;
    size_t numThreads = 1;
    bool isJSON = false;
    std::random_device rd;
    std::mt19937_64 mt64{rd()};
    uint64_t seed = mt64();

    int i = 1;
    while (i < argc) {
      bool isFailure = false;
      char const *p = argv[i];
      if (isOption(p, "--help")) {
        usage(argv);
        return EXIT_SUCCESS;
      } else if (isOption(p, "--vpack") && !isTypeAssigned) {
        isTypeAssigned = true;
        isJSON = false;
      } else if (isOption(p, "--json") && !isTypeAssigned) {
        isTypeAssigned = true;
        isJSON = true;
      } else if (isOption(p, "--i")) {
        char const *p = argv[++i];
        uint64_t value = 0;
        if (!isParamValid(p, value) || !value) {
          isFailure = true;
        } else {
          numIterations = value;
        }
      } else if (isOption(p, "--t")) {
        char const *p = argv[++i];
        uint64_t value = 0;
        if (!isParamValid(p, value) || !value) {
          isFailure = true;
        } else {
          numThreads = value;
        }
      } else if (isOption(p, "--s")) {
        char const *p = argv[++i];
        uint64_t value = 0;
        if (!isParamValid(p, value)) {
          isFailure = true;
        } else {
          seed = value;
        }
      } else {
        isFailure = true;
      }
      if (isFailure) {
        usage(argv);
        return EXIT_FAILURE;
      }
      ++i;
    }

    // validator options = uniqueness, utf-8

    std::cout << "Initial seed is " << seed << std::endl;

    size_t itsPerThread = numIterations / numThreads;
    size_t leftoverIts = numIterations % numThreads;
    std::atomic<bool> stopThreads{false};

    auto threadCallback = [&stopThreads] <typename Format> (size_t iterations, Format, uint64_t seed) {
      Builder builder;
      try {
        Options options;
        options.validateUtf8Strings = true;
        options.checkAttributeUniqueness = true;
        RandomGenerator randomGenerator(seed);
        std::cout << "Initial thread seed is " << seed << std::endl;
        Parser parser(&options);
        Validator validator(&options);
        while (iterations-- > 0 && !stopThreads.load(std::memory_order_relaxed)) {
          builder.clear();
          if constexpr (std::is_same_v<Format, JSONFormat>) {
            generateVelocypack<JSONFormat>(builder, 0, randomGenerator);
            parser.parse(builder.slice().toJson());
          } else {
            generateVelocypack<VPackFormat>(builder, 0, randomGenerator);
            validator.validate(builder.slice().start(), builder.slice().byteSize());
          }
        }
      } catch (std::exception const& e) {
        std::cerr << "Program encountered exception on thread execution: " << e.what() << " in slice ";
        if constexpr (std::is_same_v<Format, JSONFormat>) {
          std::cerr << builder.slice().toJson() << std::endl;
        } else {
          std::cerr << HexDump(builder.slice()) << std::endl;
        }
        return;
      }
    };

    std::vector<std::thread> threads;
    threads.reserve(numThreads);
    auto joinThreads = [&threads]() {
      for (auto &t: threads) {
        t.join();
      }
    };

    try {
      for (size_t i = 0; i < numThreads; ++i) {
        size_t iterations = itsPerThread;
        if (i == numThreads - 1) {
          iterations += leftoverIts;
        }
        if (isJSON) {
          threads.emplace_back(threadCallback, iterations, JSONFormat{}, seed + i);
        } else {
          threads.emplace_back(threadCallback, iterations, VPackFormat{}, seed + i);
        }
      }
      joinThreads();
    } catch (std::exception const &ex) {
      stopThreads.store(true);
      joinThreads();
    }

  VELOCYPACK_GLOBAL_EXCEPTION_CATCH
}
