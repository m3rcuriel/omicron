workspace(name = "omicron")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "new_git_repository", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

git_repository(
    name = "org_tensorflow",
    branch = "master",
    remote = "https://github.com/tensorflow/tensorflow",
)

http_archive(
    name = "io_bazel_rules_closure",
    sha256 = "ddce3b3a3909f99b28b25071c40b7fec7e2e1d1d1a4b2e933f3082aa99517105",
    strip_prefix = "rules_closure-316e6133888bfc39fb860a4f1a31cfcbae485aef",
    urls = [
        "http://mirror.tensorflow.org/github.com/bazelbuild/rules_closure/archive/316e6133888bfc39fb860a4f1a31cfcbae485aef.tar.gz",
        "https://github.com/bazelbuild/rules_closure/archive/316e6133888bfc39fb860a4f1a31cfcbae485aef.tar.gz",  # 2019-03-21
    ],
)

http_archive(
    name = "bazel_skylib",
    sha256 = "2c62d8cd4ab1e65c08647eb4afe38f51591f43f7f0885e7769832fa137633dcb",
    strip_prefix = "bazel-skylib-0.7.0",
    urls = ["https://github.com/bazelbuild/bazel-skylib/archive/0.7.0.tar.gz"],
)

load("@org_tensorflow//tensorflow:workspace.bzl", "tf_workspace")

tf_workspace()

git_repository(
    name = "googletest",
    tag = "release-1.8.1",
    remote = "https://github.com/google/googletest",
)

new_git_repository(
    name = "pybind11",
    tag = "v2.2.4",
    remote = "https://github.com/pybind/pybind11",
    build_file_content = """
cc_library(
    name = "pybind11",
    hdrs = glob(["include/**/*.h"]),
    includes = ["include"],
    deps = ["@python_local//:python_config"],
    visibility = ["//visibility:public"],
)
"""
)

new_local_repository(
    name = "python_local",
    path = "/usr/include/python3.7m",
    build_file = "BUILD.python_local",
)

local_repository(
    name = "bazel_latex",
    path = "third_party/bazel-latex",
#    sha256 = "b4dd9ae76c570b328be30cdc5ea7045a61ecd55e4e6e2e433fb3bb959be2a44b",
#    strip_prefix = "bazel-latex-0.16",
#    url = "https://github.com/ProdriveTechnologies/bazel-latex/archive/v0.16.tar.gz",
)

load("@bazel_latex//:repositories.bzl", "latex_repositories")

latex_repositories()

http_archive(
    name = "build_stack_rules_proto",
    urls = ["https://github.com/stackb/rules_proto/archive/609362dd9b08110b7a95bfa26b5e3aac3cd06905.tar.gz"],
    sha256 = "9c9fc051189dd87bd643cf69e82e3b08de03114fc03155de784ba60bd0cef4b6",
    strip_prefix = "rules_proto-609362dd9b08110b7a95bfa26b5e3aac3cd06905",
)

load("@build_stack_rules_proto//python:deps.bzl", "python_grpc_library")

python_grpc_library()

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

grpc_deps()

load("@io_bazel_rules_python//python:pip.bzl", "pip_import", "pip_repositories")

pip_repositories()

pip_import(
    name = "protobuf_py_deps",
    requirements = "@build_stack_rules_proto//python/requirements:protobuf.txt",
)

load("@protobuf_py_deps//:requirements.bzl", protobuf_pip_install = "pip_install")

protobuf_pip_install()

pip_import(
    name = "grpc_py_deps",
    requirements = "@build_stack_rules_proto//python:requirements.txt",
)

load("@grpc_py_deps//:requirements.bzl", grpc_pip_install="pip_install")

grpc_pip_install()

pip_import(
    name = "recon_deps",
    requirements = "//recon:requirements.txt",
)

load("@recon_deps//:requirements.bzl", recon_pip_install="pip_install")

recon_pip_install()
