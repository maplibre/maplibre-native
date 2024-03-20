#!/bin/bash

set -eo pipefail

flavor="legacy" # Renderer build flavor: legacy, drawable, split

while [[ $# -gt 0 ]]; do
   case $1 in
   --flavor)
      shift
      flavor="$1"
      shift
      ;;
   -*|--*)
      echo "Unknown option $1"
      exit 1
      ;;
   esac
done

# Generate the Xcode project
# Example invocation: bazel-xcodeproj.sh flavor split
bazel run //platform/ios:xcodeproj --@rules_xcodeproj//xcodeproj:extra_common_flags="--//:renderer=$flavor"
