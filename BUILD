load("@org_tensorflow//tensorflow:tensorflow.bzl", "tf_cc_binary")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "util",
    srcs = [],
    hdrs = ["util.h"],
)

cc_library(
	name = "chess",
	srcs = ["chess.cc"],
	hdrs = ["chess.h"],
    deps = [":util"],
)

cc_library(
    name = "uct",
    srcs = ["uct.cc", "particle_filter.cc"],
    hdrs = ["uct.h", "particle_filter.h"],
    deps = [":chess", ":util"],
)

cc_test(
    name = "uct_test",
    srcs = ["uct_test.cc"],
	deps = [
		":uct",
		"@googletest//:gtest",
		"@googletest//:gtest_main",
	],
)

cc_test(
	name = "chess_test",
	srcs = ["chess_test.cc"],
	deps = [
		":chess",
		"@googletest//:gtest",
		"@googletest//:gtest_main",
	],
)

tf_cc_binary(
    name = "chess_agent.so",
    srcs = ["chess_agent.cc"],
    deps = [
        "@pybind11//:pybind11",
        ":chess",
    ],
    copts = ["-fexceptions"],
    linkshared = 1,
    visibility = ["//visibility:public"],
)
