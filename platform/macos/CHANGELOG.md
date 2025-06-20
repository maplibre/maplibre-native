# Changelog for Mapbox Maps SDK for macOS

MapLibre welcomes participation and contributions from everyone. Please read [`Contributing Guide`](https://github.com/maplibre/maplibre-native/blob/main/CONTRIBUTING.md) to get started.

## 0.19.1 - September 06, 2021

### Bug Fixes

* Invalid tile url template for MapLibre style [#107](https://github.com/maplibre/maplibre-native/pull/107)
* Adjust local glyphs [#122](https://github.com/maplibre/maplibre-native/pull/122)
* Return correct Mapbox TileServerOptions [#132](https://github.com/maplibre/maplibre-native/pull/132)

### Other

* Fix perf regression in cached tiles of tile pyramid [#129](https://github.com/maplibre/maplibre-native/pull/129)
* Upgrade to newest protozero [#148](https://github.com/maplibre/maplibre-native/pull/148)
* Fix build for Xcode 12.5 & 13-beta. [#153](https://github.com/maplibre/maplibre-native/pull/153)

## 0.19.0 - June 29, 2021

### Features

* Replacing hardcoded configuration with configurable API and removing Mapbox assets and dependencies [#90](https://github.com/maplibre/maplibre-native/pull/90)

## 0.18.0 - March 12, 2021

### Features

* Added open geo url support to the macos app.

### Other

* mapbox-gl-js submodule has been replaced with maplibre-gl-js
* Removed Mapbox Mobile Events and Telemetry [#7](https://github.com/maplibre/maplibre-native/pull/7)

## 0.17.0 - January 6, 2021

### Features

* Added the mbtiles file source for rendering vector tiles from file stored locally on the device.

## 0.16.0

### Styles and rendering

* Added the `mgl_distanceFrom:` expression function for calculating the shortest distance between the evaluated feature and an `MLNPointAnnotation`, `MLNPointCollection`, `MLNPolyline`, `MLNMultiPolyline`, `MLNPolygon`, or `MLNMultiPolygon` that you specify as part of an `NSExpression` format string. ([#295](https://github.com/mapbox/mapbox-gl-native-ios/pull/295))
* Added the `MLNCircleStyleLayer.circleSortKey` property. ([mapbox/mapbox-gl-native#15875](https://github.com/mapbox/mapbox-gl-native/pull/15875))
* Chinese, Japanese, and Korean characters are now set in the font specified in style JSON or by the `MLNSymbolLayer.textFontNames` property. If the named font is not installed on the device or bundled with the application, the characters are set in a fallback font listed in the `MLNIdeographicFontFamilyName` key in the Info.plist file. ([#189](https://github.com/mapbox/mapbox-gl-native-ios/pull/189))
* The `MLNIdeographicFontFamilyName` key in the Info.plist file can now specify the fallback fonts for CJK characters by their PostScript names or display names in addition to font family names. For example, you can specify `NotoSansCJKjp-Bold` or `Noto Sans CJK JP Bold`, which ensures that the characters are set in bold. You can alternatively specify these font names in the `MLNIdeographicFontFamilyName` key of `NSUserDefaults.standardUserDefaults`. ([#189](https://github.com/mapbox/mapbox-gl-native-ios/pull/189))
* CJK characters are now laid out according to the font, so fonts with nonsquare glyphs have the correct kerning. ([#189](https://github.com/mapbox/mapbox-gl-native-ios/pull/189))
* Fixed an issue where the baseline for CJK characters was too low compared to non-CJK characters. ([#189](https://github.com/mapbox/mapbox-gl-native-ios/pull/189))
* Fixed a crash when calling the `-[MLNStyle removeImageForName:]` method with the name of a nonexistent image. ([mapbox/mapbox-gl-native#16391](https://github.com/mapbox/mapbox-gl-native/pull/16391))
* Fixed an issue where properties such as `MLNFillStyleLayer.fillColor` and `MLNLineStyleLayer.lineColor` misinterpreted non-opaque `NSColor`s. ([#266](https://github.com/mapbox/mapbox-gl-native-ios/pull/266))

### Offline maps

* Added the `-[MLNOfflinePack setContext:completionHandler:]` method for replacing the data associated with an offline pack, such as a name. ([#288](https://github.com/mapbox/mapbox-gl-native-ios/pull/288))
* Added the `MLNOfflineStorage.databasePath` and `MLNOfflineStorage.databaseURL` properties to obtain the path of the database that contains offline packs and the ambient cache. To customize this path, set the `MLNOfflineStorageDatabasePath` in Info.plist. ([#298](https://github.com/mapbox/mapbox-gl-native-ios/pull/298))
* Fixed an error that occurred if your implementation of the `-[MLNOfflineStorageDelegate offlineStorage:URLForResourceOfKind:]` method returned a local file URL. ([mapbox/mapbox-gl-native#16428](https://github.com/mapbox/mapbox-gl-native/pull/16428))

### Other changes

* Fixed a crash when encountering an invalid polyline. ([mapbox/mapbox-gl-native#16409](https://github.com/mapbox/mapbox-gl-native/pull/16409))
* Fixed an issue where an `MLNMapSnapshotOptions` with an invalid `MLNMapCamera.centerCoordinate`, negative `MLNMapCamera.heading`, negative `MLNMapCamera.pitch`, and negative `MLNMapSnapshotOptions.zoomLevel` resulted in a snapshot centered on Null Island at zoom level 0 even if the style specified a different initial center coordinate or zoom level. ([#280](https://github.com/mapbox/mapbox-gl-native-ios/pull/280))
* Certain logging statements no longer run on the main thread. ([mapbox/mapbox-gl-native#16325](https://github.com/mapbox/mapbox-gl-native/pull/16325))

## 0.15.1

* Fixed various crashes, including crashes on launch, on macOS 10.11.0 through 10.14._x_. ([mapbox/mapbox-gl-native#16412](https://github.com/mapbox/mapbox-gl-native/pull/16412))

## 0.15.0

### Styles and rendering

* Added the `-[MLNMapViewDelegate mapView:shouldRemoveStyleImage:]` method for optimizing style image caching. ([#14769](https://github.com/mapbox/mapbox-gl-native/pull/14769))
* Added the `image` expression function for converting an image name into a style image. Use this function in expressions in style JSON or with the `MLN_FUNCTION()` syntax in an `NSExpression` format string. Image expressions are compatible with the `mgl_attributed:` expression function and `MLNAttributedExpression` classes for embedding icons inline in text labels. ([#15877](https://github.com/mapbox/mapbox-gl-native/pull/15877), [#15937](https://github.com/mapbox/mapbox-gl-native/pull/15937))
* The `IN` and `CONTAINS` predicate operators can now test whether a string is a substring of another string or whether the evaluated feature (`SELF`) lies within a given `MLNShape` or `MLNFeature`. ([#183](https://github.com/mapbox/mapbox-gl-native-ios/pull/183), [#184](https://github.com/mapbox/mapbox-gl-native-ios/pull/184))
* Added the `MLNSymbolStyleLayer.textWritingModes` layout property. This property can be set to `MLNTextWritingModeHorizontal` or `MLNTextWritingModeVertical`. ([#14932](https://github.com/mapbox/mapbox-gl-native/pull/14932))
* Added the `MLNLineStyleLayer.lineSortKey` and `MLNFillStyleLayer.fillSortKey` properties. ([#179](https://github.com/mapbox/mapbox-gl-native-ios/pull/179))
* The `MLNSymbolStyleLayer.iconTextFit` property now respects the cap insets of any nine-part stretchable image passed into the `-[MLNStyle setImage:forName:]` method. You can define the stretchable area in Xcode’s asset catalog or by setting the `NSImage.capInsets` property. ([#182](https://github.com/mapbox/mapbox-gl-native-ios/pull/182))
* Fixed crashes triggered when `MLNSource` and `MLNStyleLayer` objects are accessed after having been invalidated after a style change. ([#15539](https://github.com/mapbox/mapbox-gl-native/pull/15539))
* Fixed an issue where fill extrusion layers would be incorrectly rendered above other layers. ([#15065](https://github.com/mapbox/mapbox-gl-native/pull/15065))
* Fixed rendering and collision detection issues with using `MLNSymbolStyleLayer.textVariableAnchor` and `MLNSymbolStyleLayer.iconTextFit` properties on the same layer. ([#15367](https://github.com/mapbox/mapbox-gl-native/pull/15367))
* Fixed symbol overlap when zooming out quickly. ([#15416](https://github.com/mapbox/mapbox-gl-native/pull/15416))
* Fixed an issue where non-template images would draw as template images when used in the same style layer. ([#15456](https://github.com/mapbox/mapbox-gl-native/pull/15456))
* Fixed an issue where the collision boxes for symbols would not be updated when `MLNSymbolStyleLayer.textTranslation` or `MLNSymbolStyleLayer.iconTranslation` were used. ([#15467](https://github.com/mapbox/mapbox-gl-native/pull/15467))
* Fixed an issue that caused `MLNTileSourceOptionMaximumZoomLevel` to be ignored when setting `MLNTileSource.configurationURL`. ([#15581](https://github.com/mapbox/mapbox-gl-native/pull/15581))
* Fixed an issue where `MLNSymbolStyleLayer.symbolSortKey` could sort text and icons incorrectly. ([#16023](https://github.com/mapbox/mapbox-gl-native/pull/16023))
* Fixed an issue where style layers backed by a shape source could flicker when transitioning between styles. ([#15907](https://github.com/mapbox/mapbox-gl-native/pull/15907), [#15941](https://github.com/mapbox/mapbox-gl-native/pull/15941))
* Improved the performance of loading a style that has many style images. ([#16187](https://github.com/mapbox/mapbox-gl-native/pull/16187))
* Updated “map ID” to the more accurate term “tileset ID” in documentation; updated “style's Map ID” to the more accurate term “style URL”. ([#15116](https://github.com/mapbox/mapbox-gl-native/pull/15116))

### Camera

* Setting `MLNMapView.contentInset` now moves the map’s focal point to the center of the content frame after insetting. ([#14664](https://github.com/mapbox/mapbox-gl-native/pull/14664))
* The `-[MLNMapView setCamera:withDuration:animationTimingFunction:edgePadding:completionHandler:]` method now adds the current value of the `MLNMapView.contentInsets` property to the `edgePadding` parameter. ([#14813](https://github.com/mapbox/mapbox-gl-native/pull/14813))
* Added variants of multiple animated `MLNMapView` methods that accept completion handlers ([#14381](https://github.com/mapbox/mapbox-gl-native/pull/14381)):
  * `-[MLNMapView setVisibleCoordinateBounds:edgePadding:animated:completionHandler:]`
  * `-[MLNMapView setContentInsets:animated:completionHandler:]`
  * `-[MLNMapView showAnnotations:edgePadding:animated:completionHandler:]`
* Added the `MLNMapView.minimumPitch` and `MLNMapView.maximumPitch` properties to further limit how much the user or your code can tilt the map. ([#208](https://github.com/mapbox/mapbox-gl-native-ios/pull/208))
* Fixed an issue where it was possible to set the map’s content insets then tilt the map enough to see the horizon, causing performance issues. ([#15195](https://github.com/mapbox/mapbox-gl-native/pull/15195))
* Fixed an issue where animated camera transitions zoomed in or out too dramatically. ([#15281](https://github.com/mapbox/mapbox-gl-native/pull/15281))
* Improved performance when continuously animating a tilted map. ([#16287](https://github.com/mapbox/mapbox-gl-native/pull/16287))

### Feature querying

* Fixed an issue where `-[MLNMapView visibleFeaturesInRect:]` and `-[MLNShapeSource featuresMatchingPredicate:]` omitted some features from the return value. ([#14884](https://github.com/mapbox/mapbox-gl-native/pull/14884))
* Fixed an issue where `-[MLNMapView visibleFeaturesInRect:]` and `-[MLNShapeSource featuresMatchingPredicate:]` could return incorrect coordinates at zoom levels 20 and higher. ([#15560](https://github.com/mapbox/mapbox-gl-native/pull/15560))
* Improved feature querying performance. ([#14930](https://github.com/mapbox/mapbox-gl-native/pull/14930))

### Snapshots

* Added an `-[MLNMapSnapshotter startWithOverlayHandler:completionHandler:]` method to provide the snapshot's current `CGContext` in order to perform custom drawing on `MLNMapSnapshot` objects. ([#15530](https://github.com/mapbox/mapbox-gl-native/pull/15530))
* Added the `MLNMapSnapshotter.delegate` property and `MLNMapSnapshotterDelegate` protocol for customizing the style before taking a snapshot. ([#235](https://github.com/mapbox/mapbox-gl-native-ios/pull/235))
* You no longer need to explicitly capture the `MLNMapSnapshotter` object in the completion handler that you specify in `-[MLNMapSnapshotter startWithCompletionHandler:]`. Even if you declare the snapshotter locally without holding a strong reference to it, the snapshotter is only deallocated after the completion handler finishes, and the completion handler generally receives a valid snapshot. ([#210](https://github.com/mapbox/mapbox-gl-native-ios/pull/210))
* The `-[MLNMapSnapshotter cancel]` method no longer calls the completion handler passed into `-[MLNMapSnapshotter startWithCompletionHandler:]`. ([#210](https://github.com/mapbox/mapbox-gl-native-ios/pull/210))
* Fixed an issue where the `MLNMapSnapshotter.loading` property always returned `NO`, even while loading a snapshot. ([#210](https://github.com/mapbox/mapbox-gl-native-ios/pull/210))

### Networking and storage

* Ideographic glyphs from Chinese, Japanese, and Korean are no longer downloaded by default as part of offline packs; they are instead rendered on-device, saving bandwidth and storage while improving performance. ([#14176](https://github.com/mapbox/mapbox-gl-native/pull/14176))
* Downloaded offline packs no longer reduce the storage space available for ambient caching of tiles and other resources. ([#15622](https://github.com/mapbox/mapbox-gl-native/pull/15622))
* Added the `MLNMapView.prefetchesTiles` property to configure lower-resolution tile prefetching behavior. ([#14816](https://github.com/mapbox/mapbox-gl-native/pull/14816))
* Added the `-[MLNOfflineStorage preloadData:forURL:modificationDate:expirationDate:eTag:mustRevalidate:completionHandler:]` method for determining when the data is ready to retrieve from the cache. ([#188](https://github.com/mapbox/mapbox-gl-native-ios/pull/188))
* Fixed a crash when `-[MLNOfflinePack invalidate]` is called on different threads. ([#15582](https://github.com/mapbox/mapbox-gl-native/pull/15582))
* Fixed issues where an offline pack would stop downloading before completion. ([#16230](https://github.com/mapbox/mapbox-gl-native/pull/16230), [#16240](https://github.com/mapbox/mapbox-gl-native/pull/16240))
* When an offline pack encounters an HTTP 404 error, the `MLNOfflinePackUserInfoKeyError` user info key of the `MLNOfflinePackErrorNotification` now indicates the resource that could not be downloaded. ([#16240](https://github.com/mapbox/mapbox-gl-native/pull/16240))
* Expired resources are now fetched at a lower priority than new resources. ([#15950](https://github.com/mapbox/mapbox-gl-native/pull/15950))

### Localization and internationalization

* The `MLNIdeographicFontFamilyName` Info.plist key now also accepts an array of font family names, to customize font fallback behavior. It can also be set to a Boolean value of `NO` to force the SDK to typeset CJK characters in a remote font specified by `MLNSymbolStyleLayer.textFontNames`. ([#14862](https://github.com/mapbox/mapbox-gl-native/pull/14862))
* The `MLNIdeographicFontFamilyName` Info.plist key can now be set to the display names or PostScript names of individual fonts. ([#189](https://github.com/mapbox/mapbox-gl-native-ios/pull/189))
* The `-[MLNStyle localizeLabelsIntoLocale:]` and `-[NSExpression mgl_expressionLocalizedIntoLocale:]` methods can now localize text into Traditional Chinese and Vietnamese. ([#173](https://github.com/mapbox/mapbox-gl-native-ios/pull/173))

### Other changes

* Fixed a memory leak when zooming with any options enabled in the `MLNMapView.debugMask` property. ([#15395](https://github.com/mapbox/mapbox-gl-native/pull/15395))
* `MLNLoggingLevel` has been updated to better match core log levels. You can now use `MLNLoggingConfiguration.loggingLevel` to filter logs from core. ([#15120](https://github.com/mapbox/mapbox-gl-native/pull/15120))

## 0.14.0 - May 22, 2018

### Styles and rendering

* Client-side text rendering of CJK ideographs is now enabled by default. ([#13988](https://github.com/mapbox/mapbox-gl-native/pull/13988))
* Added an `MLNStyle.performsPlacementTransitions` property to control how long it takes for colliding labels to fade out. ([#13565](https://github.com/mapbox/mapbox-gl-native/pull/13565))
* Added the `-[MLNShapeSource leavesOfCluster:offset:limit:]`, `-[MLNShapeSource childrenOfCluster:]`, `-[MLNShapeSource zoomLevelForExpandingCluster:]` methods for inspecting a cluster in an `MLNShapeSource`s created with the `MLNShapeSourceOptionClustered` option. Feature querying now returns clusters represented by `MLNPointFeatureCluster` objects (that conform to the `MLNCluster` protocol). ([#12952](https://github.com/mapbox/mapbox-gl-native/pull/12952)
* Fixed a crash when casting large numbers in `NSExpression`. ([#13580](https://github.com/mapbox/mapbox-gl-native/pull/13580))
* Fixed an issue that caused `MLN_FUNCTION` to ignore multiple formatting parameters when passed a `format` function as parameter. ([#14064](https://github.com/mapbox/mapbox-gl-native/pull/14064))
* Added `mgl_attributed:` expression operator, which concatenates `MLNAttributedExpression` objects for specifying rich text in the `MLNSymbolStyleLayer.text` property. ([#14094](https://github.com/mapbox/mapbox-gl-native/pull/14094))
* Fixed an issue that caused conditional expressions to crash when passed nested conditional expressions as parameters. ([#14181](https://github.com/mapbox/mapbox-gl-native/pull/14181))
* Fixed a possible crash with certain expressions containing arguments that evaluate to a dictionary containing `NSArray` or `NSNumber` values. ([#14352](https://github.com/mapbox/mapbox-gl-native/pull/14352))
* Fixed a bug where non-opaque `NSColor` values were ignored when assigned to a style layer color property. ([#14406](https://github.com/mapbox/mapbox-gl-native/pull/14406))
* Fixed a bug where some layers weren’t rendering correctly after panning. ([#14527](https://github.com/mapbox/mapbox-gl-native/pull/14527))
* Changed placement order of `MLNSymbolStyleLayer` to match the viewport-y order when `MLNSymbolStyleLayer.symbolZOrder` is set to `MLNSymbolZOrderViewportY`, allowing icons to overlap but not text. ([#14486](https://github.com/mapbox/mapbox-gl-native/pull/14486))

### Other changes

* Added Czech and Galician localizations. ([#13782](https://github.com/mapbox/mapbox-gl-native/pull/13782), [#14095](https://github.com/mapbox/mapbox-gl-native/pull/14095))
* Added `MLNNetworkConfiguration` class to customize the SDK’s `NSURLSessionConfiguration` object. ([#13886](https://github.com/mapbox/mapbox-gl-native/pull/13886))
* Fixed a bug with `MLNMapView.visibleAnnotations` that resulted in incorrect results and performance degradation. ([#13745](https://github.com/mapbox/mapbox-gl-native/pull/13745))
* Fixed a bug where selecting partially on-screen annotations (without a callout) would move the map. ([#13727](https://github.com/mapbox/mapbox-gl-native/pull/13727))
* Fixed a bug that caused offline packs created prior to v0.7.0 (introduced in [#11055](https://github.com/mapbox/mapbox-gl-native/pull/11055)) to be marked as `MLNOfflinePackStateInactive`. ([#14188](https://github.com/mapbox/mapbox-gl-native/pull/14188))

## 0.13.0 - December 20, 2018

### Packaging

* This SDK’s dynamic framework now has a bundle identifier of `com.mapbox.Mapbox`. ([#12857](https://github.com/mapbox/mapbox-gl-native/pull/12857))
* `MLNMapView`, `MLNShapeOfflineRegion`, and `MLNTilePyramidOfflineRegion` now default to version 11 of the Mapbox Streets style. Similarly, several class properties of `MLNStyle`, such as `MLNStyle.lightStyleURL`, have been updated to return URLs to new versions of their respective styles. ([#13585](https://github.com/mapbox/mapbox-gl-native/pull/13585))

### Styles and rendering

* Fixed an issue where the `{prefix}` token in tile URL templates was evaluated incorrectly when requesting a source’s tiles. ([#13429](https://github.com/mapbox/mapbox-gl-native/pull/13429))
* Added an `-[MLNStyle removeSource:error:]` method that returns a descriptive error if the style fails to remove the source, whereas `-[MLNStyle removeSource:]` fails silently. ([#13399](https://github.com/mapbox/mapbox-gl-native/pull/13399))
* Added the `MLNFillExtrusionStyleLayer.fillExtrusionHasVerticalGradient` property. ([#13463](https://github.com/mapbox/mapbox-gl-native/pull/13463))
* Added support for setting `MLNCollisionBehaviorPre4_0` in `NSUserDefaults`. ([#13426](https://github.com/mapbox/mapbox-gl-native/pull/13426))
* `-[MLNStyle localizeLabelsIntoLocale:]` and `-[NSExpression(MLNAdditions) mgl_expressionLocalizedIntoLocale:]` can automatically localize styles that use version 8 of the Mapbox Streets source. ([#13481](https://github.com/mapbox/mapbox-gl-native/pull/13481))
* Fixed symbol flickering during instantaneous transitions. ([#13535](https://github.com/mapbox/mapbox-gl-native/pull/13535))
* Fixed a crash when specifying `MLNShapeSourceOptionLineDistanceMetrics` when creating an `MLNShapeSource`. ([#13543](https://github.com/mapbox/mapbox-gl-native/pull/13543))

### Other changes

* Renamed `-[MLNOfflineStorage putResourceWithUrl:data:modified:expires:etag:mustRevalidate:]` to `-[MLNOfflineStorage preloadData:forURL:modificationDate:expirationDate:eTag:mustRevalidate:]`. ([#13318](https://github.com/mapbox/mapbox-gl-native/pull/13318))
* `MLNMapSnapshotter` now respects the `MLNIdeographicFontFamilyName` key in Info.plist, which reduces bandwidth consumption when snapshotting regions that contain Chinese or Japanese characters. ([#13427](https://github.com/mapbox/mapbox-gl-native/pull/13427))
* Added `MLNLoggingConfiguration` and `MLNLoggingBlockHandler` that handle error and fault events produced by the SDK. ([#13235](https://github.com/mapbox/mapbox-gl-native/pull/13235))

## 0.12.0 - November 8, 2018

### Styles and rendering

* `MLNSymbolStyleLayer.text` can now be set to rich text with varying fonts and text sizes. ([#12624](https://github.com/mapbox/mapbox-gl-native/pull/12624))
* Added an `MLNSymbolStyleLayer.symbolZOrder` property for forcing point features in a symbol layer to be layered in the same order that they are specified in the layer’s associated source. ([#12783](https://github.com/mapbox/mapbox-gl-native/pull/12783))
* Fixed a crash when the `MLNBackgroundStyleLayer.backgroundPattern`, `MLNFillExtrusionStyleLayer.fillExtrusionPattern`, `MLNFillStyleLayer.fillPattern`, or `MLNLineStyleLayer.linePattern` property evaluates to `nil` for a particular feature. ([#12896](https://github.com/mapbox/mapbox-gl-native/pull/12896))
* Fixed a crash when using the `MLN_LET`, `MLN_MATCH`, `MLN_IF`, or `MLN_FUNCTION` functions without a colon inside an `NSExpression` or `NSPredicate` format string. ([#13189](https://github.com/mapbox/mapbox-gl-native/pull/13189))
* Fixed a crash setting the `MLNLineStyleLayer.lineGradient` property to an expression containing the `$lineProgress` variable. Added an `NSExpression.lineProgressVariableExpression` class property that returns an expression for the `$lineProgress` variable. ([#13192](https://github.com/mapbox/mapbox-gl-native/pull/13192))
* Fixed an issue where features in `MLNFillStyleLayer` and `MLNLineStyleLayer` would occasionally flicker when zooming in and out. ([#12982](https://github.com/mapbox/mapbox-gl-native/pull/12982))
* Feature querying can now return point features represented by icons that have both the `MLNSymbolStyleLayer.iconRotation` and `MLNSymbolStyleLayer.iconOffset` properties applied. ([#13105](https://github.com/mapbox/mapbox-gl-native/pull/13105))
* Fixed a crash when casting an `NSColor` to an `NSColor` inside an `NSExpression`. ([#12864](https://github.com/mapbox/mapbox-gl-native/pull/12864))
* `NIL` cast to an `NSNumber` now evaluates to 0 inside an `NSExpression`. ([#12864](https://github.com/mapbox/mapbox-gl-native/pull/12864))
* Fixed a crash when applying the `to-array` operator to an empty array inside a JSON expression. ([#12864](https://github.com/mapbox/mapbox-gl-native/pull/12864))
* Added the `MLNCollisionBehaviorPre4_0` Info.plist key to restore the collision detection behavior in version 0.6 of the SDK. ([#12941](https://github.com/mapbox/mapbox-gl-native/pull/12941))

### Offline maps

* Network requests by `MLNMapView` are now prioritized over offline pack downloads. ([#13019](https://github.com/mapbox/mapbox-gl-native/pull/13019))
* Added `-[MLNOfflineStorage addContentsOfFile:withCompletionHandler:]` and `-[MLNOfflineStorage addContentsOfURL:withCompletionHandler:]` methods to add pregenerated offline packs to offline storage. ([#12791](https://github.com/mapbox/mapbox-gl-native/pull/12791))
* Added the `-[MLNOfflineStorage putResourceWithUrl:data:modified:expires:etag:mustRevalidate:]` method to allow pre-warming of the ambient cache. ([#13119](https://github.com/mapbox/mapbox-gl-native/pull/13119))
* Fixed an issue where some tiles were rendered incorrectly when the device was unable to connect to the Internet. ([#12931](https://github.com/mapbox/mapbox-gl-native/pull/12931))

### Other changes

* Added `MLNAltitudeForZoomLevel()` and `MLNZoomLevelForAltitude()` methods for converting between zoom levels used by `MLNMapView` and altitudes used by `MLNMapCamera`. ([#12986](https://github.com/mapbox/mapbox-gl-native/pull/12986))
* Deprecated the `+[MLNMapCamera cameraLookingAtCenterCoordinate:fromDistance:pitch:heading:]` method in favor of `+[MLNMapCamera cameraLookingAtCenterCoordinate:altitude:pitch:heading:]` and `+[MLNMapCamera cameraLookingAtCenterCoordinate:acrossDistance:pitch:heading:]`. ([#12966](https://github.com/mapbox/mapbox-gl-native/pull/12966))
* Fixed an issue where `+[MLNMapCamera cameraLookingAtCenterCoordinate:fromEyeCoordinate:eyeAltitude:]` created a camera looking from the wrong eye coordinate. ([#12966](https://github.com/mapbox/mapbox-gl-native/pull/12966))
* Added an `MLNMapCamera.viewingDistance` property based on the existing `MLNMapCamera.altitude` property. ([#12966](https://github.com/mapbox/mapbox-gl-native/pull/12966))
* Fixed an issue where the map view could not be panned after setting `MLNMapView.visibleCoordinateBounds` to a coordinate bounds that spanned exactly the longitudes −180° and 180°. ([#13006](https://github.com/mapbox/mapbox-gl-native/pull/13006))
* Fixed an issue where `-[MLNMapSnapshotter startWithQueue:completionHandler:]` failed to call its completion handler in some cases. ([#12355](https://github.com/mapbox/mapbox-gl-native/pull/12355))
* Fixed an issue where snapshots had the wrong heading and pitch. ([#13123](https://github.com/mapbox/mapbox-gl-native/pull/13123))
* Fixed an issue where `MLNMapView` produced a designable error in Interface Builder storyboards in Xcode 10. ([#12883](https://github.com/mapbox/mapbox-gl-native/pull/12883))

## 0.11.0 - September 13, 2018

### Styles and rendering

* When a symbol in an `MLNSymbolStyleLayer` has both an icon and text, both are shown or hidden together based on available space. ([#12521](https://github.com/mapbox/mapbox-gl-native/pull/12521))
* Invalid values of `MLNSymbolStyleLayer.textFontNames` are treated as warnings instead of errors. ([#12414](https://github.com/mapbox/mapbox-gl-native/pull/12414))
* Added an `MLNLineStyleLayer.lineGradient` property that can be used to define a gradient with which to color a line feature. ([#12575](https://github.com/mapbox/mapbox-gl-native/pull/12575))
* The `MLNLineStyleLayer.linePattern`, `MLNFillStyleLayer.fillPattern`, and `MLNFillStyleLayer.fillExtrusionPattern` properties can now be set to expressions that refer to feature attributes. ([#12284](https://github.com/mapbox/mapbox-gl-native/pull/12284))
* Reduced the amount of memory consumed by font data after changing the style. ([#12414](https://github.com/mapbox/mapbox-gl-native/pull/12414))
* `-[MLNShapeSource initWithIdentifier:shape:options:]` and `-[MLNComputedShapeSource setFeatures:inTileAtX:y:zoomLevel:]` warn about possible attribute loss when passing in an `MLNShapeCollection` object. ([#12625](https://github.com/mapbox/mapbox-gl-native/pull/12625))
* Added an `MLNShapeSourceOptionLineDistanceMetrics` option that enables or disables calculating line distance metrics. ([#12604](https://github.com/mapbox/mapbox-gl-native/pull/12604))
* Fixed an issue where the `cubic-bezier` curve type for `mgl_interpolate:withCurveType:parameters:stops:` expressions was misinterpreted for some style layer properties. ([#12826](https://github.com/mapbox/mapbox-gl-native/pull/12826))
* Fixed an issue that could cause symbols to fade in during pan operations instead of always showing when using `MLNSymbolStyleLayer.iconAllowsOverlap` or `MLNSymbolStyleLayer.textAllowsOverlap` properties. ([#12698](https://github.com/mapbox/mapbox-gl-native/pull/12698))

### Offline maps

* Added the `MLNShapeOfflineRegion` class for creating an offline pack that covers an arbitrary shape. ([#11447](https://github.com/mapbox/mapbox-gl-native/pull/11447))
* Fixed crashes when offline storage encountered certain SQLite errors. ([#12224](https://github.com/mapbox/mapbox-gl-native/pull/12224))

### Other changes

* The `-[MLNMapView annotationAtPoint:]` method can now return annotations near tile boundaries at high zoom levels. ([#12570](https://github.com/mapbox/mapbox-gl-native/pull/12570))
* Added an `-[MLNMapViewDelegate mapView:shapeAnnotationIsEnabled:]` method to specify whether an annotation is selectable. ([#12352](https://github.com/mapbox/mapbox-gl-native/pull/12352))
* Fixed inconsistencies in exception naming. ([#12583](https://github.com/mapbox/mapbox-gl-native/issues/12583))
* Fixed an issue where `-[MLNMapView convertCoordinateBounds:toRectToView:]` would return an empty CGRect if the bounds crossed the antimeridian. ([#12758](https://github.com/mapbox/mapbox-gl-native/pull/12758))

## 0.10.0 - August 15, 2018

## Styles and rendering

* Token string syntax (`"{token}"`) in `MLNSymbolStyleLayer` `text` and `iconImageName` properties is now correctly converted to the appropriate `NSExpression` equivalent. ([#11659](https://github.com/mapbox/mapbox-gl-native/issues/11659))
* Fixed a crash when switching between two styles having layers with the same identifier but different layer types. ([#12432](https://github.com/mapbox/mapbox-gl-native/issues/12432))
* Added a new option to `MLNSymbolPlacement`, `MLNSymbolPlacementLineCenter`, that places the label relative to the center of the geometry. ([#12337](https://github.com/mapbox/mapbox-gl-native/pull/12337))

## Other changes

* Fixed an issue where the symbols for `MLNMapPointForCoordinate` could not be found. ([#12445](https://github.com/mapbox/mapbox-gl-native/issues/12445))
* Fixed an issue causing country and ocean labels to disappear after calling `-[MLNStyle localizeLabelsIntoLocale:]` when the system language is set to Simplified Chinese. ([#12164](https://github.com/mapbox/mapbox-gl-native/issues/12164))
* Closed a security vulnerability introduced in 0.8.0 that would potentially allow the owner of a style to compromise apps loading that style. ([#12571](https://github.com/mapbox/mapbox-gl-native/pull/12571))

## 0.9.0 - July 18, 2018

## Styles and rendering

* Added an `MLNRasterStyleLayer.rasterResamplingMode` property for configuring how raster style layers are overscaled. ([#12176](https://github.com/mapbox/mapbox-gl-native/pull/12176))
* `-[MLNStyle localizeLabelsIntoLocale:]` and `-[NSExpression mgl_expressionLocalizedIntoLocale:]` can automatically localize labels into Japanese or Korean based on the system’s language settings. ([#12286](https://github.com/mapbox/mapbox-gl-native/pull/12286))
* The `c` and `d` options are supported within comparison predicates for case and diacritic insensitivity, respectively. ([#12329](https://github.com/mapbox/mapbox-gl-native/pull/12329))
* Added the `collator` and `resolved-locale` expression operators to more precisely compare strings in style JSON. A subset of this functionality is available through predicate options when creating an `NSPredicate`. ([#11869](https://github.com/mapbox/mapbox-gl-native/pull/11869))
* Fixed a crash in `-[MLNStyle localizeLabelsIntoLocale:]` on macOS 10.11. ([#12123](https://github.com/mapbox/mapbox-gl-native/pull/12123))
* Fixed a crash that occurred when creating an `MLN_MATCH` expression using non-expressions as arguments. ([#12332](https://github.com/mapbox/mapbox-gl-native/pull/12332))
* Fixed a crash when trying to parse expressions containing legacy filters. ([#12263](https://github.com/mapbox/mapbox-gl-native/pull/12263))

## Other changes

* Added `-[MLNMapView camera:fittingShape:edgePadding:]` and `-[MLNMapView camera:fittingCoordinateBounds:edgePadding:]` allowing you specify the pitch and direction for the calculated camera. ([#12213](https://github.com/mapbox/mapbox-gl-native/pull/12213))
* Added `-[MLNMapSnapshot coordinateForPoint:]` that returns a map coordinate for a specified snapshot image point. ([#12221](https://github.com/mapbox/mapbox-gl-native/pull/12221))
* Fixed an issue where `-[MLNMapShapshot pointForCoordinate:]` returned incorrect points. ([#12221](https://github.com/mapbox/mapbox-gl-native/pull/12221))
* Improved caching performance. ([#12072](https://github.com/mapbox/mapbox-gl-native/pull/12072))
* Remove unnecessary memory use when collision debug mode is disabled. ([#12294](https://github.com/mapbox/mapbox-gl-native/issues/12294))

## 0.8.0 - June 20, 2018

### Packaging

* The minimum deployment target for this SDK is now macOS 10.11.0. ([#11776](https://github.com/mapbox/mapbox-gl-native/pull/11776))
* Fixed an issue where `MLNMapView` produced a designable error in Interface Builder storyboards. ([#12140](https://github.com/mapbox/mapbox-gl-native/pull/12140))

### Styles and rendering

* Added support for aggregate expressions as input values to `MLN_MATCH` expressions. ([#11866](https://github.com/mapbox/mapbox-gl-native/pull/11866))
* Fixed a crash that occurred when style JSON contained an invalid filter containing an expression. ([#12065](https://github.com/mapbox/mapbox-gl-native/pull/12065))
* Fixed a crash in `-[MLNStyle localizeLabelsIntoLocale:]` on macOS 10.11. ([#12123](https://github.com/mapbox/mapbox-gl-native/pull/12123))
* Unknown tokens in URLs are now preserved, rather than replaced with an empty string. ([#11787](https://github.com/mapbox/mapbox-gl-native/issues/11787))
* Fixed an issue preventing nested key path expressions from accessing the correct feature attributes. ([#11959](https://github.com/mapbox/mapbox-gl-native/pull/11959))
* Fixed an issue where `MLNSymbolStyleLayer` flickered when straddling the antimeridian. ([#11938](https://github.com/mapbox/mapbox-gl-native/pull/11938))

### Other changes

* Adjusted when and how the camera transition update and finish callbacks are called, fixing recursion bugs. ([#11614](https://github.com/mapbox/mapbox-gl-native/pull/11614))
* Fixed a crash that could occur when reusing `MLNMapSnapshotter` or using multiple snapshotters at the same time. ([#11831](https://github.com/mapbox/mapbox-gl-native/pull/11831))
* Fixed an issue where an empty `MLNFeature` array caused high CPU utilization. ([#11985](https://github.com/mapbox/mapbox-gl-native/pull/11985))
* Improved offline download performance. ([#11284](https://github.com/mapbox/mapbox-gl-native/pull/11284))
* Fixed an issue that caused -[MLNMapView visibleFeaturesAtPoint:] to return an empty array when adding or removing features. ([#12076](https://github.com/mapbox/mapbox-gl-native/pull/12076))

## 0.7.1 - May 15, 2018

### Style layers

* Deprecated `+[NSExpression featurePropertiesVariableExpression]`; use `+[NSExpression featureAttributesVariableExpression]` instead. ([#11748](https://github.com/mapbox/mapbox-gl-native/pull/11748))
* Added an `-[NSPredicate(MLNAdditions) predicateWithMLNJSONObject:]` method and `NSPredicate.mgl_jsonExpressionObject` property. ([#11810](https://github.com/mapbox/mapbox-gl-native/pull/11810))
* Added `FIRST`, `LAST`, and `SIZE` symbolic array subscripting support to expressions. ([#11770](https://github.com/mapbox/mapbox-gl-native/pull/11770))
* Inside an expression, casting `nil` to a string turns it into the empty string instead of the string `"null"`. ([#11904](https://github.com/mapbox/mapbox-gl-native/pull/11904))
* Fixed an issue where certain colors were being misrepresented in `NSExpression` obtained from `MLNStyleLayer` getters. ([#11725](https://github.com/mapbox/mapbox-gl-native/pull/11725))

### Annotations

* Fixed an issue where selecting an onscreen annotation could move the map unintentionally. ([#11731](https://github.com/mapbox/mapbox-gl-native/pull/11731))
* Fixed an issue where an `MLNOverlay` object straddling the antimeridian had an empty `MLNOverlay.overlayBounds` value. ([#11783](https://github.com/mapbox/mapbox-gl-native/pull/11783))

### Other changes

* Reduced per-frame render CPU time. ([#11811](https://github.com/mapbox/mapbox-gl-native/issues/11811))
* Fixed a crash when removing an `MLNOfflinePack`. ([#6092](https://github.com/mapbox/mapbox-gl-native/issues/6092))
* If English is the first language listed in the user’s Preferred Languages setting, `-[MLNStyle localizeLabelsIntoLocale:]` no longer prioritizes other languages over English. ([#11907](https://github.com/mapbox/mapbox-gl-native/pull/11907))

## 0.7.0 - April 19, 2018

The 0.7._x_ series of releases will be the last to support macOS 10.10. The minimum macOS deployment version will increase to macOS 10.11.0 in a future release.

### Packaging

* Added Arabic, Danish, Hebrew, and European Portuguese localizations. ([#10967](https://github.com/mapbox/mapbox-gl-native/pull/10967), [#11136](https://github.com/mapbox/mapbox-gl-native/pull/11134), [#11695](https://github.com/mapbox/mapbox-gl-native/pull/11695))
* Removed methods, properties, and constants that had been deprecated as of v0.6.1. ([#11205](https://github.com/mapbox/mapbox-gl-native/pull/11205))
* Refined certain Swift interfaces by converting them from class methods to class properties. ([#11674](https://github.com/mapbox/mapbox-gl-native/pull/11674))

### Style layers

* The layout and paint properties on subclasses of `MLNStyleLayer` are now of type `NSExpression` instead of `MLNStyleValue`. A new “Predicates and Expressions” guide provides an overview of the supported operators, which include arithmetic and conditional operators. ([#10726](https://github.com/mapbox/mapbox-gl-native/pull/10726))
* A style can now display a heatmap layer that visualizes a point data distribution. You can customize the appearance at runtime using the `MLNHeatmapStyleLayer` class. ([#11046](https://github.com/mapbox/mapbox-gl-native/pull/11046))
* A style can now display a smooth hillshading layer and customize its appearance at runtime using the `MLNHillshadeStyleLayer` class. Hillshading is based on a rasterized digital elevation model supplied by the `MLNRasterDEMSource` class. ([#10642](https://github.com/mapbox/mapbox-gl-native/pull/10642))
* You can now set the `MLNVectorStyleLayer.predicate` property to a predicate that contains arithmetic and calls to built-in `NSExpression` functions. You may need to cast a feature attribute key to `NSString` or `NSNumber` before comparing it to a string or number. ([#11587](https://github.com/mapbox/mapbox-gl-native/pull/11587))
* Replaced the `MLNStyle.localizesLabels` property with an `-[MLNStyle localizeLabelsIntoLocale:]` method that allows you to specify the language to localize into. Note that this method does not automatically update the style when the system’s preferred language changes. Also added an `-[NSExpression(MLNAdditions) mgl_expressionLocalizedIntoLocale:]` method for localizing an individual value used with `MLNSymbolStyleLayer.text`. ([#11651](https://github.com/mapbox/mapbox-gl-native/pull/11651))
* Fixed incorrect color calibration on macOS 10.13 High Sierra when using color-related methods of `MLNStyleLayer` subclasses, as well as when displaying an `MLNAttributionInfo`. It is no longer necessary to explicitly convert an `NSColor` to the sRGB color space before using these classes on High Sierra. ([#11391](https://github.com/mapbox/mapbox-gl-native/pull/11391))
* The `MLNSymbolStyleLayer.textFontNames` property can now depend on a feature’s attributes. ([#10850](https://github.com/mapbox/mapbox-gl-native/pull/10850))
* Changes to the `MLNStyleLayer.minimumZoomLevel` and `MLNStyleLayer.maximumZoomLevel` properties take effect immediately. ([#11399](https://github.com/mapbox/mapbox-gl-native/pull/11399))

### Content sources

* Renamed `MLNRasterSource` to `MLNRasterTileSource` and `MLNVectorSource` to `MLNVectorTileSource`. ([#11568](https://github.com/mapbox/mapbox-gl-native/pull/11568))
* Added an `MLNComputedShapeSource` class that allows applications to supply vector data to a style layer on a per-tile basis. ([#9983](https://github.com/mapbox/mapbox-gl-native/pull/9983))
* Properties such as `MLNSymbolStyleLayer.iconAllowsOverlap` and `MLNSymbolStyleLayer.iconIgnoresPlacement` now account for symbols in other sources. ([#10436](https://github.com/mapbox/mapbox-gl-native/pull/10436))

### Map rendering

* Improved the reliability of collision detection between symbols near the edges of tiles, as well as between symbols when the map is tilted. It is no longer necessary to enable `MLNSymbolStyleLayer.symbolAvoidsEdges` to prevent symbols in adjacent tiles from overlapping with each other. ([#10436](https://github.com/mapbox/mapbox-gl-native/pull/10436))
* Symbols can fade in and out as the map pans, rotates, or tilts. ([#10436](https://github.com/mapbox/mapbox-gl-native/pull/10436))
* Properties such as `MLNSymbolStyleLayer.iconAllowsOverlap` and `MLNSymbolStyleLayer.iconIgnoresPlacement` now account for symbols in other sources. ([#10436](https://github.com/mapbox/mapbox-gl-native/pull/10436))
* Added the `MLNTileSourceOptionTileCoordinateBounds` option to create an `MLNTileSource` that only supplies tiles within a specific geographic bounding box. ([#11141](https://github.com/mapbox/mapbox-gl-native/pull/11141))
* Fixed an issue preventing a dynamically-added `MLNRasterStyleLayer` from drawing until the map pans. ([#10270](https://github.com/mapbox/mapbox-gl-native/pull/10270))
* Fixed an issue preventing `MLNImageSource`s from drawing on the map when the map is zoomed in and tilted. ([#10677](https://github.com/mapbox/mapbox-gl-native/pull/10677))
* Improved the sharpness of raster tiles on Retina displays. ([#10984](https://github.com/mapbox/mapbox-gl-native/pull/10984))
* Fixed a crash parsing a malformed style. ([#11001](https://github.com/mapbox/mapbox-gl-native/pull/11001))
* Fixed an issue where symbols with empty labels would always be hidden. ([#11206](https://github.com/mapbox/mapbox-gl-native/pull/11206))
* Fixed an issue where a tilted map could flicker while displaying rotating symbols. ([#11488](https://github.com/mapbox/mapbox-gl-native/pull/11488))
* Increased the maximum width of labels by a factor of two. ([#11508](https://github.com/mapbox/mapbox-gl-native/pull/11508))

### Annotations

* Fixed an issue where tapping a group of annotations may not have selected the nearest annotation. ([#11438](https://github.com/mapbox/mapbox-gl-native/pull/11438))
* The `MLNMapView.selectedAnnotations` property (backed by `-[MLNMapView setSelectedAnnotations:]`) now selects annotations that are off-screen. ([#9790](https://github.com/mapbox/mapbox-gl-native/issues/9790))
* The `animated` parameter to `-[MLNMapView selectAnnotation:animated:]` now controls whether the annotation and its callout are brought on-screen. If `animated` is `NO` then the annotation is selected if offscreen, but the map is not panned. Currently only point annotations are supported.([#3249](https://github.com/mapbox/mapbox-gl-native/issues/3249))
* Fixed a crash when rapidly adding and removing annotations. ([#11551](https://github.com/mapbox/mapbox-gl-native/issues/11551), [#11575](https://github.com/mapbox/mapbox-gl-native/issues/11575))

### Map snapshots

* Fixed a memory leak that occurred when creating a map snapshot. ([#10585](https://github.com/mapbox/mapbox-gl-native/pull/10585))
* Fixed an issue that caused `MLNMapSnapshotter.pointForCoordinate` to return an incorrect value. ([#11035](https://github.com/mapbox/mapbox-gl-native/pull/11035))

### Other changes

* The `-[MLNMapView convertRect:toCoordinateBoundsFromView:]` method and the `MLNMapView.visibleCoordinateBounds` property’s getter now indicate that the coordinate bounds straddles the antimeridian by extending one side beyond ±180 degrees longitude. ([#11265](https://github.com/mapbox/mapbox-gl-native/pull/11265))
* Feature querying results now account for the `MLNSymbolStyleLayer.circleStrokeWidth` property. ([#10897](https://github.com/mapbox/mapbox-gl-native/pull/10897))
* Reduced offline download sizes for styles with symbol layers that render only icons, and no text. ([#11055](https://github.com/mapbox/mapbox-gl-native/pull/11055))

## v0.6.1 - January 16, 2018

This version of the Mapbox macOS SDK corresponds to version 3.7.3 of the Mapbox Maps SDK for iOS.

* Fixed a crash while zooming while annotations are present on the map. ([#10791](https://github.com/mapbox/mapbox-gl-native/pull/10791))
* CJK characters can be displayed in a locally installed font or a custom font bundled with the application, reducing map download times. Specify the font name using the `MLNIdeographicFontFamilyName` key in the application’s Info.plist file. ([#10522](https://github.com/mapbox/mapbox-gl-native/pull/10522))

## v0.6.0 - December 23, 2017

This version of the Mapbox macOS SDK corresponds to version 3.7.2 of the Mapbox Maps SDK for iOS.

### Packaging

* Renamed this SDK from Mapbox macOS SDK to Mapbox Maps SDK for macOS. ([#10610](https://github.com/mapbox/mapbox-gl-native/pull/10610), [#10793](https://github.com/mapbox/mapbox-gl-native/pull/10793))
* Added a Bulgarian localization. ([#10309](https://github.com/mapbox/mapbox-gl-native/pull/10309))

### Networking and storage

* Added a new `MLNMapSnapshotter` class for capturing rendered map images from an `MLNMapView`’s camera. ([#9891](https://github.com/mapbox/mapbox-gl-native/pull/9891))
* Reduced the time it takes to create new `MLNMapView` instances in some cases. ([#9864](https://github.com/mapbox/mapbox-gl-native/pull/9864))
* Added support for forced cache revalidation that will eliminate flickering that was sometimes visible for certain types of tiles (e.g., traffic tiles). ([#9670](https://github.com/mapbox/mapbox-gl-native/pull/9670), [#9103](https://github.com/mapbox/mapbox-gl-native/issues/9103))
* Improved the performance of the SDK when parsing vector tile data used to render the map. ([#9312](https://github.com/mapbox/mapbox-gl-native/pull/9312))

### Styles

* Added a new type of source, represented by the `MLNImageSource` class at runtime, that displays a georeferenced image. ([#9110](https://github.com/mapbox/mapbox-gl-native/pull/9110))
* Setting a style using `MLNMapView`'s `styleURL` property now smoothly transitions from the previous style to the new style and maintains equivalent layers and sources along with their identifiers. ([#9256](https://github.com/mapbox/mapbox-gl-native/pull/9256))
* Added `MLNCircleStyleLayer.circlePitchAlignment` and `MLNSymbolStyleLayer.iconPitchAlignment` properties to control whether circles and symbols lie flat against a tilted map. ([#9426](https://github.com/mapbox/mapbox-gl-native/pull/9426), [#9479](https://github.com/mapbox/mapbox-gl-native/pull/9479))
* Added an `MLNSymbolStyleLayer.iconAnchor` property to control where an icon is anchored. ([#9849](https://github.com/mapbox/mapbox-gl-native/pull/9849))
* The `maximumTextWidth` and `textLetterSpacing` properties of `MLNSymbolStyleLayer` are now compatible with `MLNSourceStyleFunction`s and `MLNCompositeStyleFunction`s, allowing data-driven styling of these properties. ([#9870](https://github.com/mapbox/mapbox-gl-native/pull/9870))
* Improved the legibility of labels that follow lines when the map is tilted. ([#9009](https://github.com/mapbox/mapbox-gl-native/pull/9009))
* Fixed an issue that could cause flickering when a translucent raster style layer was present. ([#9468](https://github.com/mapbox/mapbox-gl-native/pull/9468))
* Fixed an issue that could cause antialiasing between polygons on the same layer to fail if the fill layers used data-driven styling for the fill color. ([#9699](https://github.com/mapbox/mapbox-gl-native/pull/9699))
* The previously deprecated support for style classes has been removed. For interface compatibility, the API methods remain, but they are now non-functional.

### Annotations and user interaction

* Fixed several bugs and performance issues related to the use of annotations backed by `MLNAnnotationImage`s. The limits on the number and size of images and glyphs has been effectively eliminated and should now depend on hardware constraints. These fixes also apply to images used to represent icons in `MLNSymbolStyleLayer`s. ([#9213](https://github.com/mapbox/mapbox-gl-native/pull/9213))
* Increased the default maximum zoom level from 20 to 22. ([#9835](https://github.com/mapbox/mapbox-gl-native/pull/9835))
* Added an `overlays` property to `MLNMapView`. ([#8617](https://github.com/mapbox/mapbox-gl-native/pull/8617))
* Fixed incorrect hit targets for `MLNAnnotationImage`-backed annotations that caused `-[MLNMapViewDelegate mapView:didSelectAnnotation:]` to be called unnecessarily. ([#10538](https://github.com/mapbox/mapbox-gl-native/pull/10538))
* Added `-[MLNMapView cameraThatFitsShape:direction:edgePadding:]` to get a camera with zoom level and center coordinate computed to fit a shape. ([#10107](https://github.com/mapbox/mapbox-gl-native/pull/10107))
* Added support selection of shape and polyline annotations.([#9984](https://github.com/mapbox/mapbox-gl-native/pull/9984))
* Fixed an issue where a shape annotation callout was not displayed if the centroid was not visible. ([#10255](https://github.com/mapbox/mapbox-gl-native/pull/10255))

### Other changes

* Fixed distortion in the logo view on macOS 10.13 High Sierra. ([#10606](https://github.com/mapbox/mapbox-gl-native/pull/10606))
* Fixed an issue that could cause line label rendering glitches when the line geometry is projected to a point behind the plane of the camera. ([#9865](https://github.com/mapbox/mapbox-gl-native/pull/9865))
* Fixed an issue that could cause a crash when using `-[MLNMapView flyToCamera:completionHandler:]` and related methods with zoom levels at or near the maximum value. ([#9381](https://github.com/mapbox/mapbox-gl-native/pull/9381))
* Fixed an issue where removing a `MLNOpenGLStyleLayer` from a map might result in a crash. ([#10765](https://github.com/mapbox/mapbox-gl-native/pull/10765))
* Added documentation for usage of coordinate bounds that cross the anti-meridian. ([#10783](https://github.com/mapbox/mapbox-gl-native/pull/10783))
* Removed duplicated variables in `MLNMapSnapshotter`. ([#10702](https://github.com/mapbox/mapbox-gl-native/pull/10702))

## 0.5.1 - September 26, 2017

This version of the Mapbox macOS SDK corresponds to version 3.6.4 of the Mapbox iOS SDK.

* Added an `MLNStyle.localizesLabels` property, off by default, that localizes any Mapbox Streets–sourced symbol layer into the user’s preferred language. ([#9582](https://github.com/mapbox/mapbox-gl-native/pull/9582))
* Fixed an issue that caused `-[MLNShapeSource featuresMatchingPredicate:]` and `-[MLNVectorSource featuresInSourceLayersWithIdentifiers:predicate:]` to always return an empty array. ([#9784](https://github.com/mapbox/mapbox-gl-native/pull/9784))
* `MLNMapView`’s `minimumZoomLevel` and `maximumZoomLevel` properties are now available in Interface Builder’s Attributes inspector. ([#9729](https://github.com/mapbox/mapbox-gl-native/pull/9729))
* Added a Hungarian localization. ([#9945](https://github.com/mapbox/mapbox-gl-native/pull/9945))
* Deprecated `+[MLNStyle trafficDayStyleURL]` and `+[MLNStyle trafficNightStyleURL]` with no replacement method. To use the Traffic Day and Traffic Night styles going forward, we recommend that you use the underlying URL. ([#9918](https://github.com/mapbox/mapbox-gl-native/pull/9918))
* Fixed an issue where stale (but still valid) map data could be ignored in offline mode. ([#10012](https://github.com/mapbox/mapbox-gl-native/pull/10012))

## 0.5.0 - June 30, 2017

This version of the Mapbox macOS SDK corresponds to version 3.6.0 of the Mapbox iOS SDK.

### Packaging

* Xcode 8.0 or higher is now recommended for using this SDK. ([#8775](https://github.com/mapbox/mapbox-gl-native/pull/8775))
* Updated MLNMapView’s logo view to display [the new Mapbox logo](https://www.mapbox.com/blog/new-mapbox-logo/). ([#8771](https://github.com/mapbox/mapbox-gl-native/pull/8771), [#8773](https://github.com/mapbox/mapbox-gl-native/pull/8773))

### Styles

* Added support for 3D extrusion of buildings and other polygonal features via the `MLNFillExtrusionStyleLayer` class and the `fill-extrusion` layer type in style JSON. ([#8431](https://github.com/mapbox/mapbox-gl-native/pull/8431))
* MLNMapView and MLNTilePyramidOfflineRegion now default to version 10 of the Mapbox Streets style. Similarly, several style URL class methods of MLNStyle return URLs to version 10 styles. Unversioned variations of these methods are no longer deprecated. `MLNStyleDefaultVersion` should no longer be used with any style other than Streets. ([#6301](https://github.com/mapbox/mapbox-gl-native/pull/6301))
* Added class methods to MLNStyle that correspond to the new [Traffic Day and Traffic Night](https://www.mapbox.com/blog/live-traffic-maps/) styles. ([#6301](https://github.com/mapbox/mapbox-gl-native/pull/6301))
* MLNSymbolStyleLayer’s `iconImageName`, `iconScale`, `textFontSize`, `textOffset`, and `textRotation` properties can now be set to a source or composite function. ([#8544](https://github.com/mapbox/mapbox-gl-native/pull/8544), [#8590](https://github.com/mapbox/mapbox-gl-native/pull/8590), [#8592](https://github.com/mapbox/mapbox-gl-native/pull/8592), [#8593](https://github.com/mapbox/mapbox-gl-native/pull/8593))
* Fixed an issue where setting the `MLNVectorStyleLayer.predicate` property failed to take effect if the relevant source was not in use by a visible layer at the time. ([#8653](https://github.com/mapbox/mapbox-gl-native/pull/8653))
* Fixed an issue preventing programmatically added style layers from appearing in already cached tiles. ([#8954](https://github.com/mapbox/mapbox-gl-native/pull/8954))
* Fixed an issue causing a composite function’s highest zoom level stop to be misinterpreted. ([#8613](https://github.com/mapbox/mapbox-gl-native/pull/8613), [#8790](https://github.com/mapbox/mapbox-gl-native/pull/8790))
* Fixed an issue where re-adding a layer that had been previously removed from a style would reset its paint properties. Moved initializers for `MLNTileSource`, `MLNStyleLayer`, and `MLNForegroundStyleLayer` to their concrete subclasses; because these classes were already intended for initialization only via concrete subclasses, this should have no developer impact. ([#8626](https://github.com/mapbox/mapbox-gl-native/pull/8626))
* Fixed a crash that occurred when removing a source that was still being used by one or more style layers. Since this is a programming error, a warning is logged to the console instead. ([#9129](https://github.com/mapbox/mapbox-gl-native/pull/9129))
* Feature querying results now account for any changes to a feature’s size caused by a source or composite style function. ([#8665](https://github.com/mapbox/mapbox-gl-native/pull/8665))
* Fixed the behavior of composite functions that specify fractional zoom level stops. ([#9289](https://github.com/mapbox/mapbox-gl-native/pull/9289))
* Letter spacing is now disabled in Arabic text so that ligatures are drawn correctly. ([#9062](https://github.com/mapbox/mapbox-gl-native/pull/9062))
* Improved the performance of styles using source and composite style functions. ([#9185](https://github.com/mapbox/mapbox-gl-native/pull/9185), [#9257](https://github.com/mapbox/mapbox-gl-native/pull/9257))

### Annotations

* The default marker image has been made slightly larger and now matches the version in the Mapbox iOS SDK. ([#9370](https://github.com/mapbox/mapbox-gl-native/pull/9370))
* The `MLNPolyline.coordinate` and `MLNPolygon.coordinate` properties now return the midpoint and centroid, respectively, instead of the first coordinate. ([#8713](https://github.com/mapbox/mapbox-gl-native/pull/8713))

### User interaction

* Fixed an issue causing the map to go blank during a flight animation that travels a very short distance. ([#9199](https://github.com/mapbox/mapbox-gl-native/pull/9199))
* Fixed an issue causing the mouse cursor to jump after Shift- or Option-dragging the map. ([#9390](https://github.com/mapbox/mapbox-gl-native/pull/9390))
* The Improve This Map button in the attribution action sheet now leads to a feedback tool that matches MLNMapView’s rotation and pitch. `-[MLNAttributionInfo feedbackURLAtCenterCoordinate:zoomLevel:]` no longer respects the feedback URL specified in TileJSON. ([#9078](https://github.com/mapbox/mapbox-gl-native/pull/9078))

### Other changes

* Fixed a crash when calling `MLNMultiPolygon.coordinate` [#8713](https://github.com/mapbox/mapbox-gl-native/pull/8713)
* Fixed an issue causing attribution button text to appear blue instead of black. ([#8701](https://github.com/mapbox/mapbox-gl-native/pull/8701))
* Fixed a crash or console spew when MLNMapView is initialized with a frame smaller than 64 points wide by 64 points tall. ([#8562](https://github.com/mapbox/mapbox-gl-native/pull/8562))
* The error passed into `-[MLNMapViewDelegate mapViewDidFailLoadingMap:withError:]` now includes a more specific description and failure reason. ([#8418](https://github.com/mapbox/mapbox-gl-native/pull/8418))
* Improved CPU and battery performance while animating a tilted map’s camera in an area with many labels. ([#9031](https://github.com/mapbox/mapbox-gl-native/pull/9031))
* Fixed an issue rendering polylines that contain duplicate vertices. ([#8808](https://github.com/mapbox/mapbox-gl-native/pull/8808))
* Added struct boxing to `MLNCoordinateSpan`, `MLNCoordinateBounds`, `MLNOfflinePackProgress`, and `MLNTransition`. ([#9343](https://github.com/mapbox/mapbox-gl-native/pull/9343))

## 0.4.1 - April 8, 2017

This version of the Mapbox macOS SDK corresponds to version 3.5.2 of the Mapbox iOS SDK.

* Fixed an issue causing code signing failures and bloating the framework. ([#8640](https://github.com/mapbox/mapbox-gl-native/pull/8640))
* Fixed an issue that could cause a crash if annotations unknown to the map view were interacted with. ([#8686](https://github.com/mapbox/mapbox-gl-native/pull/8686))
* Renamed the “Data-Driven Styling” guide to “Using Style Functions at Runtime” and clarified the meaning of data-driven styling in the guide’s discussion of runtime style functions. ([#8627](https://github.com/mapbox/mapbox-gl-native/pull/8627))

## 0.4.0 - April 2, 2017

This version of the Mapbox macOS SDK corresponds to version 3.5.1 of the Mapbox iOS SDK.

### Internationalization

* Added support for right-to-left text and Arabic ligatures in labels. ([#6984](https://github.com/mapbox/mapbox-gl-native/pull/6984), [#7123](https://github.com/mapbox/mapbox-gl-native/pull/7123))
* Improved the line wrapping behavior of point-placed labels, especially labels written in Chinese and Japanese. ([#6828](https://github.com/mapbox/mapbox-gl-native/pull/6828), [#7446](https://github.com/mapbox/mapbox-gl-native/pull/7446))
* CJK characters now remain upright in vertically oriented labels that have line placement, such as road labels. ([#7114](https://github.com/mapbox/mapbox-gl-native/issues/7114))
* Added Catalan, Chinese (Simplified and Traditional), Dutch, Finnish, French, German, Japanese, Lithuanian, Polish, Portuguese (Brazilian), Spanish, Swedish, Ukrainian, and Vietnamese localizations. ([#7316](https://github.com/mapbox/mapbox-gl-native/pull/7316), [#7503](https://github.com/mapbox/mapbox-gl-native/pull/7503), [#7899](https://github.com/mapbox/mapbox-gl-native/pull/7899), [#7999](https://github.com/mapbox/mapbox-gl-native/pull/7999), [#8113](https://github.com/mapbox/mapbox-gl-native/pull/8113), [#8256](https://github.com/mapbox/mapbox-gl-native/pull/8256))

### Styles

* Added support for data-driven styling in the form of source and composite style functions. `MLNStyleFunction` is now an abstract class, with `MLNCameraStyleFunction` providing the behavior of `MLNStyleFunction` in previous releases. New `MLNStyleFunction` subclasses allow you to vary a style attribute by the values of attributes of features in the source. ([#7596](https://github.com/mapbox/mapbox-gl-native/pull/7596))
* Added methods to MLNShapeSource and MLNVectorSource for querying features loaded by the source, whether or not they’re visible on the map. ([#8263](https://github.com/mapbox/mapbox-gl-native/pull/8263))
* Added `circleStrokeColor`, `circleStrokeWidth`, and `circleStrokeOpacity` properties to MLNCircleStyleLayer and support for corresponding properties in style JSON files. ([#7356](https://github.com/mapbox/mapbox-gl-native/pull/7356))
* Point-placed labels in symbol style layers are now placed at more optimal locations within polygons. ([#7465](https://github.com/mapbox/mapbox-gl-native/pull/7465))
* Fixed flickering that occurred when manipulating a style layer. ([#7616](https://github.com/mapbox/mapbox-gl-native/pull/7616))
* Symbol style layers can now render point collections (known as multipoints in GeoJSON). ([#7445](https://github.com/mapbox/mapbox-gl-native/pull/7445))
* Added a `transition` property to MLNStyle to customize the timing of changes to style layers. ([#7711](https://github.com/mapbox/mapbox-gl-native/pull/7711))
* Added properties to MLNStyleLayer subclasses to customize the timing of transitions between values of individual attributes. ([#8225](https://github.com/mapbox/mapbox-gl-native/pull/8225))
* Fixed an issue causing lines and text labels toward the top of the map view to appear blurry when the map is tilted. ([#7444](https://github.com/mapbox/mapbox-gl-native/pull/7444))
* Fixed incorrect interpolation of style functions in Boolean-typed style attributes. ([#7526](https://github.com/mapbox/mapbox-gl-native/pull/7526))
* Removed support for the `ref` property in layers in style JSON files. ([#7586](https://github.com/mapbox/mapbox-gl-native/pull/7586))
* Fixed an issue that collapsed consecutive newlines within text labels. ([#7446](https://github.com/mapbox/mapbox-gl-native/pull/7446))
* Fixed artifacts when drawing particularly acute line joins. ([#7786](https://github.com/mapbox/mapbox-gl-native/pull/7786))
* Fixed an issue in which a vector style layer predicate involving the `$id` key path would exclude all features from the layer. ([#7989](https://github.com/mapbox/mapbox-gl-native/pull/7989), [#7971](https://github.com/mapbox/mapbox-gl-native/pull/7971))
* Fixed an issue causing vector style layer predicates to be evaluated as if each feature had a `$type` attribute of 1, 2, or 3. The `$type` key path can now be compared to `Point`, `LineString`, or `Polygon`, as described in the documentation. ([#7971](https://github.com/mapbox/mapbox-gl-native/pull/7971))
* When setting an `MLNShapeSource`’s shape to an `MLNFeature` instance, any `NSColor` attribute value is now converted to the equivalent CSS string representation for use with `MLNInterpolationModeIdentity` in style functions. ([#8025](https://github.com/mapbox/mapbox-gl-native/pull/8025))
* An exception is no longer thrown if layers or sources are removed from a style before they are added. ([#7962](https://github.com/mapbox/mapbox-gl-native/pull/7962))
* Renamed MLNStyleConstantValue to MLNConstantStyleValue. For compatibility with previous releases, MLNStyleConstantValue is now an alias of MLNConstantStyleValue. ([#8090](https://github.com/mapbox/mapbox-gl-native/pull/8090))
* Fixed a crash that could occur when switching styles after adding an MLNSource to the style. ([#8298](https://github.com/mapbox/mapbox-gl-native/pull/8298))

### Annotations and user interaction

* Added a method to MLNMapViewDelegate, `-mapView:shouldChangeFromCamera:toCamera:`, that you can implement to restrict which parts the user can navigate to using gestures. ([#5584](https://github.com/mapbox/mapbox-gl-native/pull/5584))
* When a map view is the first responder, pressing <kbd>+</kbd>, <kbd>-</kbd>, or <kbd>=</kbd> now zooms the map. ([#8033](https://github.com/mapbox/mapbox-gl-native/pull/8033))
* Changing the coordinates of a point annotation no longer deselects the annotation. ([#8269](https://github.com/mapbox/mapbox-gl-native/pull/8269))
* Fixed an issue that could cause a crash when point annotations were added and removed while simultaneously querying source features. ([#8374](https://github.com/mapbox/mapbox-gl-native/pull/8374))
* Fixed an issue preventing MLNMapView from adding a polyline annotation with the same coordinates as a polygon annotation. ([#8355](https://github.com/mapbox/mapbox-gl-native/pull/8355))
* Zooming by double-tap, two-finger tap, zoom buttons, shortcut keys, or demo app menu items or shortcut keys now zooms to the nearest integer zoom level. ([#8027](https://github.com/mapbox/mapbox-gl-native/pull/8027))
* Fixed an issue where translucent point annotations along tile boundaries would be drawn darker than expected. ([#6832](https://github.com/mapbox/mapbox-gl-native/pull/6832))

### Networking and offline maps

* Offline pack notifications are now posted by `MLNOfflinePack` instances instead of the shared `MLNOfflineStorage` object. For backwards compatibility, the `userInfo` dictionary still indicates the pack’s state and progress. ([#7952](https://github.com/mapbox/mapbox-gl-native/pull/7952))
* Fixed a memory leak in MLNMapView. ([#7956](https://github.com/mapbox/mapbox-gl-native/pull/7956))
* Fixed an issue that could prevent a cached style from appearing while the computer is offline. ([#7770](https://github.com/mapbox/mapbox-gl-native/pull/7770))
* Fixed an issue that could prevent a style from loading when reestablishing a network connection. ([#7902](https://github.com/mapbox/mapbox-gl-native/pull/7902))
* `MLNOfflineStorage` instances now support a delegate conforming to `MLNOfflineStorageDelegate`, which allows altering URLs before they are requested from the Internet. ([#8084](https://github.com/mapbox/mapbox-gl-native/pull/8084))

### Other changes

* Added support for the Carthage dependency manager. See [this SDK’s homepage](https://mapbox.github.io/mapbox-gl-native/macos/) for setup instructions. ([#8257](https://github.com/mapbox/mapbox-gl-native/pull/8257))
* Fixed an issue that, among other things, caused various islands to disappear at certain zoom levels. ([#7621](https://github.com/mapbox/mapbox-gl-native/pull/7621))
* Added a method to MLNMapView that allows you to specify a predicate when querying for visible features. ([#8256](https://github.com/mapbox/mapbox-gl-native/pull/8246))
* Fixed flickering that occurred when panning past the antimeridian. ([#7574](https://github.com/mapbox/mapbox-gl-native/pull/7574))
* Added a `MLNDistanceFormatter` class for formatting geographic distances. ([#7888](https://github.com/mapbox/mapbox-gl-native/pull/7888))

## 0.3.1 - February 21, 2017

This version of the Mapbox macOS SDK corresponds to version 3.4.2 of the Mapbox iOS SDK.

* Fixed an issue causing MLNMapView’s `camera`’s `heading` to be set to a negative value, indicating an undefined heading, when the map view faces northwest. The heading is now wrapped to between zero and 360 degrees, for consistency with MLNMapView’s `direction` property. ([#7724](https://github.com/mapbox/mapbox-gl-native/pull/7724))
* Fixed a crash that occurred when moving a window containing an MLNMapView from one screen to another. ([#8004](https://github.com/mapbox/mapbox-gl-native/pull/8004))
* Fixed an issue preventing the use of the integrated GPU on machines that have more than one GPU. Follow the instructions in [Technical Q&A 1734](https://developer.apple.com/library/content/qa/qa1734/_index.html) to enable integrated GPU usage in your application. ([#7834](https://github.com/mapbox/mapbox-gl-native/pull/7834))
* Fixed an issue causing the mouse cursor to jump after shift- or option-dragging a map view if the window opened on a screen with a different size than the screen with keyboard focus. ([#7846](https://github.com/mapbox/mapbox-gl-native/pull/7846))
* Deprecated the style class methods in MLNStyle. ([#7785](https://github.com/mapbox/mapbox-gl-native/pull/7785))
* Improved the performance of trivial camera animations. ([#7125](https://github.com/mapbox/mapbox-gl-native/pull/7125))

## 0.3.0 - January 21, 2016

This version of the Mapbox macOS SDK corresponds to version 3.4.0 of the Mapbox iOS SDK. The two SDKs have very similar feature sets. The main differences are the lack of user location tracking and annotation views. Some APIs have been adapted to macOS conventions, particularly the use of NSPopover for callout views.

### Packaging

* Fixed an issue causing code signing failures and bloating the framework. ([#5850](https://github.com/mapbox/mapbox-gl-native/pull/5850))
* Xcode 7.3 or higher is now required for using this SDK. ([#6059](https://github.com/mapbox/mapbox-gl-native/issues/6059))
* Fixed an issue with symbols not being properly stripped from the dynamic framework when built with `make xpackage SYMBOLS=NO`. ([#6531](https://github.com/mapbox/mapbox-gl-native/pull/6531))
* The API reference has a sharper look. ([#7422](https://github.com/mapbox/mapbox-gl-native/pull/7422))
* Added documentation for the Info.plist keys used by this SDK. ([#6833](https://github.com/mapbox/mapbox-gl-native/pull/6833))

### Styles and data

* A new runtime styling API allows you to adjust the style and content of the base map dynamically. All the options available in [Mapbox Studio](https://www.mapbox.com/studio/) are now exposed via MLNStyle and subclasses of MLNStyleLayer and MLNSource. ([#5727](https://github.com/mapbox/mapbox-gl-native/pull/5727))
* MLNMapView’s `styleURL` property can now be set to an absolute file URL. ([#6026](https://github.com/mapbox/mapbox-gl-native/pull/6026))
* When creating an MLNShapeSource, you can now specify options for clustering point features within the shape source. Similarly, GeoJSON sources specified by the stylesheet at design time can specify the `cluster`, `clusterMaxZoom`, and `clusterRadius` attributes. ([#5724](https://github.com/mapbox/mapbox-gl-native/pull/5724))
* When creating an MLNTileSource, you can now specify that the tile URLs use [TMS](https://en.wikipedia.org/wiki/Tile_Map_Service) coordinates by setting `MLNTileSourceOptionTileCoordinateSystem` to `MLNTileCoordinateSystemTMS`. TileJSON files can specify `"scheme": "tms"`. ([#2270](https://github.com/mapbox/mapbox-gl-native/pull/2270))
* Fixed an issue causing abstract `MLNMultiPointFeature` objects to be returned in feature query results. Now concrete `MLNPointCollectionFeature` objects are returned. MLNMultiPointFeature is now an alias of MLNPointCollectionFeature. ([#6742](https://github.com/mapbox/mapbox-gl-native/pull/6742))
* Fixed rendering artifacts and missing glyphs that occurred after viewing a large number of CJK characters on the map. ([#5908](https://github.com/mapbox/mapbox-gl-native/pull/5908))
* Fixed an issue where the style zoom levels were not respected when deciding when to render a layer. ([#5811](https://github.com/mapbox/mapbox-gl-native/issues/5811))
* Fixed an issue where feature querying sometimes failed to return the expected features when the map was tilted. ([#6773](https://github.com/mapbox/mapbox-gl-native/pull/6773))
* MLNFeature’s `attributes` and `identifier` properties are now writable. ([#6728](https://github.com/mapbox/mapbox-gl-native/pull/6728))
* Attribution views now display the correct attribution for the current style. ([#5999](https://github.com/mapbox/mapbox-gl-native/pull/5999))
* If MLNMapView is unable to obtain or parse a style, it now calls its delegate’s `-mapViewDidFailLoadingMap:withError:` method. ([#6145](https://github.com/mapbox/mapbox-gl-native/pull/6145))
* Added the `-[MLNMapViewDelegate mapView:didFinishLoadingStyle:]` delegate method, which offers the earliest opportunity to modify the layout or appearance of the current style before the map view is displayed to the user. ([#6636](https://github.com/mapbox/mapbox-gl-native/pull/6636))
* Fixed an issue causing stepwise zoom functions to be misinterpreted. ([#6328](https://github.com/mapbox/mapbox-gl-native/pull/6328))
* A source’s tiles are no longer rendered when the map is outside the source’s supported zoom levels. ([#6345](https://github.com/mapbox/mapbox-gl-native/pull/6345))
* Fixed crashes that could occur when loading a malformed stylesheet. ([#5736](https://github.com/mapbox/mapbox-gl-native/pull/5736))
* Improved style parsing performance. ([#6170](https://github.com/mapbox/mapbox-gl-native/pull/6170))
* Improved feature querying performance. ([#6514](https://github.com/mapbox/mapbox-gl-native/pull/6514))
* Fixed an issue where shapes that cannot currently be visually represented as annotations were still shown on the map as point annotations. ([#6764](https://github.com/mapbox/mapbox-gl-native/issues/6764))

### Annotations

* Added `showAnnotations:animated:` and `showAnnotations:edgePadding:animated:`, which moves the map viewport to show the specified annotations. ([#5749](https://github.com/mapbox/mapbox-gl-native/pull/5749))
* Added new methods to MLNMultiPoint for changing the vertices along a polyline annotation or the exterior of a polygon annotation. ([#6565](https://github.com/mapbox/mapbox-gl-native/pull/6565))
* Fixed an exception raised when adding a custom annotation model object to MLNMapView. ([#7746](https://github.com/mapbox/mapbox-gl-native/pull/7746))
* Added new APIs to MLNMapView to query for visible annotations. ([#6061](https://github.com/mapbox/mapbox-gl-native/pull/6061))
* Shape, feature, and annotation classes now conform to NSSecureCoding. ([#6559](https://github.com/mapbox/mapbox-gl-native/pull/6559))
* Various method arguments that are represented as C arrays of `CLLocationCoordinate2D` instances have been marked `const` to streamline bridging to Swift. ([#7215](https://github.com/mapbox/mapbox-gl-native/pull/7215))
* To make an MLNPolyline or MLNPolygon span the antimeridian, specify coordinates with longitudes greater than 180° or less than −180°. ([#6088](https://github.com/mapbox/mapbox-gl-native/pull/6088))
* Fixed an issue where placing a point annotation on Null Island also placed a duplicate annotation on its antipode. ([#3563](https://github.com/mapbox/mapbox-gl-native/pull/3563))
* Fixed an issue that caused an assertion failure if a `MLNShapeCollection` (a GeoJSON GeometryCollection) was created with an empty array of shapes. ([#7632](https://github.com/mapbox/mapbox-gl-native/pull/7632))
* Improved the precision of annotations at zoom levels greater than 18. ([#5517](https://github.com/mapbox/mapbox-gl-native/pull/5517))

### Networking and offline maps

* Fixed an issue preventing an MLNMapView from loading tiles while an offline pack is downloading. ([#6446](https://github.com/mapbox/mapbox-gl-native/pull/6446))
* Fixed an issue causing an MLNOfflinePack’s progress to continue to update after calling `-suspend`. ([#6186](https://github.com/mapbox/mapbox-gl-native/pull/6186))
* Fixed an issue preventing cached annotation images from displaying while the device is offline. ([#6358](https://github.com/mapbox/mapbox-gl-native/pull/6358))
* Fixed a crash that could occur when the device is disconnected while downloading an offline pack. ([#6293](https://github.com/mapbox/mapbox-gl-native/pull/6293))
* Fixed a crash that occurred when encountering a rate-limit error in response to a network request. ([#6223](https://github.com/mapbox/mapbox-gl-native/pull/6223))
* Added support for an `MLNMapboxAPIBaseURL` key in an app's `Info.plist` in order to customize the base URL used for retrieving map data, styles, and other resources. ([#6709](https://github.com/mapbox/mapbox-gl-native/pull/6709))
* Query parameters are no longer stripped from mapbox: URLs used as resource URLs. ([#6182](https://github.com/mapbox/mapbox-gl-native/pull/6182), [#6432](https://github.com/mapbox/mapbox-gl-native/pull/6432))
* Database errors are now logged to the console. ([#6291](https://github.com/mapbox/mapbox-gl-native/pull/6291))

### Other changes

* Raster tiles such as those from Mapbox Satellite are now cached, eliminating flashing while panning back and forth. ([#7091](https://github.com/mapbox/mapbox-gl-native/pull/7091))
* Fixed an issue where the map view’s center would always be calculated as if the view occupied the entire window. ([#6102](https://github.com/mapbox/mapbox-gl-native/pull/6102))
* Notification names and user info keys are now string enumeration values for ease of use in Swift. ([#6794](https://github.com/mapbox/mapbox-gl-native/pull/6794))
* Fixed a typo in the documentation for the MLNCompassDirectionFormatter class. ([#5879](https://github.com/mapbox/mapbox-gl-native/pull/5879))
* The NSClickGestureRecognizer on MLNMapView that is used for selecting annotations now fails if a click does not select an annotation. ([#7246](https://github.com/mapbox/mapbox-gl-native/pull/7246))

## 0.2.1 - July 19, 2016

* Fixed a crash that occurred when a sprite URL lacks a file extension. See [this comment](https://github.com/mapbox/mapbox-gl-native/issues/5722#issuecomment-233701251) to determine who may be affected by this bug. ([#5723](https://github.com/mapbox/mapbox-gl-native/pull/5723))
* Right-clicking to open MLNMapView’s context menu no longer prevents the user from subsequently panning the map by clicking and dragging. ([#5593](https://github.com/mapbox/mapbox-gl-native/pull/5593))
* Fixed an issue causing overlapping polylines and polygons to be drawn in undefined z-order. Shapes are always drawn in the order they are added to the map, from the oldest on the bottom to the newest on the top. ([#5710](https://github.com/mapbox/mapbox-gl-native/pull/5710))
* Improved the design of the generated API documentation. ([#5306](https://github.com/mapbox/mapbox-gl-native/pull/5306))
* As the user zooms in, tiles from lower zoom levels are scaled up until tiles for higher zoom levels are loaded. ([#5143](https://github.com/mapbox/mapbox-gl-native/pull/5143))
* Per documentation, the first and last coordinates in an MLNPolygon must be identical in order for the polygon to draw correctly. The same is true for an MLNPolygon’s interior polygon. ([#5514](https://github.com/mapbox/mapbox-gl-native/pull/5514))
* Added [quadkey](https://msdn.microsoft.com/en-us/library/bb259689.aspx) support and limited WMS support in raster tile URL templates. ([#5628](https://github.com/mapbox/mapbox-gl-native/pull/5628))
* Fixed a crash that occurred when a style or other resource URL has a query string. ([#5554](https://github.com/mapbox/mapbox-gl-native/pull/5554))
* Fixed an issue causing polyline and polygon annotations to disappear when the zoom level is one less than the maximum zoom level. ([#5418](https://github.com/mapbox/mapbox-gl-native/pull/5418))
* Added a property to MLNOfflineStorage, `countOfBytesCompleted`, that indicates the disk space occupied by all cached and offline resources. ([#5585](https://github.com/mapbox/mapbox-gl-native/pull/5585))
* The `text-pitch-alignment` property is now supported in stylesheets for improved street label legibility on a tilted map. ([#5288](https://github.com/mapbox/mapbox-gl-native/pull/5288))
* The `icon-text-fit` and `icon-text-fit-padding` properties are now supported in stylesheets, allowing the background of a shield to automatically resize to fit the shield’s text. ([#5334](https://github.com/mapbox/mapbox-gl-native/pull/5334))
* The `circle-pitch-scale` property is now supported in stylesheets, allowing circle features in a tilted base map to scale or remain the same size as the viewing distance changes. ([#5576](https://github.com/mapbox/mapbox-gl-native/pull/5576))
* The `identifier` property of an MLNFeature may now be either a number or string. ([#5514](https://github.com/mapbox/mapbox-gl-native/pull/5514))
* Improved the performance of relocating a point annotation by changing its `coordinate` property. ([#5385](https://github.com/mapbox/mapbox-gl-native/pull/5385))
* Replaced the wireframe debug mask with an overdraw visualization debug mask to match Mapbox GL JS’s overdraw inspector. ([#5403](https://github.com/mapbox/mapbox-gl-native/pull/5403))
* MLNMapDebugOverdrawVisualizationMask and MLNMapDebugStencilBufferMask no longer have any effect in Release builds of the SDK. These debug masks have been disabled for performance reasons. ([#5555](https://github.com/mapbox/mapbox-gl-native/pull/5555))

## 0.2.0 - June 14, 2016

* This version of the Mapbox macOS SDK roughly corresponds to version 3.3.0-beta.1 of the Mapbox iOS SDK. The two SDKs have very similar feature sets. The main differences are the lack of user location tracking and annotation views. Some APIs have been adapted to macOS conventions, particularly the use of NSPopover for callout views.
* Renamed the SDK to the Mapbox macOS SDK.
* Fixed an issue in which Mapbox.framework was nested inside another folder named Mapbox.framework. ([#4998](https://github.com/mapbox/mapbox-gl-native/pull/4998))
* Added methods to MLNMapView for obtaining the underlying map data rendered by the current style, along with additional classes to represent complex geometry in that data. ([#5110](https://github.com/mapbox/mapbox-gl-native/pull/5110))
* An MLNPolygon can now have interior polygons, representing holes knocked out of the overall shape. ([#5110](https://github.com/mapbox/mapbox-gl-native/pull/5110))
* Fixed a vector tile parsing bug that sometimes caused properties in the vector tile source to be mismatched. ([#5183](https://github.com/mapbox/mapbox-gl-native/pull/5183))
* Fixed a crash passing a mixture of point and shape annotations into `-[MLNMapView addAnnotations:]`. ([#5097](https://github.com/mapbox/mapbox-gl-native/pull/5097))
* Fixed an issue (speculatively) where the tile cache could be included in iCloud backups. ([#5124](https://github.com/mapbox/mapbox-gl-native/pull/5124))
* Improved performance viewing regions with large landcover polygons when viewing a style that uses the Mapbox Streets source. ([#2444](https://github.com/mapbox/mapbox-gl-native/pull/2444))
* Fixed a memory leak when using raster resources. ([#5141](https://github.com/mapbox/mapbox-gl-native/pull/5141))
* Added `MLNCoordinateInCoordinateBounds()`, a function that tests whether or not a coordinate is in a given bounds. ([#5053](https://github.com/mapbox/mapbox-gl-native/pull/5053))
* Fixed an issue in which fade transitions (such as on street labels in some styles) lagged behind the map when quickly zooming in and out. ([#4579](https://github.com/mapbox/mapbox-gl-native/pull/4579))
* Added new options to `MLNMapDebugMaskOptions` that show wireframes and the stencil buffer instead of the color buffer. ([#4359](https://github.com/mapbox/mapbox-gl-native/pull/4359))
* Declarations in the API documentation are shown in both Objective-C and Swift. ([realm/jazzy#530](https://github.com/realm/jazzy/pull/530))

## 0.1.0 - May 10, 2016

* This version of the Mapbox OS X SDK roughly corresponds to version 3.3.0-alpha.2 of the Mapbox iOS SDK. The two SDKs have very similar feature sets. The main difference is the lack of user location tracking. Some APIs have been adapted to OS X conventions, particularly the use of NSPopover for callout views.
