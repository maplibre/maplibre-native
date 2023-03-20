# Info.plist Keys

The Mapbox Maps SDK for macOS supports custom `Info.plist` keys in your application in order to configure various settings.

## MLNApiKey

Some vector tiles servers require a API key. Set the API key to be used by all instances of `MLNMapView` in the current application.

As an alternative, you can use `MLNSettings.accessToken` to set a token in code. See [our guide](https://www.mapbox.com/help/ios-private-access-token/) for some tips on keeping access tokens in open source code private.

## MLNTileServerBaseURL

Use this key if you need to customize the API base URL used throughout the SDK.

## MLNIdeographicFontFamilyName

This key configures a global fallback font or fonts for [client-side text rendering](customizing-fonts.html#client-side-fonts) of Chinese hanzi, Japanese kana, and Korean hangul characters (CJK) that appear in text labels.

If the fonts you specify in the `MLNSymbolStyleLayer.textFontNames` property are all unavailable or lack a glyph for rendering a given CJK character, the map uses the contents of this key to choose a [system font](https://developer.apple.com/fonts/system-fonts/) or a font [bundled with your application](https://developer.apple.com/documentation/bundleresources/information_property_list/atsapplicationfontspath). This key specifies a fallback for all style layers in all map views and map snapshots. If you do not specify this key or none of the font names matches, the map applies a font from the system’s font cascade list, which may vary based on the system language and other installed applications.

This key can either be set to a single string or an array of strings, which the map tries to apply in order from most preferred to least preferred. Each string can be a family name (for example, “PingFang TC”), display name (“PingFang TC Ultralight”), or PostScript name (“PingFangTC-Ultralight”).

To disable client-side rendering of CJK characters in favor of [server-side rendering](customizing-fonts.html#server-side-fonts), set this key to the Boolean value `NO`.

## MLNOfflineStorageDatabasePath

This key customizes the file path at which `MLNOfflineStorage` keeps the offline map database, which contains any offline packs as well as the ambient cache. Most applications should not need to customize this path; however, you could customize it to implement a migration path between different versions of your application.

The key is interpreted as either an absolute file path or a file path relative to the main bundle’s resource folder, resolving any tilde or symbolic link. The path must be writable. If a database does not exist at the path you specify, one will be created automatically.

An offline map database can consume a significant amount of the user’s bandwidth and iCloud storage due to iCloud backups. To exclude the database from backups, set the containing directory’s `NSURLIsExcludedFromBackupKey` resource property to the Boolean value `YES` using the `-[NSURL setResourceValue:forKey:error:]` method. The entire directory will be affected, not just the database file. If the user restores the application from a backup, your application will need to restore any offline packs that had been previously downloaded.

At runtime, you can obtain the value of this key using the `MLNOfflineStorage.databasePath` and `MLNOfflineStorage.databaseURL` properties.

## MLNCollisionBehaviorPre4_0

If this key is set to YES (`true`), collision detection is performed only between symbol style layers based on the same source, as in versions 0.1–0.7 of the Mapbox Maps SDK for iOS. In other words, symbols in an `MLNSymbolStyleLayer` based on one source (for example, an `MLNShapeSource`) may overlap with symbols in another layer that is based on a different source (such as the Mapbox Streets source). This is the case regardless of the `MLNSymbolStyleLayer.iconAllowsOverlap`, `MLNSymbolStyleLayer.iconIgnoresPlacement`, `MLNSymbolStyleLayer.textAllowsOverlap`, and `MLNSymbolStyleLayer.textIgnoresPlacement` properties.

Beginning in version 0.7, the SDK also performs collision detection between style layers based on different sources by default. For the default behavior, omit the `MLNCollisionBehaviorPre4_0` key or set it to NO (`false`). This property is so named because version 0.7 of the Mapbox Maps SDK for macOS corresponds to version 4.0 of the Mapbox Maps SDK for iOS.
