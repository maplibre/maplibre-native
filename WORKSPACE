workspace(name = "Maplibre")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "build_bazel_rules_apple",
    sha256 = "8ac4c7997d863f3c4347ba996e831b5ec8f7af885ee8d4fe36f1c3c8f0092b2c",
    url = "https://github.com/bazelbuild/rules_apple/releases/download/2.5.0/rules_apple.2.5.0.tar.gz",
)

http_archive(
    name = "rules_xcodeproj",
    sha256 = "f5c1f4bea9f00732ef9d54d333d9819d574de7020dbd9d081074232b93c10b2c",
    url = "https://github.com/MobileNativeFoundation/rules_xcodeproj/releases/download/1.13.0/release.tar.gz",
)

load(
    "@rules_xcodeproj//xcodeproj:repositories.bzl",
    "xcodeproj_rules_dependencies",
)

xcodeproj_rules_dependencies()

load("@bazel_features//:deps.bzl", "bazel_features_deps")

bazel_features_deps()

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
    "@build_bazel_rules_apple//apple:apple.bzl",
    "provisioning_profile_repository"
)

provisioning_profile_repository(
    name = "local_provisioning_profiles"
)
