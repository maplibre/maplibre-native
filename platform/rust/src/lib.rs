mod tile_server_options;
pub use tile_server_options::TileServerOptions;

#[cxx::bridge(namespace = "ml::bridge")]
mod ffi {
    // C++ types exposed to Rust.
    unsafe extern "C++" {
        include!("maplibre-native/include/wrapper.h");

        #[namespace = "mbgl"]
        type TileServerOptions;

        fn TileServerOptions_new() -> UniquePtr<TileServerOptions>;
        fn TileServerOptions_mapbox() -> UniquePtr<TileServerOptions>;
        fn TileServerOptions_maplibre() -> UniquePtr<TileServerOptions>;
        fn TileServerOptions_maptiler() -> UniquePtr<TileServerOptions>;

        fn TileServerOptions_withBaseURL(obj: Pin<&mut TileServerOptions>, value: &str);
        fn baseURL(self: &TileServerOptions) -> &CxxString;
        fn TileServerOptions_withUriSchemeAlias(obj: Pin<&mut TileServerOptions>, value: &str);
        fn uriSchemeAlias(self: &TileServerOptions) -> &CxxString;
        unsafe fn TileServerOptions_withSourceTemplate(
            obj: Pin<&mut TileServerOptions>,
            source_template: &str,
            domain_name: &str,
            version_prefix: *const i8,
        );
        fn sourceTemplate(self: &TileServerOptions) -> &CxxString;
        fn sourceDomainName(self: &TileServerOptions) -> &CxxString;
        unsafe fn TileServerOptions_sourceVersionPrefix(value: &TileServerOptions) -> *const i8;
        unsafe fn TileServerOptions_withStyleTemplate(
            obj: Pin<&mut TileServerOptions>,
            style_template: &str,
            domain_name: &str,
            version_prefix: *const i8,
        );
        fn styleTemplate(self: &TileServerOptions) -> &CxxString;
        fn styleDomainName(self: &TileServerOptions) -> &CxxString;
        unsafe fn TileServerOptions_styleVersionPrefix(value: &TileServerOptions) -> *const i8;
        unsafe fn TileServerOptions_withSpritesTemplate(
            obj: Pin<&mut TileServerOptions>,
            sprites_template: &str,
            domain_name: &str,
            version_prefix: *const i8,
        );
        fn spritesTemplate(self: &TileServerOptions) -> &CxxString;
        fn spritesDomainName(self: &TileServerOptions) -> &CxxString;
        unsafe fn TileServerOptions_spritesVersionPrefix(value: &TileServerOptions) -> *const i8;
        unsafe fn TileServerOptions_withGlyphsTemplate(
            obj: Pin<&mut TileServerOptions>,
            glyphs_template: &str,
            domain_name: &str,
            version_prefix: *const i8,
        );
        fn glyphsTemplate(self: &TileServerOptions) -> &CxxString;
        fn glyphsDomainName(self: &TileServerOptions) -> &CxxString;
        unsafe fn TileServerOptions_glyphsVersionPrefix(value: &TileServerOptions) -> *const i8;
        unsafe fn TileServerOptions_withTileTemplate(
            obj: Pin<&mut TileServerOptions>,
            tile_template: &str,
            domain_name: &str,
            version_prefix: *const i8,
        );
        fn tileTemplate(self: &TileServerOptions) -> &CxxString;
        fn tileDomainName(self: &TileServerOptions) -> &CxxString;
        unsafe fn TileServerOptions_tileVersionPrefix(value: &TileServerOptions) -> *const i8;
        unsafe fn TileServerOptions_withApiKeyParameterName(
            obj: Pin<&mut TileServerOptions>,
            value: &str,
        );
        fn apiKeyParameterName(self: &TileServerOptions) -> &CxxString;
        fn TileServerOptions_setRequiresApiKey(obj: Pin<&mut TileServerOptions>, value: bool);
        fn requiresApiKey(self: &TileServerOptions) -> bool;

        // /// Gets the default styles.
        // const std::vector<mbgl::util::DefaultStyle> defaultStyles() const;

        // /// Sets the collection default styles.
        // TileServerOptions& withDefaultStyles(std::vector<mbgl::util::DefaultStyle> styles);

        // fn withDefaultStyle(self: &TileServerOptions);
        unsafe fn TileServerOptions_withDefaultStyle(obj: Pin<&mut TileServerOptions>, value: &str);
        fn defaultStyle(self: &TileServerOptions) -> &CxxString;
    }
}
