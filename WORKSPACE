workspace(name = "Maplibre")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "build_bazel_rules_apple",
    sha256 = "2a0a35c9f72a0b0ac9238ecb081b0da4bb3e9739e25d2a910cc6b4c4425c01be",
    url = "https://github.com/bazelbuild/rules_apple/releases/download/2.4.1/rules_apple.2.4.1.tar.gz",
)

http_archive(
    name = "rules_xcodeproj",
    sha256 = "24aed0bc6cf4132f62e08d4b81656107ea8d7804c9f7f57cfb9acbd0992ba75b",
    url = "https://github.com/MobileNativeFoundation/rules_xcodeproj/releases/download/1.8.1/release.tar.gz",
)

load(
    "@build_bazel_rules_apple//apple:repositories.bzl",
    "apple_rules_dependencies",
)

apple_rules_dependencies()

load(
    "@build_bazel_rules_swift//swift:repositories.bzl",
    "swift_rules_dependencies",
)

swift_rules_dependencies()

load(
    "@build_bazel_rules_swift//swift:extras.bzl",
    "swift_rules_extra_dependencies",
)

swift_rules_extra_dependencies()

load(
    "@build_bazel_apple_support//lib:repositories.bzl",
    "apple_support_dependencies",
)

apple_support_dependencies()

load(
    "@rules_xcodeproj//xcodeproj:repositories.bzl",
    "xcodeproj_rules_dependencies",
)

xcodeproj_rules_dependencies()

load(
    "@build_bazel_rules_apple//apple:apple.bzl",
    "provisioning_profile_repository",
)

provisioning_profile_repository(
    name = "local_provisioning_profiles",
)
