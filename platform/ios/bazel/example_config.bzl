# Find your team ID inside a .mobileprovision file or in your keychain (Apple development: your@email -> Get Info -> Organizational Unit)
APPLE_MOBILE_PROVISIONING_PROFILE_TEAM_ID = "TC45MCF93C"
APPLE_MOBILE_PROVISIONING_PROFILE_NAME = "iOS Team Provisioning Profile: *"
BUNDLE_ID_PREFIX = "com.firstnamelastname"

# Change the BUILD_MODE to "xcode" below if you experience problems with
# not finding a provisioning profile when building.
# https://github.com/MobileNativeFoundation/rules_xcodeproj/blob/main/docs/bazel.md#xcodeproj-build_mode
BUILD_MODE = "bazel"

# Enter your API key for MapTiler here, if you have one.
API_KEY = "0000000000"
# Semantic version number to include in plist files.
SEM_VER = "0.0.0"