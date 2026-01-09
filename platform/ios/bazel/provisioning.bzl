load("@darwin_config//:config.bzl", "APPLE_MOBILE_PROVISIONING_PROFILE_NAME", "APPLE_MOBILE_PROVISIONING_PROFILE_TEAM_ID")
load("@rules_apple//apple:apple.bzl", "local_provisioning_profile")
load("@rules_xcodeproj//xcodeproj:defs.bzl", "xcode_provisioning_profile")

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
