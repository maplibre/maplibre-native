mod tile_server_options;
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
}
