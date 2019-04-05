load("@bazel_latex//:latex.bzl", "latex_document")

latex_document(
    name = "report",
    srcs = glob([
        "chapters/*.tex",
    ]) + [
        "@bazel_latex//packages:graphicx",
        "@bazel_latex//packages:ieeetran",
    ],
    main = "report.tex",
)
