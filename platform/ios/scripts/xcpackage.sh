#!/usr/bin/env bash

set -e
set -o pipefail
set -u

NAME=Mapbox
OUTPUT=build/ios/pkg
DERIVED_DATA=build/ios
PRODUCTS=${DERIVED_DATA}
LOG_PATH=build/xcodebuild-$(date +"%Y-%m-%d_%H%M%S").log

BUILD_DOCS=${BUILD_DOCS:-true}
SYMBOLS=${SYMBOLS:-YES}

BUILDTYPE=${BUILDTYPE:-Debug}
if [[ ${SYMBOLS} == YES && ${BUILDTYPE} == Release ]]; then
    BUILDTYPE='RelWithDebInfo'
fi

FORMAT=${FORMAT:-dynamic}
if [[ ${FORMAT} != "dynamic" ]]; then
    echo "Error: only dynamic FORMAT is supported in xcpackage build."
    exit 1
fi

function step { >&2 echo -e "\033[1m\033[36m* $@\033[0m"; }
function finish { >&2 echo -en "\033[0m"; }
trap finish EXIT

IOS_SDK_VERSION=`xcrun --sdk iphoneos --show-sdk-version`
IOS_SIM_SDK_VERSION=`xcrun --sdk iphonesimulator --show-sdk-version`

step "Configuring ${FORMAT} framework for iphoneos ${IOS_SDK_VERSION} and iphonesimulator ${IOS_SIM_SDK_VERSION} (symbols: ${SYMBOLS}, buildtype: ${BUILDTYPE})"

xcodebuild -version

rm -rf ${OUTPUT}
BINOUT="${OUTPUT}"/dynamic
mkdir -p ${BINOUT}    

step "Recording library version…"
VERSION="${OUTPUT}"/version.txt
echo -n "https://github.com/maplibre/maplibre-gl-native/commit/" > ${VERSION}
HASH=`git log | head -1 | awk '{ print $2 }' | cut -c 1-10` && true
echo -n "maplibre-gl-native-ios "
echo ${HASH}
echo ${HASH} >> ${VERSION}

PROJ_VERSION=$(git rev-list --count HEAD)
SEM_VERSION=$( git describe --tags --match=ios-v*.*.* --abbrev=0 | sed 's/^ios-v//' )
SHORT_VERSION=${SEM_VERSION%-*}

step "Building targets (build ${PROJ_VERSION}, version ${SEM_VERSION})"

SCHEME='dynamic'

CI_XCCONFIG=''
if [[ ! -z "${CI:=}" ]]; then
    xcconfig='platform/darwin/ci.xcconfig'
    echo "CI environment, using ${xcconfig}"
    CI_XCCONFIG="-xcconfig ./${xcconfig}"
fi

step "Building ${FORMAT} archive for iOS Simulator using ${SCHEME} scheme -> ${BINOUT}/${NAME}-iphonesimulator.xcarchive"
xcodebuild \
    archive \
    CURRENT_SEMANTIC_VERSION=${SEM_VERSION} \
    CURRENT_COMMIT_HASH=${HASH} \
    SKIP_INSTALL=NO \
    BUILD_LIBRARY_FOR_DISTRIBUTION=YES \
    -workspace ./platform/ios/ios.xcworkspace \
    -archivePath ${BINOUT}/${NAME}-iphonesimulator.xcarchive \
    -scheme ${SCHEME} \
    -configuration ${BUILDTYPE} \
    -sdk iphonesimulator \
    -jobs ${JOBS} | tee ${LOG_PATH} | xcpretty

step "Building ${FORMAT} archive for iOS devices using ${SCHEME} scheme  -> ${BINOUT}/${NAME}-iphoneos.xcarchive"
xcodebuild \
    archive \
    CURRENT_SEMANTIC_VERSION=${SEM_VERSION} \
    CURRENT_COMMIT_HASH=${HASH} \
    SKIP_INSTALL=NO \
    BUILD_LIBRARY_FOR_DISTRIBUTION=YES \
    -workspace ./platform/ios/ios.xcworkspace \
    -archivePath ${BINOUT}/${NAME}-iphoneos.xcarchive \
    -scheme ${SCHEME} \
    -configuration ${BUILDTYPE} \
    -sdk iphoneos \
    -jobs ${JOBS} | tee ${LOG_PATH} | xcpretty

LIBS=(Mapbox.a)

function realpath {
    [[ $1 = /* ]] && echo "$1" || echo "$PWD/${1#./}"
}

function addFramework {
    local archive=$1
    local frameworkName=$2
    local includeDebugSymbols=$3

    local frameworkPath="${archive}/Products/Library/Frameworks/${frameworkName}.framework"
    BUILD_ARGS="$BUILD_ARGS -framework $frameworkPath"
    
    if [[ $includeDebugSymbols == true ]]; then
        SYMBOL_PATH=$(realpath "${archive}/dSYMs/${frameworkName}.framework.dSYM")
        BUILD_ARGS="$BUILD_ARGS -debug-symbols ${SYMBOL_PATH}"
    fi
}

step "Creating ${NAME} xcframework"

INCLUDE_DEBUG_SYMBOLS=false
if [[ $BUILDTYPE='RelWithDebInfo' || $BUILDTYPE='Debug' ]]; then
    INCLUDE_DEBUG_SYMBOLS=true
fi


# Mapbox mobile
BUILD_ARGS=""
addFramework "${BINOUT}/${NAME}-iphoneos.xcarchive" ${NAME} ${INCLUDE_DEBUG_SYMBOLS}
addFramework "${BINOUT}/${NAME}-iphonesimulator.xcarchive" ${NAME} ${INCLUDE_DEBUG_SYMBOLS}
BUILD_ARGS="$BUILD_ARGS -output ${BINOUT}/${NAME}.xcframework"
echo "Creating ${NAME}.xcframework with args: $BUILD_ARGS"
xcodebuild -create-xcframework $BUILD_ARGS
echo "${NAME}.xcframework created"

step "Copying library resources…"
cp -pv LICENSE.md ${OUTPUT}
sed -n -e '/^## /,$p' platform/ios/CHANGELOG.md > "${OUTPUT}/CHANGELOG.md"

rm -rf /tmp/mbgl
mkdir -p /tmp/mbgl/
README=/tmp/mbgl/README.md
cp platform/ios/docs/pod-README.md "${README}"
sed -i '' -e '/{{STATIC}}/,/{{\/STATIC}}/d' "${README}"
sed -i '' \
    -e '/{{DYNAMIC}}/d' -e '/{{\/DYNAMIC}}/d' \
    -e '/{{STATIC}}/d' -e '/{{\/STATIC}}/d' \
    "${README}"
cp ${README} "${OUTPUT}"

if [ ${BUILD_DOCS} == true ]; then
    step "Generating API documentation for ${BUILDTYPE} Build…"
    make idocument OUTPUT="${OUTPUT}/documentation"
fi
