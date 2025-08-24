package org.maplibre.android.plugin

class PluginProtocolHandlerResource {

    enum class Kind {
        Unknown,
        Style,
        Source,
        Tile,
        Glyphs,
        SpriteImage,
        SpriteJSON,
        Image
    };

    enum class LoadingMethod {
        Unknown,
        CacheOnly,
        NetworkOnly,
        All
    };

    var resourceURL: String = "";

    var kind: Int = 0;

    var resourceKind: Kind = Kind.Unknown;

    var loadingMethod: LoadingMethod = LoadingMethod.Unknown;

    var tileData: TileData? = null;

}
