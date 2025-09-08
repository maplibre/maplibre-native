# Customizing Fonts

Using custom fonts

MapLibre Native iOS can render text that is part of an ``MLNSymbolStyleLayer`` in a font of your choice. The font customization options discussed in this document do not apply to user interface elements such as the scale bar or annotation callout views.

## Server-side fonts

By default, the map renders characters using glyphs downloaded from the server. You apply fonts when building a style with [Maputnik](https://maputnik.github.io), in the `text-font` layout property in style JSON, or in the ``MLNSymbolStyleLayer/textFontNames`` property at runtime. The values in these properties must be font display names, not font family names or PostScript names.

Each font name in the list must match a font that is present on the server; otherwise, the text will not load, even if one of the fonts is available. Each font name must be included in the `{fontstack}` portion of the JSON stylesheet’s [`glyphs`](https://maplibre.org/maplibre-style-spec/glyphs/) property.

[Martin](https://maplibre.org/martin/sources-fonts.html) can serve fonts directly. You can also generate fonts with [font-maker](https://github.com/maplibre/font-maker).

## Client-side fonts

By default, Chinese hanzi, Japanese kana, and Korean hangul characters (CJK) are rendered on the client side. Client-side text rendering uses less bandwidth than server-side text rendering, especially when viewing regions of the map that feature a wide variety of CJK characters.

First, the map attempts to apply a font that you specify the same way as you would specify a server-side font: in [Maputnik](https://maputnik.github.io), in the `text-font` layout property in style JSON, or in the `MLNSymbolStyleLayer.textFontNames` property at runtime. Instead of downloading the glyphs, the map tries to find a [system font](https://developer.apple.com/fonts/system-fonts/) or a font [bundled with your application](https://developer.apple.com/documentation/uikit/text_display_and_fonts/adding_a_custom_font_to_your_app) that matches one of these fonts based on its family name (for example, “PingFang TC”), display name (“PingFang TC Ultralight”), or PostScript name (“PingFangTC-Ultralight”).

If the symbol layer does not specify an available font that contains the required glyphs, then the map tries to find a matching font in the `MLNIdeographicFontFamilyName` [Info.plist key](doc:Info.plist_Keys). Like the ``MLNSymbolStyleLayer/textFontNames`` property, this key can contain a family name, display name, or PostScript name. This key is a global fallback that applies to all layers uniformly. It can either be a single string or an array of strings, which the map tries to apply in order from most preferred to least preferred.

Each character is rendered in the first font you specify that has a glyph for the character. If the entire list of fonts is exhausted, the map uses the system’s font cascade list, which may vary based on the device model and system language.

To disable client-side rendering of CJK characters, set the `MLNIdeographicFontFamilyName` key to the Boolean value `NO`. The map will revert to server-side font rendering.
