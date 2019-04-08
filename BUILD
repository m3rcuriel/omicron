load("@org_tensorflow//tensorflow:tensorflow.bzl", "tf_cc_binary")

cc_library(
	name = "chess",
	srcs = ["chess.cc"],
	hdrs = ["chess.h"],
)

cc_library(
    name = "uct",
    srcs = ["uct.cc"],
    hdrs = ["uct.h"],
    deps = [":chess"],
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
        "@org_tensorflow//tensorflow/cc:cc_ops",
        "@org_tensorflow//tensorflow/cc:client_session",
        "@org_tensorflow//tensorflow/core:tensorflow",
        "@pybind11//:pybind11",
    ],
    copts = ["-fexceptions"],
    linkshared = 1,
    visibility = ["//visibility:public"],
)
