mod map_renderer;
mod tile_server_options;

pub use map_renderer::{ImageRenderer, ImageRendererOptions, Static, Tile, Image};
pub use tile_server_options::TileServerOptions;

#[cxx::bridge(namespace = "ml::bridge")]
mod ffi {
    unsafe extern "C++" {
        include!("maplibre-native/include/tile_server_options.h");

        #[namespace = "mbgl"]
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

        #[namespace = "ml::bridge"]
        type MapRenderer;

        fn MapRenderer_new() -> UniquePtr<MapRenderer>;

        //     void set_size(uint32_t width, uint32_t height) { size = {width, height}; }
        //
        //     void set_pixel_ratio(float ratio) { pixelRatio = ratio; }
        //
        //     void set_map_mode(uint32_t mode) { mapMode = static_cast<mbgl::MapMode>(mode); }
        //
        //     void set_debug_flags(uint32_t flags) { debugFlags = static_cast<mbgl::MapDebugOptions>(flags); }
        //
        //     void set_camera(double lat, double lon, double zoom, double bearing, double pitch) {
        //         cameraOptions.withCenter(mbgl::LatLng{lat, lon}).withZoom(zoom).withBearing(bearing).withPitch(pitch);
        //     }
        //
        //     void set_api_key(const std::string& key) { apiKey = key; }
        //
        //     void set_cache_path(const std::string& path) { cachePath = path; }
        //
        //     void set_asset_root(const std::string& path) { assetRoot = path; }
        //
        //     void set_style_url(const std::string& url) { styleUrl = url; }
        //
        //     // Main rendering method
        //     std::vector<uint8_t> render();

        // fn set_size(&self, width: u32, height: u32);
        // fn set_pixel_ratio(&self, ratio: f64);
        // fn set_map_mode(&self, mode: u32);
        // fn set_debug_flags(&self, flags: u32);
        // fn set_camera(&self, lat: f64, lon: f64, zoom: f64, bearing: f64, pitch: f64);
        // fn set_api_key(&self, key: &str);

        fn MapRenderer_setSize(obj: Pin<&mut MapRenderer>, width: u32, height: u32);
        fn MapRenderer_render(obj: Pin<&mut MapRenderer>) -> UniquePtr<CxxVector<u8>>;
    }
}
