export ROOT = vendor/mapbox-gl-native
export IOS_OUTPUT_PATH = build/ios
export IOS_WORK_PATH = platform/ios/ios.xcworkspace

.PHONY: iproj
iproj:
	make -f vendor/mapbox-gl-native/Makefile iproj
	xed $(IOS_WORK_PATH)