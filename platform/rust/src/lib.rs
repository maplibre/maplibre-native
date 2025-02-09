// FIXME: Remove this before merging
#![allow(unused)]

mod map_renderer;
mod tile_server_options;

pub use ffi::MapDebugOptions;
pub use map_renderer::{Image, ImageRenderer, ImageRendererOptions, Static, Tile};
pub use tile_server_options::TileServerOptions;

#[cxx::bridge(namespace = "mln::bridge")]
mod ffi {
    //
    // CXX validates enum types against the C++ definition during compilation
    //

    #[repr(u32)]
    #[derive(Debug, Clone, Copy, PartialEq, Eq)]
    enum MapMode {
        Continuous,
        Static,
        Tile,
    }

    #[repr(u32)]
    #[derive(Debug, Clone, Copy, PartialEq, Eq)]
    enum MapDebugOptions {
        NoDebug = 0,
        TileBorders = 0b0000_0010, // 1 << 1
        ParseStatus = 0b0000_0100, // 1 << 2
        Timestamps = 0b0000_1000,  // 1 << 3
        Collision = 0b0001_0000,   // 1 << 4
        Overdraw = 0b0010_0000,    // 1 << 5
        StencilClip = 0b0100_0000, // 1 << 6
        DepthBuffer = 0b1000_0000, // 1 << 7
    }

    #[namespace = "mbgl"]
    unsafe extern "C++" {
        include!("mbgl/map/mode.hpp");

        type MapMode;
        type MapDebugOptions;
    }

    unsafe extern "C++" {
        include!("maplibre-native/include/tile_server_options.h");

        type TileServerOptions;

        fn TileServerOptions_new() -> UniquePtr<TileServerOptions>;
        fn TileServerOptions_mapbox() -> UniquePtr<TileServerOptions>;
        fn TileServerOptions_maplibre() -> UniquePtr<TileServerOptions>;
        fn TileServerOptions_maptiler() -> UniquePtr<TileServerOptions>;

        fn TileServerOptions_withBaseURL(obj: Pin<&mut TileServerOptions>, value: &str);
        fn baseURL(&self) -> &CxxString;
        fn TileServerOptions_withUriSchemeAlias(obj: Pin<&mut TileServerOptions>, value: &str);
        fn uriSchemeAlias(&self) -> &CxxString;
        unsafe fn TileServerOptions_withSourceTemplate(
            obj: Pin<&mut TileServerOptions>,
            source_template: &str,
            domain_name: &str,
            version_prefix: *const i8,
        );
        fn sourceTemplate(&self) -> &CxxString;
        fn sourceDomainName(&self) -> &CxxString;
        unsafe fn TileServerOptions_sourceVersionPrefix(value: &TileServerOptions) -> *const i8;
        unsafe fn TileServerOptions_withStyleTemplate(
            obj: Pin<&mut TileServerOptions>,
            style_template: &str,
            domain_name: &str,
            version_prefix: *const i8,
        );
        fn styleTemplate(&self) -> &CxxString;
        fn styleDomainName(&self) -> &CxxString;
        unsafe fn TileServerOptions_styleVersionPrefix(value: &TileServerOptions) -> *const i8;
        unsafe fn TileServerOptions_withSpritesTemplate(
            obj: Pin<&mut TileServerOptions>,
            sprites_template: &str,
            domain_name: &str,
            version_prefix: *const i8,
        );
        fn spritesTemplate(&self) -> &CxxString;
        fn spritesDomainName(&self) -> &CxxString;
        unsafe fn TileServerOptions_spritesVersionPrefix(value: &TileServerOptions) -> *const i8;
        unsafe fn TileServerOptions_withGlyphsTemplate(
            obj: Pin<&mut TileServerOptions>,
            glyphs_template: &str,
            domain_name: &str,
            version_prefix: *const i8,
        );
        fn glyphsTemplate(&self) -> &CxxString;
        fn glyphsDomainName(&self) -> &CxxString;
        unsafe fn TileServerOptions_glyphsVersionPrefix(value: &TileServerOptions) -> *const i8;
        unsafe fn TileServerOptions_withTileTemplate(
            obj: Pin<&mut TileServerOptions>,
            tile_template: &str,
            domain_name: &str,
            version_prefix: *const i8,
        );
        fn tileTemplate(&self) -> &CxxString;
        fn tileDomainName(&self) -> &CxxString;
        unsafe fn TileServerOptions_tileVersionPrefix(value: &TileServerOptions) -> *const i8;
        unsafe fn TileServerOptions_withApiKeyParameterName(
            obj: Pin<&mut TileServerOptions>,
            value: &str,
        );
        fn apiKeyParameterName(&self) -> &CxxString;
        fn TileServerOptions_setRequiresApiKey(obj: Pin<&mut TileServerOptions>, value: bool);
        fn requiresApiKey(&self) -> bool;

        // /// Gets the default styles.
        // const std::vector<mbgl::util::DefaultStyle> defaultStyles() const;

        // /// Sets the collection default styles.
        // TileServerOptions& withDefaultStyles(std::vector<mbgl::util::DefaultStyle> styles);

        // fn withDefaultStyle(&self);
        unsafe fn TileServerOptions_withDefaultStyle(obj: Pin<&mut TileServerOptions>, value: &str);
        fn defaultStyle(&self) -> &CxxString;
    }

    unsafe extern "C++" {
        include!("maplibre-native/include/map_renderer.h");

        type MapRenderer;

        // MapRenderer_new(mbgl::MapMode mapMode, uint32_t width, uint32_t height, float pixelRatio, const rust::Str cachePath)
        fn MapRenderer_new(
            mapMode: MapMode,
            width: u32,
            height: u32,
            pixelRatio: f32,
            cachePath: &str,
            assetRoot: &str,
            apiKey: &str,
        ) -> UniquePtr<MapRenderer>;
        fn MapRenderer_render(obj: Pin<&mut MapRenderer>) -> UniquePtr<CxxString>;
        fn MapRenderer_setDebugFlags(obj: Pin<&mut MapRenderer>, flags: MapDebugOptions);
        fn MapRenderer_setCamera(
            obj: Pin<&mut MapRenderer>,
            lat: f64,
            lon: f64,
            zoom: f64,
            bearing: f64,
            pitch: f64,
        );
        fn MapRenderer_setStyleUrl(obj: Pin<&mut MapRenderer>, url: &str);
    }
}
