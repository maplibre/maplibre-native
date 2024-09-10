export BUILDTYPE ?= Debug
export RENDERER ?= drawable
export IS_LOCAL_DEVELOPMENT ?= true
export TARGET_BRANCH ?= main

CMAKE ?= cmake


ifeq ($(BUILDTYPE), Release)
else ifeq ($(BUILDTYPE), RelWithDebInfo)
else ifeq ($(BUILDTYPE), Sanitize)
else ifeq ($(BUILDTYPE), Debug)
else
  $(error BUILDTYPE must be Debug, Sanitize, Release or RelWithDebInfo)
endif

ifeq ($(RENDERER), drawable)
else ifeq ($(RENDERER), vulkan)
else
  $(error RENDERER must be 'drawable' (OpenGL) or 'vulkan')
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

MLN_ANDROID_ABIS  = arm-v7;armeabi-v7a
MLN_ANDROID_ABIS += arm-v8;arm64-v8a
MLN_ANDROID_ABIS += x86;x86
MLN_ANDROID_ABIS += x86-64;x86_64

MLN_ANDROID_LOCAL_WORK_DIR = /data/local/tmp/core-tests
MLN_ANDROID_LOCAL_BENCHMARK_DIR = /data/local/tmp/benchmark
MLN_ANDROID_LIBDIR = lib$(if $(filter arm-v8 x86-64,$1),64)
MLN_ANDROID_DALVIKVM = dalvikvm$(if $(filter arm-v8 x86-64,$1),64,32)
MLN_ANDROID_APK_SUFFIX = $(if $(filter Release,$(BUILDTYPE)),release,debug)
MLN_ANDROID_CORE_TEST_DIR = MapLibreAndroid/.externalNativeBuild/cmake/$(buildtype)/$2/core-tests
MLN_ANDROID_BENCHMARK_DIR = MapLibreAndroid/.externalNativeBuild/cmake/$(buildtype)/$2/benchmark
MLN_ANDROID_STL ?= c++_static
SOCKET_TIMEOUT = 360000
CONNECTION_TIMEOUT = 360000
MLN_ANDROID_GRADLE = ./gradlew --parallel --max-workers=$(JOBS) -Pmapbox.buildtype=$(buildtype) -Pmapbox.stl=$(MLN_ANDROID_STL) -Dorg.gradle.internal.http.socketTimeout=$(SOCKET_TIMEOUT) -Dorg.gradle.internal.http.connectionTimeout=$(CONNECTION_TIMEOUT)
MLN_ANDROID_GRADLE_SINGLE_JOB = ./gradlew --parallel --max-workers=1 -Pmapbox.buildtype=$(buildtype) -Pmapbox.stl=$(MLN_ANDROID_STL) -Dorg.gradle.internal.http.socketTimeout=$(SOCKET_TIMEOUT) -Dorg.gradle.internal.http.connectionTimeout=$(CONNECTION_TIMEOUT)

# Generate code based on the style specification
.PHONY: android-style-code
android-style-code:
	node scripts/generate-style-code.js
style-code: android-style-code

define ANDROID_RULES
# $1 = arm-v7 (short arch)
# $2 = armeabi-v7a (internal arch)

.PHONY: android-test-lib-$1
android-test-lib-$1:
	$(MLN_ANDROID_GRADLE) -Pmaplibre.abis=$2 -Pmapbox.with_test=true :MapLibreAndroidTestApp:assemble$(BUILDTYPE)

.PHONY: android-benchmark-$1
android-benchmark-$1:
	$(MLN_ANDROID_GRADLE) -Pmaplibre.abis=$2 -Pmapbox.with_benchmark=true :MapLibreAndroidTestApp:assemble$(BUILDTYPE)

# Build SDK for for specified abi
.PHONY: android-lib-$1
android-lib-$1:
	$(MLN_ANDROID_GRADLE) -Pmaplibre.abis=$2 :MapLibreAndroid:assemble$(RENDERER)$(BUILDTYPE)

# Build test app and SDK for for specified abi
.PHONY: android-$1
android-$1:
	$(MLN_ANDROID_GRADLE) -Pmaplibre.abis=$2 :MapLibreAndroidTestApp:assemble$(BUILDTYPE)

# Build the core test for specified abi
.PHONY: android-core-test-$1
android-core-test-$1: android-test-lib-$1
	# Compile main sources and extract the classes (using the test app to get all transitive dependencies in one place)
	mkdir -p $(MLN_ANDROID_CORE_TEST_DIR)
	unzip -o MapLibreAndroidTestApp/build/outputs/apk/$(buildtype)/MapLibreAndroidTestApp-$(MLN_ANDROID_APK_SUFFIX).apk classes.dex -d $(MLN_ANDROID_CORE_TEST_DIR)

run-android-core-test-$1-%: android-core-test-$1
	# Ensure clean state on the device
	adb shell "rm -Rf $(MLN_ANDROID_LOCAL_WORK_DIR) && mkdir -p $(MLN_ANDROID_LOCAL_WORK_DIR)/test && mkdir -p $(MLN_ANDROID_LOCAL_WORK_DIR)/scripts/style-spec-reference"

	# Push all needed files to the device
	adb push $(MLN_ANDROID_CORE_TEST_DIR)/classes.dex $(MLN_ANDROID_LOCAL_WORK_DIR) > /dev/null 2>&1
	adb push MapLibreAndroid/build/intermediates/intermediate-jars/$(buildtype)/jni/$2/libmaplibre.so $(MLN_ANDROID_LOCAL_WORK_DIR) > /dev/null 2>&1
	adb push test/fixtures $(MLN_ANDROID_LOCAL_WORK_DIR)/test > /dev/null 2>&1
	adb push scripts/style-spec-reference/v8.json $(MLN_ANDROID_LOCAL_WORK_DIR)/scripts/style-spec-reference > /dev/null 2>&1
	adb push MapLibreAndroid/build/intermediates/cmake/$(buildtype)/obj/$2/mbgl-test $(MLN_ANDROID_LOCAL_WORK_DIR) > /dev/null 2>&1

# Create gtest filter for skipped tests.
	$(eval SKIPPED_TESTS := -$(shell sed -n '/#\|^$$/!p' tests/skipped.txt | sed ':a;$!N;s/\n/:/g;ta'))

	# Kick off the tests
	adb shell "export LD_LIBRARY_PATH=$(MLN_ANDROID_LOCAL_WORK_DIR) && cd $(MLN_ANDROID_LOCAL_WORK_DIR) && chmod +x mbgl-test && ./mbgl-test --class_path=$(MLN_ANDROID_LOCAL_WORK_DIR)/classes.dex --gtest_filter=$$*:$(SKIPPED_TESTS)"

	# Gather the results and unpack them
	adb shell "cd $(MLN_ANDROID_LOCAL_WORK_DIR) && tar -cvzf results.tgz test/fixtures/*  > /dev/null 2>&1"
	adb pull $(MLN_ANDROID_LOCAL_WORK_DIR)/results.tgz $(MLN_ANDROID_CORE_TEST_DIR)/ > /dev/null 2>&1
	rm -rf $(MLN_ANDROID_CORE_TEST_DIR)/results && mkdir -p $(MLN_ANDROID_CORE_TEST_DIR)/results
	tar -xzf $(MLN_ANDROID_CORE_TEST_DIR)/results.tgz --strip-components=2 -C $(MLN_ANDROID_CORE_TEST_DIR)/results

# Run the core test for specified abi
.PHONY: run-android-core-test-$1
run-android-core-test-$1: run-android-core-test-$1-*

# Run benchmarks for specified abi
.PHONY: run-android-benchmark-$1
run-android-benchmark-$1: run-android-benchmark-$1-*

run-android-benchmark-$1-%: android-benchmark-$1
	mkdir -p $(MLN_ANDROID_BENCHMARK_DIR)
	unzip -o MapLibreAndroidTestApp/build/outputs/apk/$(buildtype)/MapLibreAndroidTestApp-$(MLN_ANDROID_APK_SUFFIX).apk classes.dex -d $(MLN_ANDROID_BENCHMARK_DIR)

	# Delete old test folder and create new one
	adb shell "rm -Rf $(MLN_ANDROID_LOCAL_BENCHMARK_DIR) && mkdir -p $(MLN_ANDROID_LOCAL_BENCHMARK_DIR)/benchmark && mkdir -p $(MLN_ANDROID_LOCAL_BENCHMARK_DIR)/test"

	# Push compiled java sources, test data and executable to device
	adb push $(MLN_ANDROID_BENCHMARK_DIR)/classes.dex $(MLN_ANDROID_LOCAL_BENCHMARK_DIR) > /dev/null 2>&1
	adb push MapLibreAndroid/build/intermediates/intermediate-jars/$(buildtype)/jni/$2/libmaplibre.so $(MLN_ANDROID_LOCAL_BENCHMARK_DIR) > /dev/null 2>&1
	adb push benchmark/fixtures $(MLN_ANDROID_LOCAL_BENCHMARK_DIR)/benchmark > /dev/null 2>&1
	adb push test/fixtures $(MLN_ANDROID_LOCAL_BENCHMARK_DIR)/test > /dev/null 2>&1
	adb push MapLibreAndroid/build/intermediates/cmake/$(buildtype)/obj/$2/mbgl-benchmark $(MLN_ANDROID_LOCAL_BENCHMARK_DIR) > /dev/null 2>&1

	# Run benchmark. Number of benchmark iterations can be set by run-android-benchmark-N parameter.
	adb shell "export LD_LIBRARY_PATH=$(MLN_ANDROID_LOCAL_BENCHMARK_DIR) && cd $(MLN_ANDROID_LOCAL_BENCHMARK_DIR) && chmod +x mbgl-benchmark && ./mbgl-benchmark --class_path=$(MLN_ANDROID_LOCAL_BENCHMARK_DIR)/classes.dex --benchmark_repetitions=$$* --benchmark_format=json --benchmark_out=results.json"

	# Pull results.json from the device
	rm -rf $(MLN_ANDROID_BENCHMARK_DIR)/results && mkdir -p $(MLN_ANDROID_BENCHMARK_DIR)/results
	adb pull $(MLN_ANDROID_LOCAL_BENCHMARK_DIR)/results.json $(MLN_ANDROID_BENCHMARK_DIR)/results > /dev/null 2>&1

# Run the test app on connected android device with specified abi
.PHONY: run-android-$1
run-android-$1:
	-adb uninstall org.maplibre.testapp 2> /dev/null
	$(MLN_ANDROID_GRADLE) -Pmaplibre.abis=$2 :MapLibreAndroidTestApp:install$(BUILDTYPE) && adb shell am start -n org.maplibre.testapp/.activity.FeatureOverviewActivity

# Build test app instrumentation tests apk and test app apk for specified abi
.PHONY: android-ui-test-$1
android-ui-test-$1:
	$(MLN_ANDROID_GRADLE) -Pmaplibre.abis=$2 :MapLibreAndroidTestApp:assembleDebug :MapLibreAndroidTestApp:assembleAndroidTest

# Run test app instrumentation tests on a connected android device or emulator with specified abi
.PHONY: run-android-ui-test-$1
run-android-ui-test-$1:
	-adb uninstall org.maplibre.testapp 2> /dev/null
	$(MLN_ANDROID_GRADLE) -Pmaplibre.abis=$2 :MapLibreAndroidTestApp:connectedAndroidTest

# Run Java Instrumentation tests on a connected android device or emulator with specified abi and test filter
run-android-ui-test-$1-%:
	-adb uninstall org.maplibre.testapp 2> /dev/null
	$(MLN_ANDROID_GRADLE) -Pmaplibre.abis=$2 :MapLibreAndroidTestApp:connectedAndroidTest -Pandroid.testInstrumentationRunnerArguments.class="$$*"

# Symbolicate native stack trace with the specified abi
.PHONY: android-ndk-stack-$1
android-ndk-stack-$1:
	adb logcat | ndk-stack -sym MapLibreAndroid/build/intermediates/cmake/debug/obj/$2/

# Run render tests with pixelmatch
.PHONY: run-android-render-test-$1
run-android-render-test-$1: $(BUILD_DEPS)
	-adb uninstall org.maplibre.testapp 2> /dev/null
	# delete old test results
	rm -rf build/render-test/maplibre/
  # copy test definitions & ignore file to test app assets folder, clear old ones first
	rm -rf MapLibreAndroidTestApp/src/main/assets/integration
	cp -r metrics/integration MapLibreAndroidTestApp/src/main/assets
	cp platform/node/test/ignores.json MapLibreAndroidTestApp/src/main/assets/integration/ignores.json
	# run RenderTest.java to generate static map images
	$(MLN_ANDROID_GRADLE) -Pmaplibre.abis=$2 :MapLibreAndroidTestApp:connectedAndroidTest -Pandroid.testInstrumentationRunnerArguments.class="org.maplibre.testapp.render.RenderTest"
	# pull generated images from the device
	adb pull "`adb shell 'printenv EXTERNAL_STORAGE' | tr -d '\r'`/maplibre/render" build/render-test
	# copy expected result and run pixelmatch
	python scripts/run-render-test.py
	# remove test definitions from assets
	rm -rf MapLibreAndroidTestApp/src/main/assets/integration

endef

# Explodes the arguments into individual variables
define ANDROID_RULES_INVOKER
$(call ANDROID_RULES,$(word 1,$1),$(word 2,$1))
endef

$(foreach abi,$(MLN_ANDROID_ABIS),$(eval $(call ANDROID_RULES_INVOKER,$(subst ;, ,$(abi)))))

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
run-android-unit-test:
	$(MLN_ANDROID_GRADLE) -Pmaplibre.abis=none :MapLibreAndroid:testLegacyDebugUnitTest --info
run-android-unit-test-%:
	$(MLN_ANDROID_GRADLE) -Pmaplibre.abis=none :MapLibreAndroid:testLegacyDebugUnitTest --info --tests "$*"

# Builds a release package and .tar.gz with debug symbols of the Android SDK
.PHONY: apackage
apackage:
	echo "Building for $(RENDERER)"
	make android-lib-arm-v7 && make android-lib-arm-v8 && make android-lib-x86 && make android-lib-x86-64
	$(MLN_ANDROID_GRADLE) -Pmaplibre.abis=all assemble$(RENDERER)$(BUILDTYPE)
	mkdir -p build
	tar -czvf build/debug-symbols.tar.gz -C MapLibreAndroid/build/intermediates/library_jni/*/*JniLibsProjectOnly/jni .

# Build test app instrumentation tests apk and test app apk for all abi's
.PHONY: android-ui-test
android-ui-test:
	$(MLN_ANDROID_GRADLE) -Pmaplibre.abis=all :MapLibreAndroidTestApp:assembleDebug :MapLibreAndroidTestApp:assembleAndroidTest

#Run instrumentations tests on MicroSoft App Center, ${devices} can be "xiaomi","huawei","OnePlus","htc"
.PHONY: run-android-test-app-center
run-android-test-app-center:
	appcenter test run espresso --app "MapLibre-mobile/Maps-sdk" --devices "MapLibre-mobile/${devices}" --app-path MapLibreAndroidTestApp/build/outputs/apk/debug/MapLibreAndroidTestApp-debug.apk  --test-series "master" --locale "en_US" --build-dir MapLibreAndroidTestApp/build/outputs/apk/androidTest/debug --token ${APPCENTER_ACCESS_TOKEN}

# Uploads the compiled Android SDK to Maven Central Staging
.PHONY: run-android-publish
run-android-publish:
	$(MLN_ANDROID_GRADLE_SINGLE_JOB)-Pmaplibre.abis=all :MapLibreAndroid:publishReleasePublicationToSonatypeRepository closeAndReleaseSonatypeStagingRepository

# Dump system graphics information for the test app
.PHONY: android-gfxinfo
android-gfxinfo:
	adb shell dumpsys gfxinfo org.maplibre.testapp reset

# Runs checkstyle and lint on the java code
.PHONY: android-check
android-check : android-ktlint android-checkstyle android-lint-sdk android-lint-test-app run-android-nitpick

# Runs checkstyle on the java code
.PHONY: android-checkstyle
android-checkstyle:
	$(MLN_ANDROID_GRADLE) -Pmaplibre.abis=none :MapLibreAndroid:checkstyle :MapLibreAndroidTestApp:checkstyle

# Runs checkstyle on the kotlin code
.PHONY: android-ktlint
android-ktlint:
	$(MLN_ANDROID_GRADLE) -Pmaplibre.abis=none checkstyle

# Runs lint on the Android SDK java code
.PHONY: android-lint-sdk
android-lint-sdk:
	$(MLN_ANDROID_GRADLE) -Pmaplibre.abis=none :MapLibreAndroid:lint

# Runs lint on the Android test app java code
.PHONY: android-lint-test-app
android-lint-test-app:
	$(MLN_ANDROID_GRADLE) -Pmaplibre.abis=none :MapLibreAndroidTestApp:lint

# Generates LICENSE.md file based on all Android project dependencies
.PHONY: android-license
android-license:
	$(MLN_ANDROID_GRADLE) -Pmaplibre.abis=none :MapLibreAndroid:licenseDrawableReleaseReport
	python3 scripts/generate-license.py

# Symbolicate ndk stack traces for the arm-v7 abi
.PHONY: android-ndk-stack
android-ndk-stack: android-ndk-stack-arm-v7

# Run android nitpick script
.PHONY: run-android-nitpick
run-android-nitpick:
	$(MLN_ANDROID_GRADLE) -Pmaplibre.abis=none androidNitpick

# Creates a dependency graph using Graphviz
.PHONY: android-graph
android-graph:
	$(MLN_ANDROID_GRADLE) -Pmaplibre.abis=none :MapLibreAndroid:generateDependencyGraphMapboxLibraries

# Lists tasks
.PHONY: list-tasks
list-tasks:
	$(MLN_ANDROID_GRADLE) -Pmaplibre.abis=all tasks

#### Miscellaneous targets #####################################################

.PHONY: clean
clean:
	-rm -rf ./MapLibreAndroid/build \
	        ./MapLibreAndroid/.externalNativeBuild \
	        ./MapLibreAndroidTestApp/build \
	        ./MapLibreAndroidTestApp/src/androidTest/java/org/maplibre/android/testapp/activity/gen \
	        ./MapLibreAndroid/src/main/assets \
		    ./MapLibreAndroidTestApp/src/main/assets/integration
