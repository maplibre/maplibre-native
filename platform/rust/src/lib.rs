use std::ffi::{c_char, CStr};
use std::fmt;
use std::fmt::Debug;

use cxx::UniquePtr;

#[cxx::bridge(namespace = "ml::bridge")]
mod ffi {
    // C++ types exposed to Rust.
    unsafe extern "C++" {
        include!("maplibre-native/include/wrapper.h");

        // We specify the C++ namespace and the free function name exactly.
        // cxx can bind free functions directly if they have a compatible signature.
        // The signature must match what's in log2.hpp:
        //   "uint32_t ceil_log2(uint64_t x);"
        //
        // We'll express that to Rust as (u64 -> u32).
        #[namespace = "mbgl::util"]
        pub fn ceil_log2(x: u64) -> u32;

        // A function defined in the C++ Rust wrapper rather than the core lib.
        pub fn get_42() -> u32;

        #[namespace = "mbgl"]
        type TileServerOptions;

        /// Create a new default configuration
        fn TileServerOptions_new() -> UniquePtr<TileServerOptions>;
        /// Get the tile server options configured for MapLibre.
        fn TileServerOptions_mapbox() -> UniquePtr<TileServerOptions>;
        /// Get the tile server options configured for Mapbox.
        fn TileServerOptions_maplibre() -> UniquePtr<TileServerOptions>;
        /// Get the tile server options configured for MapTiler.
        fn TileServerOptions_maptiler() -> UniquePtr<TileServerOptions>;

        // /// Sets the API base URL.
        // #[cxx_name = "withBaseURL"]
        // TileServerOptions& withBaseURL(std::string baseURL);

        /// Gets the base URL.
        #[cxx_name = "baseURL"]
        fn base_url(self: &TileServerOptions) -> &CxxString;

        // /// Sets the scheme alias for the tile server. For example maptiler:// for MapTiler.
        // #[cxx_name = "withUriSchemeAlias"]
        // TileServerOptions& withUriSchemeAlias(std::string alias);

        /// Gets the URI scheme alias.
        #[cxx_name = "uriSchemeAlias"]
        fn uri_scheme_alias(self: &TileServerOptions) -> &CxxString;

        // /// Sets the template for sources.
        // #[cxx_name = "withSourceTemplate"]
        // TileServerOptions& withSourceTemplate(std::string sourceTemplate, std::string domainName, std::optional<std::string> versionPrefix);

        /// Gets the source template.
        #[cxx_name = "sourceTemplate"]
        fn source_template(self: &TileServerOptions) -> &CxxString;

        /// Gets the source domain name.
        #[cxx_name = "sourceDomainName"]
        fn source_domain_name(self: &TileServerOptions) -> &CxxString;

        /// Gets the source version prefix.
        unsafe fn TileServerOptions_sourceVersionPrefix(value: &TileServerOptions) -> *const i8;

        // /// Sets the template for styles. If `domainName` is set, the URL domain must contain the specified
        // /// string to be matched as canonical style URL.
        // #[cxx_name = "withStyleTemplate"]
        // TileServerOptions& withStyleTemplate(std::string styleTemplate, std::string domainName, std::optional<std::string> versionPrefix);

        /// Gets the style template.
        #[cxx_name = "styleTemplate"]
        fn style_template(self: &TileServerOptions) -> &CxxString;

        /// Gets the style domain name.
        #[cxx_name = "styleDomainName"]
        fn style_domain_name(self: &TileServerOptions) -> &CxxString;

        /// Gets the style version prefix.
        unsafe fn TileServerOptions_styleVersionPrefix(value: &TileServerOptions) -> *const i8;
        // fn style_version_prefix(self: &TileServerOptions) -> &CxxString;

        // /// Sets the template for sprites.
        // /// If `domainName` is set, the URL domain must contain the specified
        // /// string to be matched as canonical sprite URL.
        // #[cxx_name = "withSpritesTemplate"]
        // TileServerOptions& withSpritesTemplate(std::string spritesTemplate, std::string domainName, std::optional<std::string> versionPrefix);

        /// Gets the sprites template.
        #[cxx_name = "spritesTemplate"]
        fn sprites_template(self: &TileServerOptions) -> &CxxString;

        /// Gets the sprites domain name.
        #[cxx_name = "spritesDomainName"]
        fn sprites_domain_name(self: &TileServerOptions) -> &CxxString;

        /// Gets the sprites version prefix.
        unsafe fn TileServerOptions_spritesVersionPrefix(value: &TileServerOptions) -> *const i8;
        // fn sprites_version_prefix(self: &TileServerOptions) -> *const CxxString;

        // /// Sets the template for glyphs.
        // /// If  set, the URL domain must contain the specified
        // /// string to be matched as canonical glyphs URL.
        // #[cxx_name = "withGlyphsTemplate"]
        // TileServerOptions& withGlyphsTemplate(std::string glyphsTemplate, std::string domainName, std::optional<std::string> versionPrefix);

        /// Gets the glyphs template.
        #[cxx_name = "glyphsTemplate"]
        fn glyphs_template(self: &TileServerOptions) -> &CxxString;

        /// Gets the glyphs domain name.
        #[cxx_name = "glyphsDomainName"]
        fn glyphs_domain_name(self: &TileServerOptions) -> &CxxString;

        /// Gets the glyphs version prefix.
        unsafe fn TileServerOptions_glyphsVersionPrefix(value: &TileServerOptions) -> *const i8;
        // fn glyphs_version_prefix(self: &TileServerOptions) -> &CxxString;

        // /// Sets the template for tiles.
        // ///
        // /// If `domainName` is set, the URL domain must contain the specified
        // /// string to be matched as canonical tile URL.
        // #[cxx_name = "withTileTemplate"]
        // TileServerOptions& withTileTemplate(std::string tileTemplate, std::string domainName, std::optional<std::string> versionPrefix);

        /// Gets the tile template.
        #[cxx_name = "tileTemplate"]
        fn tile_template(self: &TileServerOptions) -> &CxxString;

        /// Gets the tile domain name.
        #[cxx_name = "tileDomainName"]
        fn tile_domain_name(self: &TileServerOptions) -> &CxxString;

        /// Gets the tile version prefix.
        unsafe fn TileServerOptions_tileVersionPrefix(value: &TileServerOptions) -> *const i8;
        // fn tile_version_prefix(self: &TileServerOptions) -> &CxxString;

        // /// Sets the access token parameter name.
        // #[cxx_name = "withApiKeyParameterName"]
        // TileServerOptions& withApiKeyParameterName(std::string apiKeyParameterName);

        /// Gets the API key parameter name.
        #[cxx_name = "apiKeyParameterName"]
        fn api_key_parameter_name(self: &TileServerOptions) -> &CxxString;

        // #[cxx_name = "setRequiresApiKey"]
        // TileServerOptions& setRequiresApiKey(bool apiKeyRequired);

        /// Checks if an API key is required.
        #[cxx_name = "requiresApiKey"]
        fn requires_api_key(self: &TileServerOptions) -> bool;

        // /// Gets the default styles.
        // #[cxx_name = "defaultStyles"]
        // const std::vector<mbgl::util::DefaultStyle> defaultStyles() const;

        // /// Sets the collection default styles.
        // #[cxx_name = "withDefaultStyles"]
        // TileServerOptions& withDefaultStyles(std::vector<mbgl::util::DefaultStyle> styles);

        // #[cxx_name = "withDefaultStyle"]
        // fn set_default_style(self: &TileServerOptions);

        /// Gets the default style.
        #[cxx_name = "defaultStyle"]
        fn default_style(self: &TileServerOptions) -> &CxxString;
    }
}

// Re-export native functions that do not need safety wrappers.
pub use ffi::{ceil_log2, get_42, TileServerOptions};

// Add more methods to make usage more ergonomic
impl TileServerOptions {
    /// Convert a pointer to a C string to an optional Rust &CStr with lifetime of &self.
    ///
    /// # Safety
    /// The &CStr will remain valid as long as the TileServerOptions `&self` remains valid.
    /// If some code tries to modify self, it will need to use &mut self,
    /// which will not compile unless there are no &CStr references, as they use the same lifetime.
    unsafe fn to_opt(&self, value: *const c_char) -> Option<&CStr> {
        value.as_ref().map(|v| CStr::from_ptr(v))
    }

    /// Create a new default configuration
    pub fn new() -> UniquePtr<TileServerOptions> {
        ffi::TileServerOptions_new()
    }

    /// Get the tile server options configured for MapLibre.
    pub fn default_mapbox() -> UniquePtr<TileServerOptions> {
        ffi::TileServerOptions_mapbox()
    }

    /// Get the tile server options configured for Mapbox.
    pub fn default_maplibre() -> UniquePtr<TileServerOptions> {
        ffi::TileServerOptions_maplibre()
    }

    /// Get the tile server options configured for MapTiler.
    pub fn default_maptiler() -> UniquePtr<TileServerOptions> {
        ffi::TileServerOptions_maptiler()
    }

    /// Gets the source version prefix.
    pub fn source_version_prefix(&self) -> Option<&CStr> {
        unsafe { self.to_opt(ffi::TileServerOptions_sourceVersionPrefix(self)) }
    }

    /// Gets the style version prefix.
    pub fn style_version_prefix(&self) -> Option<&CStr> {
        unsafe { self.to_opt(ffi::TileServerOptions_styleVersionPrefix(self)) }
    }

    /// Gets the sprites version prefix.
    pub fn sprites_version_prefix(&self) -> Option<&CStr> {
        unsafe { self.to_opt(ffi::TileServerOptions_spritesVersionPrefix(self)) }
    }

    /// Gets the glyphs version prefix.
    pub fn glyphs_version_prefix(&self) -> Option<&CStr> {
        unsafe { self.to_opt(ffi::TileServerOptions_glyphsVersionPrefix(self)) }
    }

    /// Gets the tile version prefix.
    pub fn tile_version_prefix(&self) -> Option<&CStr> {
        unsafe { self.to_opt(ffi::TileServerOptions_tileVersionPrefix(self)) }
    }
}

impl Debug for TileServerOptions {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_struct("TileServerOptions")
            .field("base_url", &self.base_url())
            .field("uri_scheme_alias", &self.uri_scheme_alias())
            .field("source_template", &self.source_template())
            .field("source_domain_name", &self.source_domain_name())
            .field("source_version_prefix", &self.source_version_prefix())
            .field("style_template", &self.style_template())
            .field("style_domain_name", &self.style_domain_name())
            .field("style_version_prefix", &self.style_version_prefix())
            .field("sprites_template", &self.sprites_template())
            .field("sprites_domain_name", &self.sprites_domain_name())
            .field("sprites_version_prefix", &self.sprites_version_prefix())
            .field("glyphs_template", &self.glyphs_template())
            .field("glyphs_domain_name", &self.glyphs_domain_name())
            .field("glyphs_version_prefix", &self.glyphs_version_prefix())
            .field("tile_template", &self.tile_template())
            .field("tile_domain_name", &self.tile_domain_name())
            .field("tile_version_prefix", &self.tile_version_prefix())
            .field("api_key_parameter_name", &self.api_key_parameter_name())
            .field("requires_api_key", &self.requires_api_key())
            .field("default_style", &self.default_style())
            .finish()
    }
}

#[cfg(test)]
mod tests {
    use insta::assert_debug_snapshot;

    use super::*;

    #[test]
    fn test_defaults() {
        assert_debug_snapshot!(TileServerOptions::new(), @r#"
        TileServerOptions {
            base_url: "",
            uri_scheme_alias: "",
            source_template: "",
            source_domain_name: "",
            source_version_prefix: None,
            style_template: "",
            style_domain_name: "",
            style_version_prefix: None,
            sprites_template: "",
            sprites_domain_name: "",
            sprites_version_prefix: None,
            glyphs_template: "",
            glyphs_domain_name: "",
            glyphs_version_prefix: None,
            tile_template: "",
            tile_domain_name: "",
            tile_version_prefix: None,
            api_key_parameter_name: "",
            requires_api_key: false,
            default_style: "",
        }
        "#);
        assert_debug_snapshot!(TileServerOptions::default_mapbox(), @r#"
        TileServerOptions {
            base_url: "https://demotiles.maplibre.org",
            uri_scheme_alias: "maplibre",
            source_template: "/tiles/{domain}.json",
            source_domain_name: "",
            source_version_prefix: None,
            style_template: "{path}.json",
            style_domain_name: "maps",
            style_version_prefix: None,
            sprites_template: "/{path}/sprite{scale}.{format}",
            sprites_domain_name: "",
            sprites_version_prefix: None,
            glyphs_template: "/font/{fontstack}/{start}-{end}.pbf",
            glyphs_domain_name: "fonts",
            glyphs_version_prefix: None,
            tile_template: "/{path}",
            tile_domain_name: "tiles",
            tile_version_prefix: None,
            api_key_parameter_name: "",
            requires_api_key: false,
            default_style: "Basic",
        }
        "#);
        assert_debug_snapshot!(TileServerOptions::default_maplibre(), @r#"
        TileServerOptions {
            base_url: "https://api.mapbox.com",
            uri_scheme_alias: "mapbox",
            source_template: "/{domain}.json",
            source_domain_name: "",
            source_version_prefix: Some(
                "/v4",
            ),
            style_template: "/styles/v1{path}",
            style_domain_name: "styles",
            style_version_prefix: None,
            sprites_template: "/styles/v1{directory}{filename}/sprite{extension}",
            sprites_domain_name: "sprites",
            sprites_version_prefix: None,
            glyphs_template: "/fonts/v1{path}",
            glyphs_domain_name: "fonts",
            glyphs_version_prefix: None,
            tile_template: "{path}",
            tile_domain_name: "tiles",
            tile_version_prefix: Some(
                "/v4",
            ),
            api_key_parameter_name: "access_token",
            requires_api_key: true,
            default_style: "Streets",
        }
        "#);
        assert_debug_snapshot!(TileServerOptions::default_maptiler(), @r#"
        TileServerOptions {
            base_url: "https://api.maptiler.com",
            uri_scheme_alias: "maptiler",
            source_template: "/tiles{path}/tiles.json",
            source_domain_name: "sources",
            source_version_prefix: None,
            style_template: "/maps{path}/style.json",
            style_domain_name: "maps",
            style_version_prefix: None,
            sprites_template: "/maps{path}",
            sprites_domain_name: "sprites",
            sprites_version_prefix: None,
            glyphs_template: "/fonts{path}",
            glyphs_domain_name: "fonts",
            glyphs_version_prefix: None,
            tile_template: "{path}",
            tile_domain_name: "tiles",
            tile_version_prefix: None,
            api_key_parameter_name: "key",
            requires_api_key: true,
            default_style: "Streets",
        }
        "#);
    }

    #[test]
    fn test_rust_wrapper() {
        let result = get_42();
        assert_eq!(result, 42, "get_42() = 42");
    }

    #[test]
    fn test_log2() {
        let result = ceil_log2(1);
        assert_eq!(result, 0, "log2(1) = 0 bits needed");

        let result = ceil_log2(2);
        assert_eq!(result, 1, "log2(2) = 1 bit needed");

        let result = ceil_log2(3);
        assert_eq!(result, 2, "log2(3) -> 2 bits needed");
    }
}
