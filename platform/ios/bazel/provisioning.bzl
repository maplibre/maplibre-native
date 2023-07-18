load("@build_bazel_rules_apple//apple:apple.bzl", "local_provisioning_profile")
load("@rules_xcodeproj//xcodeproj:defs.bzl", "xcode_provisioning_profile")
load("config.bzl", "APPLE_MOBILE_PROVISIONING_PROFILE_NAME", "APPLE_MOBILE_PROVISIONING_PROFILE_TEAM_ID", "BUNDLE_ID_PREFIX")

def configure_device_profiles():
    local_provisioning_profile(
        name = "provisioning_profile",
        profile_name = APPLE_MOBILE_PROVISIONING_PROFILE_NAME,
        team_id = APPLE_MOBILE_PROVISIONING_PROFILE_TEAM_ID,
    )

    xcode_provisioning_profile(
        name = "xcode_profile",
        managed_by_xcode = True,
        provisioning_profile = ":provisioning_profile",
        visibility = ["//visibility:public"],
    )
