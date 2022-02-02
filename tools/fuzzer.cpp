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
#include <unordered_map>
#include <fstream>

#include <velocypack/Builder.h>
#include "velocypack/vpack.h"
#include "velocypack/velocypack-exception-macros.h"
#include <ctime>
#include <random>
#include <filesystem>

namespace fs = std::filesystem;

using namespace arangodb::velocypack;

static std::unique_ptr<AttributeTranslator> translator(new AttributeTranslator);
enum class Format {
    VPACK, JSON
};

static void usage(char *argv[]) {
#ifdef __linux__
    std::cout << "Usage: " << argv[0] << " [OPTIONS] [OUT_DIR]"
              << std::endl;
#else
    std::cout << "Usage: " << argv[0] << " [OPTIONS] [OUT_DIR]"
              << std::endl;
#endif
    std::cout << "This program creates either VPack files or JSON files to use as input for other tools and stores them"
              << " in the output directory provided."
              << std::endl;
    std::cout << "Available options are:" << std::endl;
    std::cout
            << " --vpack       create VPack file."
            << std::endl;
    std::cout << " --json       create VPack file."
              << std::endl;
}

static inline bool isOption(char const *arg, char const *expected) {
    return (strcmp(arg, expected) == 0);
}

int main(int argc, char *argv[]) {
    VELOCYPACK_GLOBAL_EXCEPTION_TRY
     char const *outDirName = nullptr;
        bool allowFlags = true;
        bool isTypeAssigned = false;
        Format format;

        int i = 1;
        while (i < argc) {
            char const *p = argv[i];
            if (allowFlags && isOption(p, "--help")) {
                usage(argv);
                return EXIT_SUCCESS;
            } else if (allowFlags && isOption(p, "--")) {
                allowFlags = false;
            } else if (allowFlags && isOption(p, "--vpack") && !isTypeAssigned) {
                isTypeAssigned = true;
                format = Format::VPACK;
            } else if (allowFlags && isOption(p, "--json") && !isTypeAssigned) {
                isTypeAssigned = true;
                format = Format::JSON;
            } else if (outDirName == nullptr) {
                outDirName = p;
            } else {
                usage(argv);
                return EXIT_FAILURE;
            }
            ++i;
        }

        if (fs::exists(outDirName)) {
            fs::remove_all(outDirName);
        }
        fs::create_directory(outDirName);

        if (format == Format::VPACK) {
            std::mt19937 mt(time(nullptr));
            VPackBuilder result;

            result.openObject();
            result.add("value", VPackValue(mt()));
            result.add("messages", VPackValue(VPackValueType::Array));
            for (size_t i = 1; i < 11; ++i) {
                result.openObject();
                result.add("id", VPackValue(mt()));
                result.add("message", VPackValue("test" + i));
                result.close();
            }
            result.close();
            fs::path dir(outDirName);
            fs::path file("foo.vpack");
            fs::path full_path = dir / file;
            std::ofstream ofs(full_path, std::ofstream::out);

            if (!ofs.is_open()) {
                std::cerr << "Cannot write outfile '" << full_path << "'" << std::endl;
                return EXIT_FAILURE;
            }
            result.close();
            uint8_t const *start = result.start();
            ofs.write(reinterpret_cast<char const *>(start), result.size());
            ofs.close();
        }

    VELOCYPACK_GLOBAL_EXCEPTION_CATCH
}
