
cc_library(
    name = "velocypack",
    srcs = glob(["src/*.cpp", "src/*.h"]),
    hdrs = glob(["include/velocypack/*.h"]),
    visibility = ["//visibility:public"],
    strip_include_prefix = "include"
)


cc_binary(
    name = "json-to-vpack",
    srcs = ["tools/json-to-vpack.cpp"],
    deps = [":velocypack"]
)

cc_binary(
    name = "vpack-to-json",
    srcs = ["tools/vpack-to-json.cpp"],
    deps = [":velocypack"]
)

cc_binary(
    name = "vpack-validate",
    srcs = ["tools/vpack-validate.cpp"],
    deps = [":velocypack"]
)


cc_test(
    name = "velocypack-test",
    srcs = glob(["tests/*.cpp"]) + ["tests/tests-common.h"],
    data = glob(["tests/jsonSample/*.json"]),
    deps = [
        "//tests/googletest:gtest",
        "//:velocypack",
    ],
)
