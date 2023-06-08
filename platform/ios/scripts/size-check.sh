#!/bin/bash

if [ $# -eq 0 ]; then
    echo "Give development team as argument, eg. '484D8S5C7M'"
    exit 1
fi

RED=$'\033[1;31m'
GREEN=$'\033[1;32m'
BOLD=$'\033[1m'
NORMAL=$'\033[0m'

# The development team for signing the archive is taken as argument
DEVELOPMENT_TEAM=$1

# The configuration for the build
BUILD_CONFIGURATIONS=("Release" "MinSizeRel")

# Initial lib size in bytes
INITIAL_LIB_SIZES=(5504352 4249952)

# The path to the Xcode workspace
XCODE_WORKSPACE_PATH=platform/ios/ios.xcworkspace

# The scheme name for the build
SCHEME_NAME="iosapp"

# The archive filename
ARCHIVE_NAME="${SCHEME_NAME}.xcarchive"

if [[ ! -d $XCODE_WORKSPACE_PATH ]]; then
  echo "${RED}iOS workspace not found: $XCODE_WORKSPACE_PATH${NORMAL}"
  exit 2
fi

# Clean up any old builds
make clean
if [ ! $? -eq 0 ]; then
  echo "${RED}Failed cleaning up the build folder${NORMAL}"
  exit 3
fi

# Set up the Xcode workspace
make iproj CI=1
if [ ! $? -eq 0 ]; then
  echo "${RED}Failed setting up the Xcode workspace${NORMAL}"
  exit 4
fi

TOTAL_TESTS=${#BUILD_CONFIGURATIONS[@]}
PASSED_TESTS=0
FAILED_TESTS=0

for ((i=0; i<${TOTAL_TESTS}; i++)); do
  BUILD_CONFIGURATION=${BUILD_CONFIGURATIONS[i]}
  INITIAL_LIB_SIZE=${INITIAL_LIB_SIZES[i]}

  echo -e "\n** ${BOLD}Building size test $((i+1)) on configuration: ${BUILD_CONFIGURATION}, initial size was ${INITIAL_LIB_SIZE} bytes${NORMAL} **\n"

  # The output directory for the archive
  OUTPUT_DIR="build/ios/archive/${BUILD_CONFIGURATION}"
  
  # Build the app
  xcodebuild archive -quiet -workspace "${XCODE_WORKSPACE_PATH}" -scheme "${SCHEME_NAME}" -configuration "${BUILD_CONFIGURATION}" -archivePath "${OUTPUT_DIR}/${ARCHIVE_NAME}" DEVELOPMENT_TEAM="${DEVELOPMENT_TEAM}"
  if [ ! $? -eq 0 ]; then
    echo "${RED}Failed to compile the scheme: ${SCHEME_NAME}${NORMAL}"
    exit 5
  fi

  # The path to the maplibre dynamic lib
  LIB_PATH="${OUTPUT_DIR}/${ARCHIVE_NAME}/Products/Applications/MapLibre Native.app/Frameworks/Mapbox.framework/Mapbox"
  if [[ ! -f "$LIB_PATH" ]]; then
      echo "${RED}MapLibre dynamic lib not found: $LIB_PATH${NORMAL}"
      exit 6
  fi

  # Current lib size in bytes
  CURRENT_LIB_SIZE=$(wc -c "${LIB_PATH}" | awk '{print $1}')
  if [ -z $CURRENT_LIB_SIZE ]; then
    echo "${RED}Failed to compute the size of MapLibre: ${LIB_PATH}${NORMAL}"
    exit 7
  fi

  if (($CURRENT_LIB_SIZE > $INITIAL_LIB_SIZE)); then
    FAILED_TESTS=$((FAILED_TESTS+1))
    echo -e "${RED}\n[FAILED] MapLibre size is ${CURRENT_LIB_SIZE} bytes. This is larger than the initial size of ${INITIAL_LIB_SIZE} bytes.\n${NORMAL}"
  else
    PASSED_TESTS=$((PASSED_TESTS+1))
    if (($CURRENT_LIB_SIZE == $INITIAL_LIB_SIZE)); then
      echo -e "${GREEN}\n[PASSED] MapLibre size is ${CURRENT_LIB_SIZE} bytes. This is equal with the initial size of ${INITIAL_LIB_SIZE} bytes.\n${NORMAL}"
    else 
      echo -e "${GREEN}\n[PASSED] MapLibre size is ${CURRENT_LIB_SIZE} bytes. This is smaller than the initial size of ${INITIAL_LIB_SIZE} bytes.\n${NORMAL}"
    fi
  fi
done

echo -e "${GREEN}PASSED TESTS: ${PASSED_TESTS} of ${TOTAL_TESTS}${NORMAL}"
echo -e "${RED}FAILED TESTS: ${FAILED_TESTS} of ${TOTAL_TESTS}${NORMAL}"