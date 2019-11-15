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

#### Android targets ###########################################################

MBGL_ANDROID_ABIS  = arm-v7;armeabi-v7a
MBGL_ANDROID_ABIS += arm-v8;arm64-v8a
MBGL_ANDROID_ABIS += x86;x86
MBGL_ANDROID_ABIS += x86-64;x86_64

MBGL_ANDROID_LOCAL_WORK_DIR = /data/local/tmp/core-tests
MBGL_ANDROID_LOCAL_BENCHMARK_DIR = /data/local/tmp/benchmark
MBGL_ANDROID_LIBDIR = lib$(if $(filter arm-v8 x86-64,$1),64)
MBGL_ANDROID_DALVIKVM = dalvikvm$(if $(filter arm-v8 x86-64,$1),64,32)
MBGL_ANDROID_APK_SUFFIX = $(if $(filter Release,$(BUILDTYPE)),release,debug)
MBGL_ANDROID_CORE_TEST_DIR = MapboxGLAndroidSDK/.externalNativeBuild/cmake/$(buildtype)/$2/core-tests
MBGL_ANDROID_BENCHMARK_DIR = MapboxGLAndroidSDK/.externalNativeBuild/cmake/$(buildtype)/$2/benchmark
MBGL_ANDROID_STL ?= c++_static
MBGL_ANDROID_GRADLE = ./gradlew --parallel --max-workers=$(JOBS) -Pmapbox.buildtype=$(buildtype) -Pmapbox.stl=$(MBGL_ANDROID_STL)

# Lists all devices, and extracts the identifiers, then obtains the ABI for every one.
# Some devices return \r\n, so we'll have to remove the carriage return before concatenating.
MBGL_ANDROID_ACTIVE_ARCHS = $(shell adb devices | sed '1d;/^\*/d;s/[[:space:]].*//' | xargs -n 1 -I DEV `type -P adb` -s DEV shell getprop ro.product.cpu.abi | tr -d '\r')

# Generate code based on the style specification
.PHONY: android-style-code
android-style-code:
	node scripts/generate-style-code.js
style-code: android-style-code

# Configuration file for running CMake from Gradle within Android Studio.
gradle/configuration.gradle:
	@printf "ext {\n    node = '`command -v node || command -v nodejs`'\n    npm = '`command -v npm`'\n    ccache = '`command -v ccache`'\n}" > $@

define ANDROID_RULES
# $1 = arm-v7 (short arch)
# $2 = armeabi-v7a (internal arch)

.PHONY: android-test-lib-$1
android-test-lib-$1: gradle/configuration.gradle
	$(MBGL_ANDROID_GRADLE) -Pmapbox.abis=$2 -Pmapbox.with_test=true :MapboxGLAndroidSDKTestApp:assemble$(BUILDTYPE)

.PHONY: android-benchmark-$1
android-benchmark-$1: gradle/configuration.gradle
	$(MBGL_ANDROID_GRADLE) -Pmapbox.abis=$2 -Pmapbox.with_benchmark=true :MapboxGLAndroidSDKTestApp:assemble$(BUILDTYPE)

# Build SDK for for specified abi
.PHONY: android-lib-$1
android-lib-$1: gradle/configuration.gradle
	$(MBGL_ANDROID_GRADLE) -Pmapbox.abis=$2 :MapboxGLAndroidSDK:assemble$(BUILDTYPE)

# Build test app and SDK for for specified abi
.PHONY: android-$1
android-$1: gradle/configuration.gradle
	$(MBGL_ANDROID_GRADLE) -Pmapbox.abis=$2 :MapboxGLAndroidSDKTestApp:assemble$(BUILDTYPE)

# Build the core test for specified abi
.PHONY: android-core-test-$1
android-core-test-$1: android-test-lib-$1
	# Compile main sources and extract the classes (using the test app to get all transitive dependencies in one place)
	mkdir -p $(MBGL_ANDROID_CORE_TEST_DIR)
	unzip -o MapboxGLAndroidSDKTestApp/build/outputs/apk/$(buildtype)/MapboxGLAndroidSDKTestApp-$(MBGL_ANDROID_APK_SUFFIX).apk classes.dex -d $(MBGL_ANDROID_CORE_TEST_DIR)

run-android-core-test-$1-%: android-core-test-$1
	# Ensure clean state on the device
	adb shell "rm -Rf $(MBGL_ANDROID_LOCAL_WORK_DIR) && mkdir -p $(MBGL_ANDROID_LOCAL_WORK_DIR)/test && mkdir -p $(MBGL_ANDROID_LOCAL_WORK_DIR)/mapbox-gl-js/src/style-spec/reference"

	# Push all needed files to the device
	adb push $(MBGL_ANDROID_CORE_TEST_DIR)/classes.dex $(MBGL_ANDROID_LOCAL_WORK_DIR) > /dev/null 2>&1
	adb push MapboxGLAndroidSDK/build/intermediates/intermediate-jars/$(buildtype)/jni/$2/libmapbox-gl.so $(MBGL_ANDROID_LOCAL_WORK_DIR) > /dev/null 2>&1
	adb push test/fixtures $(MBGL_ANDROID_LOCAL_WORK_DIR)/test > /dev/null 2>&1
	adb push mapbox-gl-js/src/style-spec/reference/v8.json $(MBGL_ANDROID_LOCAL_WORK_DIR)/mapbox-gl-js/src/style-spec/reference > /dev/null 2>&1
	adb push MapboxGLAndroidSDK/build/intermediates/cmake/$(buildtype)/obj/$2/mbgl-test $(MBGL_ANDROID_LOCAL_WORK_DIR) > /dev/null 2>&1

# Create gtest filter for skipped tests.
	$(eval SKIPPED_TESTS := -$(shell sed -n '/#\|^$$/!p' tests/skipped.txt | sed ':a;$!N;s/\n/:/g;ta'))

	# Kick off the tests
	adb shell "export LD_LIBRARY_PATH=$(MBGL_ANDROID_LOCAL_WORK_DIR) && cd $(MBGL_ANDROID_LOCAL_WORK_DIR) && chmod +x mbgl-test && ./mbgl-test --class_path=$(MBGL_ANDROID_LOCAL_WORK_DIR)/classes.dex --gtest_filter=$$*:$(SKIPPED_TESTS)"

	# Gather the results and unpack them
	adb shell "cd $(MBGL_ANDROID_LOCAL_WORK_DIR) && tar -cvzf results.tgz test/fixtures/*  > /dev/null 2>&1"
	adb pull $(MBGL_ANDROID_LOCAL_WORK_DIR)/results.tgz $(MBGL_ANDROID_CORE_TEST_DIR)/ > /dev/null 2>&1
	rm -rf $(MBGL_ANDROID_CORE_TEST_DIR)/results && mkdir -p $(MBGL_ANDROID_CORE_TEST_DIR)/results
	tar -xzf $(MBGL_ANDROID_CORE_TEST_DIR)/results.tgz --strip-components=2 -C $(MBGL_ANDROID_CORE_TEST_DIR)/results

# Run the core test for specified abi
.PHONY: run-android-core-test-$1
run-android-core-test-$1: run-android-core-test-$1-*

# Run benchmarks for specified abi
.PHONY: run-android-benchmark-$1
run-android-benchmark-$1: run-android-benchmark-$1-*

run-android-benchmark-$1-%: android-benchmark-$1
	mkdir -p $(MBGL_ANDROID_BENCHMARK_DIR)
	unzip -o MapboxGLAndroidSDKTestApp/build/outputs/apk/$(buildtype)/MapboxGLAndroidSDKTestApp-$(MBGL_ANDROID_APK_SUFFIX).apk classes.dex -d $(MBGL_ANDROID_BENCHMARK_DIR)

	# Delete old test folder and create new one
	adb shell "rm -Rf $(MBGL_ANDROID_LOCAL_BENCHMARK_DIR) && mkdir -p $(MBGL_ANDROID_LOCAL_BENCHMARK_DIR)/benchmark && mkdir -p $(MBGL_ANDROID_LOCAL_BENCHMARK_DIR)/test"

	# Push compiled java sources, test data and executable to device
	adb push $(MBGL_ANDROID_BENCHMARK_DIR)/classes.dex $(MBGL_ANDROID_LOCAL_BENCHMARK_DIR) > /dev/null 2>&1
	adb push MapboxGLAndroidSDK/build/intermediates/intermediate-jars/$(buildtype)/jni/$2/libmapbox-gl.so $(MBGL_ANDROID_LOCAL_BENCHMARK_DIR) > /dev/null 2>&1
	adb push benchmark/fixtures $(MBGL_ANDROID_LOCAL_BENCHMARK_DIR)/benchmark > /dev/null 2>&1
	adb push test/fixtures $(MBGL_ANDROID_LOCAL_BENCHMARK_DIR)/test > /dev/null 2>&1
	adb push MapboxGLAndroidSDK/build/intermediates/cmake/$(buildtype)/obj/$2/mbgl-benchmark $(MBGL_ANDROID_LOCAL_BENCHMARK_DIR) > /dev/null 2>&1

	# Run benchmark. Number of benchmark iterations can be set by run-android-benchmark-N parameter.
	adb shell "export LD_LIBRARY_PATH=$(MBGL_ANDROID_LOCAL_BENCHMARK_DIR) && cd $(MBGL_ANDROID_LOCAL_BENCHMARK_DIR) && chmod +x mbgl-benchmark && ./mbgl-benchmark --class_path=$(MBGL_ANDROID_LOCAL_BENCHMARK_DIR)/classes.dex --benchmark_repetitions=$$* --benchmark_format=json --benchmark_out=results.json"

	# Pull results.json from the device
	rm -rf $(MBGL_ANDROID_BENCHMARK_DIR)/results && mkdir -p $(MBGL_ANDROID_BENCHMARK_DIR)/results
	adb pull $(MBGL_ANDROID_LOCAL_BENCHMARK_DIR)/results.json $(MBGL_ANDROID_BENCHMARK_DIR)/results > /dev/null 2>&1

# Run the test app on connected android device with specified abi
.PHONY: run-android-$1
run-android-$1: gradle/configuration.gradle
	-adb uninstall com.mapbox.mapboxsdk.testapp 2> /dev/null
	$(MBGL_ANDROID_GRADLE) -Pmapbox.abis=$2 :MapboxGLAndroidSDKTestApp:install$(BUILDTYPE) && adb shell am start -n com.mapbox.mapboxsdk.testapp/.activity.FeatureOverviewActivity

# Build test app instrumentation tests apk and test app apk for specified abi
.PHONY: android-ui-test-$1
android-ui-test-$1: gradle/configuration.gradle
	$(MBGL_ANDROID_GRADLE) -Pmapbox.abis=$2 :MapboxGLAndroidSDKTestApp:assembleDebug :MapboxGLAndroidSDKTestApp:assembleAndroidTest

# Run test app instrumentation tests on a connected android device or emulator with specified abi
.PHONY: run-android-ui-test-$1
run-android-ui-test-$1: gradle/configuration.gradle
	-adb uninstall com.mapbox.mapboxsdk.testapp 2> /dev/null
	$(MBGL_ANDROID_GRADLE) -Pmapbox.abis=$2 :MapboxGLAndroidSDKTestApp:connectedAndroidTest

# Run Java Instrumentation tests on a connected android device or emulator with specified abi and test filter
run-android-ui-test-$1-%: gradle/configuration.gradle
	-adb uninstall com.mapbox.mapboxsdk.testapp 2> /dev/null
	$(MBGL_ANDROID_GRADLE) -Pmapbox.abis=$2 :MapboxGLAndroidSDKTestApp:connectedAndroidTest -Pandroid.testInstrumentationRunnerArguments.class="$$*"

# Symbolicate native stack trace with the specified abi
.PHONY: android-ndk-stack-$1
android-ndk-stack-$1: gradle/configuration.gradle
	adb logcat | ndk-stack -sym MapboxGLAndroidSDK/build/intermediates/cmake/debug/obj/$2/

# Run render tests with pixelmatch
.PHONY: run-android-render-test-$1
run-android-render-test-$1: $(BUILD_DEPS) gradle/configuration.gradle
	-adb uninstall com.mapbox.mapboxsdk.testapp 2> /dev/null
	# delete old test results
	rm -rf build/render-test/mapbox/
  # copy test definitions & ignore file to test app assets folder, clear old ones first
	rm -rf MapboxGLAndroidSDKTestApp/src/main/assets/integration
	cp -r mapbox-gl-js/test/integration MapboxGLAndroidSDKTestApp/src/main/assets
	cp platform/node/test/ignores.json MapboxGLAndroidSDKTestApp/src/main/assets/integration/ignores.json
	# run RenderTest.java to generate static map images
	$(MBGL_ANDROID_GRADLE) -Pmapbox.abis=$2 :MapboxGLAndroidSDKTestApp:connectedAndroidTest -Pandroid.testInstrumentationRunnerArguments.class="com.mapbox.mapboxsdk.testapp.render.RenderTest"
	# pull generated images from the device
	adb pull "`adb shell 'printenv EXTERNAL_STORAGE' | tr -d '\r'`/mapbox/render" build/render-test
	# copy expected result and run pixelmatch
	python scripts/run-render-test.py
	# remove test definitions from assets
	rm -rf MapboxGLAndroidSDKTestApp/src/main/assets/integration

endef

# Explodes the arguments into individual variables
define ANDROID_RULES_INVOKER
$(call ANDROID_RULES,$(word 1,$1),$(word 2,$1))
endef

$(foreach abi,$(MBGL_ANDROID_ABIS),$(eval $(call ANDROID_RULES_INVOKER,$(subst ;, ,$(abi)))))

# Build the Android SDK and test app with abi set to arm-v7
.PHONY: android
android: android-arm-v7

# Build the Android SDK with abi set to arm-v7
.PHONY: android-lib
android-lib: android-lib-arm-v7

# Run the test app on connected android device with abi set to arm-v7
.PHONY: run-android
run-android: run-android-arm-v7

# Run Java Instrumentation tests on a connected android device or emulator with abi set to arm-v7
.PHONY: run-android-ui-test
run-android-ui-test: run-android-ui-test-arm-v7
run-android-ui-test-%: run-android-ui-test-arm-v7-%

# Run Java Unit tests on the JVM of the development machine executing this
.PHONY: run-android-unit-test
run-android-unit-test: gradle/configuration.gradle
	$(MBGL_ANDROID_GRADLE) -Pmapbox.abis=none :MapboxGLAndroidSDK:testDebugUnitTest --info
run-android-unit-test-%: gradle/configuration.gradle
	$(MBGL_ANDROID_GRADLE) -Pmapbox.abis=none :MapboxGLAndroidSDK:testDebugUnitTest  --info --tests "$*"

# Run unit test and build a coverage report from .exec file generated by unit tests and .ec file generated by instrumentation tests
.PHONY: android-create-jacoco-report
android-create-jacoco-report: gradle/configuration.gradle
	$(MBGL_ANDROID_GRADLE) -Pmapbox.abis=none :MapboxGLAndroidSDK:jacocoTestReport

# Parse merged jacoco report and send it to S3
.PHONY: android-parse-and-send-jacoco-report
android-parse-and-send-jacoco-report:
	python scripts/parse-jacoco-report.py

# Builds a release package of the Android SDK
.PHONY: apackage
apackage: gradle/configuration.gradle
	make android-lib-arm-v7 && make android-lib-arm-v8 && make android-lib-x86 && make android-lib-x86-64
	$(MBGL_ANDROID_GRADLE) -Pmapbox.abis=all assemble$(BUILDTYPE)

# Build test app instrumentation tests apk and test app apk for all abi's
.PHONY: android-ui-test
android-ui-test: gradle/configuration.gradle
	$(MBGL_ANDROID_GRADLE) -Pmapbox.abis=all :MapboxGLAndroidSDKTestApp:assembleDebug :MapboxGLAndroidSDKTestApp:assembleAndroidTest

#Run instrumentations tests on MicroSoft App Center
.PHONY: run-android-test-app-center
run-android-test-app-center:
	appcenter test run espresso --app "mapboxcn-outlook.com/MapsSdk" --devices "mapboxcn-outlook.com/china" --app-path MapboxGLAndroidSDKTestApp/build/outputs/apk/debug/MapboxGLAndroidSDKTestApp-debug.apk  --test-series "master" --locale "en_US" --build-dir MapboxGLAndroidSDKTestApp/build/outputs/apk/androidTest/debug

# Uploads the compiled Android SDK to Bintray
.PHONY: run-android-upload-to-bintray
run-android-upload-to-bintray: gradle/configuration.gradle
	$(MBGL_ANDROID_GRADLE) -Pmapbox.abis=all :MapboxGLAndroidSDK:bintrayUpload

# Uploads the compiled Android SDK SNAPSHOT to oss.jfrog.org
.PHONY: run-android-upload-to-artifactory
run-android-upload-to-artifactory: gradle/configuration.gradle
	$(MBGL_ANDROID_GRADLE) -Pmapbox.abis=all :MapboxGLAndroidSDK:artifactoryPublish

# Dump system graphics information for the test app
.PHONY: android-gfxinfo
android-gfxinfo:
	adb shell dumpsys gfxinfo com.mapbox.mapboxsdk.testapp reset

# Generates Activity sanity tests
.PHONY: test-code-android
test-code-android:
	node scripts/generate-test-code.js

# Runs checkstyle and lint on the java code
.PHONY: android-check
android-check : android-ktlint android-checkstyle android-lint-sdk android-lint-test-app run-android-nitpick

# Runs checkstyle on the java code
.PHONY: android-checkstyle
android-checkstyle: gradle/configuration.gradle
	$(MBGL_ANDROID_GRADLE) -Pmapbox.abis=none :MapboxGLAndroidSDK:checkstyle :MapboxGLAndroidSDKTestApp:checkstyle

# Runs checkstyle on the kotlin code
.PHONY: android-ktlint
android-ktlint: gradle/configuration.gradle
	$(MBGL_ANDROID_GRADLE) -Pmapbox.abis=none ktlint

# Runs lint on the Android SDK java code
.PHONY: android-lint-sdk
android-lint-sdk: gradle/configuration.gradle
	$(MBGL_ANDROID_GRADLE) -Pmapbox.abis=none :MapboxGLAndroidSDK:lint

# Runs lint on the Android test app java code
.PHONY: android-lint-test-app
android-lint-test-app: gradle/configuration.gradle
	$(MBGL_ANDROID_GRADLE) -Pmapbox.abis=none :MapboxGLAndroidSDKTestApp:lint

# Generates javadoc from the Android SDK
.PHONY: android-javadoc
android-javadoc: gradle/configuration.gradle
	$(MBGL_ANDROID_GRADLE) -Pmapbox.abis=none :MapboxGLAndroidSDK:javadocrelease

# Generates LICENSE.md file based on all Android project dependencies
.PHONY: android-license
android-license: gradle/configuration.gradle
	$(MBGL_ANDROID_GRADLE) -Pmapbox.abis=none :MapboxGLAndroidSDK:licenseReleaseReport
	python scripts/generate-license.py

# Symbolicate ndk stack traces for the arm-v7 abi
.PHONY: android-ndk-stack
android-ndk-stack: android-ndk-stack-arm-v7

# Open Android Studio if machine is macos
ifeq ($(HOST_PLATFORM), macos)
.PHONY: aproj
aproj: gradle/configuration.gradle
	open -b com.google.android.studio
endif

# Creates the configuration needed to build with Android Studio
.PHONY: android-configuration
android-configuration: gradle/configuration.gradle
	cat gradle/configuration.gradle

# Updates Android's vendor submodules
.PHONY: android-update-vendor
android-update-vendor: gradle/configuration.gradle
	$(MBGL_ANDROID_GRADLE) -Pmapbox.abis=none updateVendorSubmodules

# Run android nitpick script
.PHONY: run-android-nitpick
run-android-nitpick: android-update-vendor
	$(MBGL_ANDROID_GRADLE) -Pmapbox.abis=none androidNitpick

# Creates a dependency graph using Graphviz
.PHONY: android-graph
android-graph: gradle/configuration.gradle
	$(MBGL_ANDROID_GRADLE) -Pmapbox.abis=none :MapboxGLAndroidSDK:generateDependencyGraphMapboxLibraries

#### Miscellaneous targets #####################################################

.PHONY: clean
clean:
	-rm -rf ./gradle/configuration.gradle \
	        ./MapboxGLAndroidSDK/build \
	        ./MapboxGLAndroidSDK/.externalNativeBuild \
	        ./MapboxGLAndroidSDKTestApp/build \
	        ./MapboxGLAndroidSDKTestApp/src/androidTest/java/com/mapbox/mapboxsdk/testapp/activity/gen \
	        ./MapboxGLAndroidSDK/src/main/assets \
		    ./MapboxGLAndroidSDKTestApp/src/main/assets/integration