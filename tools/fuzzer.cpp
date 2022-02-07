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

#include <atomic>
#include <charconv>
#include <cmath>
#include <iostream>
#include <mutex>
#include <random>
#include <string>
#include <unordered_set>
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
  static constexpr uint32_t maxDepth = 10;
  static constexpr uint32_t utf81ByteFirstLowerBound = 0x00;
  static constexpr uint32_t utf81ByteFirstUpperBound = 0x7F;
  static constexpr uint32_t utf82BytesFirstLowerBound = 0xC2;
  static constexpr uint32_t utf82BytesFirstUpperBound = 0xDF;
  static constexpr uint32_t utf83BytesFirstLowerBound = 0xE0;
  static constexpr uint32_t utf83BytesFirstUpperBound = 0xEF;
  static constexpr uint32_t utf84BytesFirstLowerBound = 0xF0;
  static constexpr uint32_t utf84BytesFirstUpperBound = 0xF3; 
  static constexpr uint32_t utf8CommonLowerBound = 0x80;
  static constexpr uint32_t utf8CommonUpperBound = 0xBF;
  static constexpr uint32_t minUtf8RandStringLength = 1;
  static constexpr uint32_t maxUtf8RandStringLength = 1000;
  static constexpr uint32_t objNumMembers = 10;
  static constexpr uint32_t arrayNumMembers = 10;
};

struct RandomGenerator {
  RandomGenerator(uint64_t seed) : mt64(seed) {}

  std::mt19937_64 mt64; // this is 64 bits for generating uint64_t in ADD_UINT64 for velocypack
};

Slice nullSlice(Slice::nullSlice());

static std::mutex mtx;

static void usage(char* argv[]) {
  std::cout << "Usage: " << argv[0] << " [format] [iterations] [threads] seed"
            << std::endl;
  std::cout << "This program creates <iterations> random VPack or JSON structures and validates them."
            << std::endl;
  std::cout << "The paralelization is supplied by <threads>."
            << std::endl;
  std::cout << "The seed value supplied by <seed> is used as seed for random generation."
            << std::endl;
  std::cout << "Available format options are:" << std::endl;
  std::cout << " --vpack       create VPack."
            << std::endl;
  std::cout << " --json        create JSON."
            << std::endl;
  std::cout << "For iterations:" << std::endl;
  std::cout << " --iterations <number>  number of iterations (number > 0). Default: 1"
            << std::endl;
  std::cout << "For threads:" << std::endl;
  std::cout << " --threads <number>  number of threads (number > 0). Default: 1"
            << std::endl;
  std::cout << "For providing a seed for random generation:" << std::endl;
  std::cout
      << " --s <number> number that will be used as seed for random generation (number >= 0). Default: random_device"
      << std::endl;
}

static inline bool isOption(char const* arg, char const* expected) {
  return (strcmp(arg, expected) == 0);
}

uint32_t randWithinRange(uint32_t min, uint32_t max, RandomGenerator& randomGenerator) {
  return min + (randomGenerator.mt64() % (max - min));
}

void appendRandUtf8Char(RandomGenerator& randomGenerator, std::string& utf8Str) {
  using limits = KnownLimitValues;
  int numBytes = randWithinRange(1, 4, randomGenerator);
  switch (numBytes) {
    case 1: {
      utf8Str.push_back(
          randWithinRange(limits::utf81ByteFirstLowerBound, limits::utf81ByteFirstUpperBound, randomGenerator));
      break;
    }
    case 2: {
      utf8Str.push_back(randWithinRange(limits::utf82BytesFirstLowerBound, limits::utf82BytesFirstUpperBound,
                                                 randomGenerator));
      utf8Str.push_back(
          randWithinRange(limits::utf8CommonLowerBound, limits::utf8CommonUpperBound, randomGenerator));
      break;
    }
    case 3: {
      utf8Str.push_back(randWithinRange(limits::utf83BytesFirstLowerBound, limits::utf83BytesFirstUpperBound,
                                                 randomGenerator));
      for (uint32_t i = 0; i < 2; ++i) {
        utf8Str.push_back(
            randWithinRange(limits::utf8CommonLowerBound, limits::utf8CommonUpperBound, randomGenerator));
      }
      break;
    }
    case 4: {
      utf8Str.push_back(randWithinRange(limits::utf84BytesFirstLowerBound, limits::utf84BytesFirstUpperBound,
                                                 randomGenerator));
      for (uint32_t i = 0; i < 3; ++i) {
        utf8Str.push_back(
            randWithinRange(limits::utf8CommonLowerBound, limits::utf8CommonUpperBound, randomGenerator));
      }
      break;
    }
    default:
      VELOCYPACK_ASSERT(false);
  }
}

static void generateUtf8String(RandomGenerator& randomGenerator, std::string& utf8Str) {
  using limits = KnownLimitValues;
  uint32_t length = limits::minUtf8RandStringLength +
                    (randomGenerator.mt64() % (limits::maxUtf8RandStringLength - limits::minUtf8RandStringLength));
  for (uint32_t i = 0; i < length; ++i) {
    appendRandUtf8Char(randomGenerator, utf8Str);
  }
}

template <typename Format>
static void generateVelocypack(Builder& builder, uint32_t depth, RandomGenerator& randomGenerator) {
  using limits = KnownLimitValues;
  RandomBuilderAdditions maxValue = RandomBuilderAdditions::ADD_DOUBLE;

  if constexpr (std::is_same_v<Format, VPackFormat>) {
    maxValue = RandomBuilderAdditions::ADD_MAX_VPACK_VALUE;
  }

  std::unordered_set<std::string> keys;
  while (true) {
    RandomBuilderAdditions randomBuilderAdds = static_cast<RandomBuilderAdditions>(randomGenerator.mt64() %
                                                                                   maxValue);
    if (depth > limits::maxDepth && randomBuilderAdds <= ADD_OBJECT) {
      continue;
    }
    switch (randomBuilderAdds) {
      case ADD_ARRAY: {
        builder.openArray(randomGenerator.mt64() % 2 ? true : false);
        uint32_t numMembers = randomGenerator.mt64() % limits::arrayNumMembers;
        for (uint32_t i = 0; i < numMembers; ++i) {
          generateVelocypack<Format>(builder, depth + 1, randomGenerator);
        }
        builder.close();
        break;
      }
      case ADD_OBJECT: {
        builder.openObject(randomGenerator.mt64() % 2 ? true : false);
        uint32_t numMembers = randomGenerator.mt64() % limits::objNumMembers;
        for (uint32_t i = 0; i < numMembers; ++i) {
          std::string key;
          do {
            key = "";
            generateUtf8String(randomGenerator, key);
          } while(keys.find(key) != keys.end());
          keys.insert(key);
          builder.add(Value(key + std::to_string(i)));
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
          uint32_t uintValue = static_cast<uint32_t>(randomGenerator.mt64());
          memcpy(&doubleValue, &uintValue, sizeof(uintValue));
        } while (!std::isfinite(doubleValue));
        builder.add(Value(doubleValue));
        break;
      }
      case ADD_UTC_DATE:
        builder.add(Value(randomGenerator.mt64(), ValueType::UTCDate));
        break;
      case ADD_BINARY: {
        std::string binaries;
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
      break;
      default:
        VELOCYPACK_ASSERT(false);
    }
    break;
  }
}

static bool isParamValid(char const* p, uint64_t& value) {
  auto result = std::from_chars(p, p + strlen(p), value);
  if (result.ec != std::errc()) {
    std::cerr << "Error: wrong parameter type: " << p << std::endl;
    return false;
  }
  return true;
}

int main(int argc, char* argv[]) {
  VELOCYPACK_GLOBAL_EXCEPTION_TRY
    bool isTypeAssigned = false;
    uint32_t numIterations = 1;
    uint32_t numThreads = 1;
    bool isJSON = false;
    std::random_device rd;
    uint64_t seed = rd();

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
      } else if (isOption(p, "--iterations") && (++i < argc)) {
        char const *p = argv[i];
        uint64_t value = 0;
        if (!isParamValid(p, value) || !value) {
          isFailure = true;
        } else {
          numIterations = value;
        }
      } else if (isOption(p, "--threads") && (++i < argc)) {
        char const *p = argv[i];
        uint64_t value = 0;
        if (!isParamValid(p, value) || !value) {
          isFailure = true;
        } else {
          numThreads = value;
        }
      } else if (isOption(p, "--s") && (++i < argc)) {
        char const *p = argv[i];
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

    std::cout << "Initial seed is " << seed << std::endl;

    uint32_t itsPerThread = numIterations / numThreads;
    uint32_t leftoverIts = numIterations % numThreads;
    std::atomic<bool> stopThreads{false};

    auto threadCallback = [&stopThreads]<typename Format>(uint32_t iterations, Format, uint64_t seed) {
      Options options;
      options.validateUtf8Strings = true;
      options.checkAttributeUniqueness = true;
      options.binaryAsHex = true;
      options.datesAsIntegers = true;
      Builder builder(&options);
      try {
        RandomGenerator randomGenerator(seed);
        {
          std::lock_guard<std::mutex> lock(mtx);
          std::cout << "Initial thread seed is " << seed << std::endl;
        }
        Parser parser(&options);
        Validator validator(&options);
        while (iterations-- > 0 && !stopThreads.load(std::memory_order_relaxed)) {
          builder.clear();
          if constexpr (std::is_same_v<Format, JSONFormat>) {
            generateVelocypack<JSONFormat>(builder, 0, randomGenerator);
            parser.parse(builder.slice().toJson(&options));
          } else {
            generateVelocypack<VPackFormat>(builder, 0, randomGenerator);
            validator.validate(builder.slice().start(), builder.slice().byteSize());
          }
        }
      } catch (std::exception const& e) {
        std::lock_guard<std::mutex> lock(mtx);
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
      for (uint32_t i = 0; i < numThreads; ++i) {
        uint32_t iterations = itsPerThread;
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
