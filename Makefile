export BUILDTYPE ?= Debug
export IS_LOCAL_DEVELOPMENT ?= true
export TARGET_BRANCH ?= master

CMAKE ?= cmake


ifeq ($(BUILDTYPE), Release)
else ifeq ($(BUILDTYPE), RelWithDebInfo)
else ifeq ($(BUILDTYPE), Sanitize)
else ifeq ($(BUILDTYPE), Debug)
else
  $(error BUILDTYPE must be Debug, Sanitize, Release or RelWithDebInfo)
endif

buildtype := $(shell echo "$(BUILDTYPE)" | tr "[A-Z]" "[a-z]")

ifeq ($(shell uname -s), Darwin)
  HOST_PLATFORM = macos
  HOST_PLATFORM_VERSION = $(shell uname -m)
  export NINJA = platform/macos/ninja
  export JOBS ?= $(shell sysctl -n hw.ncpu)
else ifeq ($(shell uname -s), Linux)
  HOST_PLATFORM = linux
  HOST_PLATFORM_VERSION = $(shell uname -m)
  export NINJA = platform/linux/ninja
  export JOBS ?= $(shell grep --count processor /proc/cpuinfo)
else
  $(error Cannot determine host platform)
endif

ifeq ($(MASON_PLATFORM),)
  BUILD_PLATFORM = $(HOST_PLATFORM)
else
  BUILD_PLATFORM = $(MASON_PLATFORM)
endif

ifeq ($(MASON_PLATFORM_VERSION),)
  BUILD_PLATFORM_VERSION = $(HOST_PLATFORM_VERSION)
else
  BUILD_PLATFORM_VERSION = $(MASON_PLATFORM_VERSION)
endif

ifeq ($(MASON_PLATFORM),macos)
	MASON_PLATFORM=osx
endif

ifeq ($(V), 1)
  export XCPRETTY
  NINJA_ARGS ?= -v
else
  export XCPRETTY ?= | tee '$(shell pwd)/build/xcodebuild-$(shell date +"%Y-%m-%d_%H%M%S").log' | xcpretty
  NINJA_ARGS ?=
endif

.PHONY: default
default: test

BUILD_DEPS += ./vendor/mapbox-gl-native/Makefile
BUILD_DEPS += ./vendor/mapbox-gl-native/CMakeLists.txt

BUILD_DOCS ?= true

#### iOS targets ##############################################################

ifeq ($(HOST_PLATFORM), macos)

IOS_OUTPUT_PATH = build/ios
IOS_PROJ_PATH = '$(IOS_OUTPUT_PATH)/Mapbox GL Native.xcodeproj'
IOS_WORK_PATH = platform/ios/ios.xcworkspace
IOS_USER_DATA_PATH = $(IOS_WORK_PATH)/xcuserdata/$(USER).xcuserdatad

IOS_XCODEBUILD_SIM = xcodebuild \
	ARCHS=x86_64 ONLY_ACTIVE_ARCH=YES \
	-derivedDataPath $(IOS_OUTPUT_PATH) \
	-configuration $(BUILDTYPE) -sdk iphonesimulator \
	-workspace $(IOS_WORK_PATH) \
	-jobs $(JOBS)

ifneq ($(MORE_SIMULATORS),)
	IOS_LATEST = true
	IOS_11 = true
	IOS_10 = true
	IOS_9 = true
endif

ifdef IOS_LATEST
	IOS_XCODEBUILD_SIM += \
	-destination 'platform=iOS Simulator,OS=latest,name=iPhone 8' \
	-destination 'platform=iOS Simulator,OS=latest,name=iPhone Xs Max' \
	-destination 'platform=iOS Simulator,OS=latest,name=iPhone Xr' \
	-destination 'platform=iOS Simulator,OS=latest,name=iPad Pro (11-inch)'
endif

ifdef IOS_11
	IOS_XCODEBUILD_SIM += \
	-destination 'platform=iOS Simulator,OS=11.4,name=iPhone 7' \
	-destination 'platform=iOS Simulator,OS=11.4,name=iPhone X' \
	-destination 'platform=iOS Simulator,OS=11.4,name=iPad (5th generation)'
endif

ifdef IOS_10
	IOS_XCODEBUILD_SIM += \
	-destination 'platform=iOS Simulator,OS=10.3.1,name=iPhone SE' \
	-destination 'platform=iOS Simulator,OS=10.3.1,name=iPhone 7 Plus' \
	-destination 'platform=iOS Simulator,OS=10.3.1,name=iPad Pro (9.7-inch)'
endif

ifdef IOS_9
	IOS_XCODEBUILD_SIM += \
	-destination 'platform=iOS Simulator,OS=9.3,name=iPhone 6s Plus' \
	-destination 'platform=iOS Simulator,OS=9.3,name=iPhone 6s' \
	-destination 'platform=iOS Simulator,OS=9.3,name=iPad Air 2'
endif

# If IOS_XCODEBUILD_SIM does not contain a simulator destination, add the default.
ifeq (, $(findstring destination, $(IOS_XCODEBUILD_SIM)))
	IOS_XCODEBUILD_SIM += \
	-destination 'platform=iOS Simulator,OS=latest,name=iPhone 8'
else
	IOS_XCODEBUILD_SIM += -parallel-testing-enabled YES
endif

ifneq ($(ONLY_TESTING),)
	IOS_XCODEBUILD_SIM += -only-testing:$(ONLY_TESTING)
endif

ifneq ($(SKIP_TESTING),)
	IOS_XCODEBUILD_SIM += -skip-testing:$(SKIP_TESTING)
endif

ifneq ($(CI),)
	IOS_XCODEBUILD_SIM += -xcconfig platform/darwin/ci.xcconfig
endif

$(IOS_PROJ_PATH): $(IOS_USER_DATA_PATH)/WorkspaceSettings.xcsettings $(BUILD_DEPS)
	mkdir -p $(IOS_OUTPUT_PATH)
	(cd $(IOS_OUTPUT_PATH) && $(CMAKE) -G Xcode ../../vendor/mapbox-gl-native/next \
		-DCMAKE_SYSTEM_NAME=iOS )

$(IOS_USER_DATA_PATH)/WorkspaceSettings.xcsettings: platform/ios/WorkspaceSettings.xcsettings
	mkdir -p "$(IOS_USER_DATA_PATH)"
	cp platform/ios/WorkspaceSettings.xcsettings "$@"

.PHONY: ios
ios: $(IOS_PROJ_PATH)
	set -o pipefail && $(IOS_XCODEBUILD_SIM) -scheme 'CI' build $(XCPRETTY)

.PHONY: iproj
iproj: $(IOS_PROJ_PATH)
	xed $(IOS_WORK_PATH)

.PHONY: ios-lint
ios-lint: ios-pod-lint
	find platform/ios/framework -type f -name '*.plist' | xargs plutil -lint
	find platform/ios/app -type f -name '*.plist' | xargs plutil -lint

.PHONY: ios-pod-lint
ios-pod-lint:
	./platform/ios/scripts/lint-podspecs.js

.PHONY: ios-test
ios-test: $(IOS_PROJ_PATH)
	set -o pipefail && $(IOS_XCODEBUILD_SIM) -scheme 'CI' test $(XCPRETTY)

.PHONY: ios-integration-test
ios-integration-test: $(IOS_PROJ_PATH)
	set -o pipefail && $(IOS_XCODEBUILD_SIM) -scheme 'Integration Test Harness' test $(XCPRETTY)

.PHONY: ios-sanitize
ios-sanitize: $(IOS_PROJ_PATH)
	set -o pipefail && $(IOS_XCODEBUILD_SIM) -scheme 'CI' -enableThreadSanitizer YES -enableUndefinedBehaviorSanitizer YES test $(XCPRETTY)

.PHONY: ios-sanitize-address
ios-sanitize-address: $(IOS_PROJ_PATH)
	set -o pipefail && $(IOS_XCODEBUILD_SIM) -scheme 'CI' -enableAddressSanitizer YES test $(XCPRETTY)

.PHONY: ios-static-analyzer
ios-static-analyzer: $(IOS_PROJ_PATH)
	set -o pipefail && $(IOS_XCODEBUILD_SIM) analyze -scheme 'CI' test $(XCPRETTY)

.PHONY: ios-install-simulators
ios-install-simulators:
	xcversion simulators --install="iOS 11.4" || true
	xcversion simulators --install="iOS 10.3.1" || true
	xcversion simulators --install="iOS 9.3" || true

.PHONY: ipackage
ipackage: ipackage*
ipackage%:
	@echo make ipackage is deprecated — use make iframework.

.PHONY: iframework
iframework: $(IOS_PROJ_PATH)
	FORMAT=$(FORMAT) BUILD_DEVICE=$(BUILD_DEVICE) SYMBOLS=$(SYMBOLS) BUILD_DOCS=$(BUILD_DOCS) \
	./platform/ios/scripts/package.sh

.PHONY: ideploy
ideploy:
	caffeinate -i ./platform/ios/scripts/deploy-packages.sh

.PHONY: idocument
idocument:
	OUTPUT=$(OUTPUT) ./platform/ios/scripts/document.sh

.PHONY: darwin-style-code
darwin-style-code:
	node platform/darwin/scripts/generate-style-code.js
	node platform/darwin/scripts/update-examples.js
style-code: darwin-style-code

.PHONY: darwin-update-examples
darwin-update-examples:
	node platform/darwin/scripts/update-examples.js

.PHONY: darwin-check-public-symbols
darwin-check-public-symbols:
	node platform/darwin/scripts/check-public-symbols.js macOS iOS

endif

#### Miscellaneous targets #####################################################

.PHONY: style-code
style-code:
	node scripts/generate-style-code.js
	node scripts/generate-shaders.js

.PHONY: codestyle
codestyle:
	scripts/codestyle.sh

.PHONY: clean
clean:
	-rm -rf ./build \
	        ./lib/*.node 

.PHONY: distclean
distclean: clean
	-rm -rf ./mason_packages
	-rm -rf ./node_modules
