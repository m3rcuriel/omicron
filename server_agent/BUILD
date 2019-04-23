load("@build_stack_rules_proto//python:python_grpc_library.bzl", "python_grpc_library")
load("@recon_deps//:requirements.bzl", "requirement")

proto_library(
    name = "agent_proto",
    srcs = ["agent.proto"],
    deps = ["@com_google_protobuf//:empty_proto"],
    visibility = ["//visibility:public"],
)

python_grpc_library(
    name = "agent_python_proto",
    deps = [
        ":agent_proto",
    ],
)

sh_binary(
    name = "convert_to_bazel",
    srcs = [
        "convert_to_bazel.sh"
    ],
)

genrule(
    name = "bazel_server_agent",
    srcs = [
        "server_agent.py"
    ],
    outs = [
        "server_agent_bz.py"
    ],
    tools = [":convert_to_bazel"],
    cmd = "cat $(location server_agent.py) | ./$(location :convert_to_bazel) > $(location server_agent_bz.py)",
)

py_library(
    name = "server_agent",
    srcs = [
        "server_agent_bz.py",
    ],
    deps = [
        ":agent_python_proto",
        "//recon:player",
        requirement("python-chess"),
    ],
    visibility = ["//visibility:public"],
)
