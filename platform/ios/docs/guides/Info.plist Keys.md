# Info.plist Keys

The Mapbox Maps SDK for iOS supports custom `Info.plist` keys in your application in order to configure various settings.

## MGLMapboxAccessToken

Set the [Mapbox access token](https://www.mapbox.com/help/define-access-token/) to be used by all instances of `MGLMapView` in the current application.

Mapbox-hosted vector tiles and styles require an API access token, which you can obtain from the [Mapbox account page](https://www.mapbox.com/studio/account/tokens/). Access tokens associate requests to Mapbox’s vector tile and style APIs with your Mapbox account. They also deter other developers from using your styles without your permission.

As an alternative, you can use `MGLAccountManager.accessToken` to set a token in code. See [our guide](https://www.mapbox.com/help/ios-private-access-token/) for some tips on keeping access tokens in open source code private.

## MGLMapboxAPIBaseURL

Use this key if you need to customize the API base URL used throughout the SDK. If unset, the default Mapbox API is used.

The default value is `https://api.mapbox.com`.

## MGLMapboxMetricsEnabledSettingShownInApp

If you have implemented custom opt-out of Mapbox Telemetry within the user interface of your app, use this key to disable the built-in check for opt-out support. See [this guide](https://docs.mapbox.com/help/how-mapbox-works/attribution/#mapbox-maps-sdk-for-ios) for more details.

## MGLIdeographicFontFamilyName

This key configures client-side text rendering of Chinese hanzi, Japanese kana, and Korean hangul characters (CJK) that appear in text labels. Client-side text rendering uses less bandwidth than server-side text rendering, especially when viewing regions of the map that feature a wide variety of CJK characters.

By default, the map renders CJK characters using locally installed fonts as specified by the system’s font cascade list, which may vary from device to device, and ignores the `MGLSymbolStyleLayer.textFont` property for these characters. To customize the displayed font, set this key to a string containing a font family name (for example, “PingFang TC”) or an individual font’s display name (“PingFang TC Ultralight”) or PostScript name (“PingFangTC-Ultralight”). The key can name a font that is installed system-wide or bundled with the application.

In case your preferred font has glyphs for every character that may appear on the map, you can set this key to an array of font or font family names in order from most preferred to least preferred. Each character is rendered in the first font in the list that has a glyph for the character. If the entire font is exhausted, the map uses the system’s font cascade list.

To disable client-side rendering of CJK characters, set this key to the Boolean value `NO`. Glyphs from the fonts specified by `MGLSymbolStyleLayer.textFont` will be downloaded and rendered instead.

## MGLCollisionBehaviorPre4_0

 If this key is set to YES (`true`), collision detection is performed only between symbol style layers based on the same source, as in versions 2.0–3.7 of the Mapbox Maps SDK for iOS. In other words, symbols in an `MGLSymbolStyleLayer` based on one source (for example, an `MGLShapeSource`) may overlap with symbols in another layer that is based on a different source (such as the Mapbox Streets source). This is the case regardless of the `MGLSymbolStyleLayer.iconAllowsOverlap`, `MGLSymbolStyleLayer.iconIgnoresPlacement`, `MGLSymbolStyleLayer.textAllowsOverlap`, and `MGLSymbolStyleLayer.textIgnoresPlacement` properties.

Beginning in version 4.0, the SDK also performs collision detection between style layers based on different sources by default. For the default behavior, omit the `MGLCollisionBehaviorPre4_0` key or set it to NO (`false`).

This property may also be set using `[[NSUserDefaults standardUserDefaults] setObject:@(YES) forKey:@"MGLCollisionBehaviorPre4_0"]`; it will override any value specified in the `Info.plist`.

