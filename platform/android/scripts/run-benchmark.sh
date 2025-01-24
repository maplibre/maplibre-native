#!/bin/bash

./gradlew assembleLegacyRelease assembleLegacyReleaseAndroidTest -PtestBuildType=release

export AWS_DEVICE_FARM_PROJECT_ARN="arn:aws:devicefarm:us-west-2:373521797162:project:20687d72-0e46-403e-8f03-0941850665bc"
export AWS_DEVICE_FARM_DEVICE_POOL_ARN="arn:aws:devicefarm:us-west-2:373521797162:devicepool:20687d72-0e46-403e-8f03-0941850665bc/b4d75cb6-f210-4927-b94e-17eae054fea7"
export appType=ANDROID_APP
export appFile="MapLibreAndroidTestApp/build/outputs/apk/legacy/release/MapLibreAndroidTestApp-legacy-release.apk"
export testFile="MapLibreAndroidTestApp/build/outputs/apk/androidTest/legacy/release/MapLibreAndroidTestApp-legacy-release-androidTest.apk"
export testType="INSTRUMENTATION"
export testPackageType="INSTRUMENTATION_TEST_PACKAGE"
export testSpecArn="arn:aws:devicefarm:us-west-2:373521797162:upload:20687d72-0e46-403e-8f03-0941850665bc/14862afb-cf88-44aa-9f1e-5131cbb22f01"
export testFilter="org.maplibre.android.benchmark.Benchmark"
export name="Android Benchmark"

../../scripts/aws-device-farm/aws-device-farm-run.sh
