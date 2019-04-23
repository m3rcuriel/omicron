load("@build_stack_rules_proto//cpp:cpp_grpc_library.bzl", "cpp_grpc_library")

cpp_grpc_library(
    name = "agent_cpp_proto",
    deps = [
        "//server_agent:agent_proto",
    ],
)

cc_binary(
    name = "remote_agent",
    srcs = [
        "remote_agent.cc",
    ],
    deps = [
        ":agent_cpp_proto",
        "//:chess",
        "//:chess_agent",
    ],
)
