load("@org_tensorflow//tensorflow:tensorflow.bzl", "tf_cc_binary")

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
