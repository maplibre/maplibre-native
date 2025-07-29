package org.maplibre.android.plugin

class PluginProtocolHandlerResource {

    enum class PluginProtocolHandlerResourceKind {
        Unknown,
        Style,
        Source,
        Tile,
        Glyphs,
        SpriteImage,
        SpriteJSON,
        Image
    };

    enum class PluginProtocolHandlerResourceLoadingMethod {
        Unknown,
        CacheOnly,
        NetworkOnly,
        All
    };

    var resourceURL: String = "Test";

    var kind: Int = 0;

    var resourceKind: PluginProtocolHandlerResourceKind = PluginProtocolHandlerResourceKind.Unknown;

    var loadingMethod: PluginProtocolHandlerResourceLoadingMethod = PluginProtocolHandlerResourceLoadingMethod.Unknown;

    var tileData: TileData? = null;

}
