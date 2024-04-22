load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("//platform/darwin:bazel/darwin_config_repository_rule.bzl", "darwin_config")

http_archive(
    name = "glfw",
    build_file = "@//vendor:glfw.BUILD",
    integrity = "sha256-tewASycS/Qjohh3CcUKPBId1IAot9xnM9XUUO6dJo+k=",
    strip_prefix = "glfw-3.4",
    url = "https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip",
)

new_local_repository(
    name = "libuv",
    build_file = "@//vendor:libuv.BUILD",
    path = "/opt/homebrew/opt/libuv",
)

darwin_config(
    name = "darwin_config",
)
