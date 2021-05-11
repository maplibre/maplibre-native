# Info.plist Keys

The Mapbox Maps SDK for iOS supports custom `Info.plist` keys in your application in order to configure various settings.

## MGLApiKey

If it is required by the tileserver you use, set the API key to be used by all instances of `MGLMapView` in the current application.

As an alternative, you can use `MGLSettings.accessToken` to set a token in code.
## MGLAccuracyAuthorizationDescription

Set the Mapbox accuracy authorization description string as an element of `NSLocationTemporaryUsageDescriptionDictionary` to be used by the map to request authorization when the `MGLLocationManager.accuracyAuthorization` is set to `CLAccuracyAuthorizationReducedAccuracy`. Requesting accuracy authorization is available for devices running iOS 14.0 and above.

Example:

```xml
<key>NSLocationTemporaryUsageDescriptionDictionary</key>
 <dict>
  <key>MGLAccuracyAuthorizationDescription</key>
  <string>Mapbox requires your precise location to help you navigate the map.</string>
 </dict>
```

Remove `MGLAccuracyAuthorizationDescription` if you want to control when to request for accuracy authorization.

## MGLMapboxAPIBaseURL

Use this key if you need to customize the API base URL used throughout the SDK. If unset, the default Mapbox API is used.

The default value is `https://api.mapbox.com`.

## MGLMapboxMetricsEnabledSettingShownInApp

If you have implemented custom opt-out of Mapbox Telemetry within the user interface of your app, use this key to disable the built-in check for opt-out support. See [this guide](https://docs.mapbox.com/help/how-mapbox-works/attribution/#mapbox-maps-sdk-for-ios) for more details.

## MGLIdeographicFontFamilyName

This key configures a global fallback font or fonts for [client-side text rendering](customizing-fonts.html#client-side-fonts) of Chinese hanzi, Japanese kana, and Korean hangul characters (CJK) that appear in text labels.

If the fonts you specify in the `MGLSymbolStyleLayer.textFontNames` property are all unavailable or lack a glyph for rendering a given CJK character, the map uses the contents of this key to choose a [system font](https://developer.apple.com/fonts/system-fonts/) or a font [bundled with your application](https://developer.apple.com/documentation/uikit/text_display_and_fonts/adding_a_custom_font_to_your_app). This key specifies a fallback for all style layers in all map views and map snapshots. If you do not specify this key or none of the font names matches, the map applies a font from the system’s font cascade list, which may vary based on the device model and system language.

This key can either be set to a single string or an array of strings, which the map tries to apply in order from most preferred to least preferred. Each string can be a family name (for example, “PingFang TC”), display name (“PingFang TC Ultralight”), or PostScript name (“PingFangTC-Ultralight”).

To disable client-side rendering of CJK characters in favor of [server-side rendering](customizing-fonts.html#server-side-fonts), set this key to the Boolean value `NO`.

## MGLOfflineStorageDatabasePath

This key customizes the file path at which `MGLOfflineStorage` keeps the offline map database, which contains any offline packs as well as the ambient cache. Most applications should not need to customize this path; however, you could customize it to implement a migration path between different versions of your application.

The key is interpreted as either an absolute file path or a file path relative to the main bundle’s resource folder, resolving any tilde or symbolic link. The path must be writable. If a database does not exist at the path you specify, one will be created automatically.

An offline map database can consume a significant amount of the user’s bandwidth and iCloud storage due to iCloud backups. To exclude the database from backups, set the containing directory’s `NSURLIsExcludedFromBackupKey` resource property to the Boolean value `YES` using the `-[NSURL setResourceValue:forKey:error:]` method. The entire directory will be affected, not just the database file. If the user restores the application from a backup, your application will need to restore any offline packs that had been previously downloaded.

At runtime, you can obtain the value of this key using the `MGLOfflineStorage.databasePath` and `MGLOfflineStorage.databaseURL` properties.

## MGLCollisionBehaviorPre4_0

 If this key is set to YES (`true`), collision detection is performed only between symbol style layers based on the same source, as in versions 2.0–3.7 of the Mapbox Maps SDK for iOS. In other words, symbols in an `MGLSymbolStyleLayer` based on one source (for example, an `MGLShapeSource`) may overlap with symbols in another layer that is based on a different source (such as the Mapbox Streets source). This is the case regardless of the `MGLSymbolStyleLayer.iconAllowsOverlap`, `MGLSymbolStyleLayer.iconIgnoresPlacement`, `MGLSymbolStyleLayer.textAllowsOverlap`, and `MGLSymbolStyleLayer.textIgnoresPlacement` properties.

Beginning in version 4.0, the SDK also performs collision detection between style layers based on different sources by default. For the default behavior, omit the `MGLCollisionBehaviorPre4_0` key or set it to NO (`false`).

This property may also be set using `[[NSUserDefaults standardUserDefaults] setObject:@(YES) forKey:@"MGLCollisionBehaviorPre4_0"]`; it will override any value specified in the `Info.plist`.
