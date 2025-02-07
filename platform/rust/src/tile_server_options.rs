use std::ffi::{c_char, CStr, CString};
use std::fmt;
use std::fmt::Debug;

use cxx::UniquePtr;

use crate::ffi;

/// Configuration options for a tile server.
pub struct TileServerOptions(UniquePtr<ffi::TileServerOptions>);

/// Convert an optional C string to a pointer.
fn opt_to_ptr(value: Option<&CString>) -> *const c_char {
    value.map(|v| v.as_ptr()).unwrap_or(std::ptr::null())
}

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
    pub fn new() -> Self {
        Self(ffi::TileServerOptions_new())
    }

    /// Get the tile server options configured for MapLibre.
    pub fn default_mapbox() -> Self {
        Self(ffi::TileServerOptions_mapbox())
    }

    /// Get the tile server options configured for Mapbox.
    pub fn default_maplibre() -> Self {
        Self(ffi::TileServerOptions_maplibre())
    }

    /// Get the tile server options configured for MapTiler.
    pub fn default_maptiler() -> Self {
        Self(ffi::TileServerOptions_maptiler())
    }

    /// Sets the API base URL
    pub fn with_base_url(&mut self, base_url: &str) -> &mut Self {
        ffi::TileServerOptions_withBaseURL(self.0.pin_mut(), base_url);
        self
    }

    /// Gets the base URL.
    pub fn base_url(self: &TileServerOptions) -> &CStr {
        self.0.baseURL().as_c_str()
    }

    /// Sets the scheme alias for the tile server. For example maptiler:// for MapTiler.
    pub fn with_uri_scheme_alias(&mut self, alias: &str) -> &mut Self {
        ffi::TileServerOptions_withUriSchemeAlias(self.0.pin_mut(), alias);
        self
    }

    /// Gets the URI scheme alias.
    pub fn uri_scheme_alias(self: &TileServerOptions) -> &CStr {
        self.0.uriSchemeAlias().as_c_str()
    }

    /// Sets the template for sources.
    pub fn with_source_template(
        &mut self,
        source_template: &str,
        domain_name: &str,
        version_prefix: Option<CString>,
    ) -> &mut Self {
        unsafe {
            // Slightly inemaplibre_fficient because CString allocation is than copied to std::string
            ffi::TileServerOptions_withSourceTemplate(
                self.0.pin_mut(),
                source_template,
                domain_name,
                opt_to_ptr(version_prefix.as_ref()),
            );
        }
        self
    }

    /// Gets the source template.
    pub fn source_template(self: &TileServerOptions) -> &CStr {
        self.0.sourceTemplate().as_c_str()
    }

    /// Gets the source domain name.
    pub fn source_domain_name(self: &TileServerOptions) -> &CStr {
        self.0.sourceDomainName().as_c_str()
    }

    /// Gets the source version prefix.
    pub fn source_version_prefix(&self) -> Option<&CStr> {
        unsafe { self.to_opt(ffi::TileServerOptions_sourceVersionPrefix(&self.0)) }
    }

    /// Sets the template for styles.
    /// If set, the URL domain must contain the specified string to be matched as canonical style URL.
    pub fn with_style_template(
        &mut self,
        style_template: &str,
        domain_name: &str,
        version_prefix: Option<CString>,
    ) -> &mut Self {
        unsafe {
            // Slightly inemaplibre_fficient because CString allocation is than copied to std::string
            ffi::TileServerOptions_withStyleTemplate(
                self.0.pin_mut(),
                style_template,
                domain_name,
                opt_to_ptr(version_prefix.as_ref()),
            );
        }
        self
    }

    /// Gets the style template.
    pub fn style_template(self: &TileServerOptions) -> &CStr {
        self.0.styleTemplate().as_c_str()
    }

    /// Gets the style domain name.
    pub fn style_domain_name(self: &TileServerOptions) -> &CStr {
        self.0.styleDomainName().as_c_str()
    }

    /// Gets the style version prefix.
    pub fn style_version_prefix(&self) -> Option<&CStr> {
        unsafe { self.to_opt(ffi::TileServerOptions_styleVersionPrefix(&self.0)) }
    }

    /// Sets the template for sprites.
    /// If set, the URL domain must contain the specified string to be matched as canonical sprite URL.
    pub fn with_sprites_template(
        &mut self,
        sprites_template: &str,
        domain_name: &str,
        version_prefix: Option<CString>,
    ) -> &mut Self {
        unsafe {
            // Slightly inemaplibre_fficient because CString allocation is than copied to std::string
            ffi::TileServerOptions_withSpritesTemplate(
                self.0.pin_mut(),
                sprites_template,
                domain_name,
                opt_to_ptr(version_prefix.as_ref()),
            );
        }
        self
    }

    /// Gets the sprites template.
    pub fn sprites_template(self: &TileServerOptions) -> &CStr {
        self.0.spritesTemplate().as_c_str()
    }

    /// Gets the sprites domain name.
    pub fn sprites_domain_name(self: &TileServerOptions) -> &CStr {
        self.0.spritesDomainName().as_c_str()
    }

    /// Gets the sprites version prefix.
    pub fn sprites_version_prefix(&self) -> Option<&CStr> {
        unsafe { self.to_opt(ffi::TileServerOptions_spritesVersionPrefix(&self.0)) }
    }

    /// Sets the template for glyphs.
    /// If set, the URL domain must contain the specified string to be matched as canonical glyphs URL.
    pub fn with_glyphs_template(
        &mut self,
        glyphs_template: &str,
        domain_name: &str,
        version_prefix: Option<CString>,
    ) -> &mut Self {
        unsafe {
            // Slightly inemaplibre_fficient because CString allocation is than copied to std::string
            ffi::TileServerOptions_withGlyphsTemplate(
                self.0.pin_mut(),
                glyphs_template,
                domain_name,
                opt_to_ptr(version_prefix.as_ref()),
            );
        }
        self
    }

    /// Gets the glyphs template.
    pub fn glyphs_template(self: &TileServerOptions) -> &CStr {
        self.0.glyphsTemplate().as_c_str()
    }

    /// Gets the glyphs domain name.
    pub fn glyphs_domain_name(self: &TileServerOptions) -> &CStr {
        self.0.glyphsDomainName().as_c_str()
    }

    /// Gets the glyphs version prefix.
    pub fn glyphs_version_prefix(&self) -> Option<&CStr> {
        unsafe { self.to_opt(ffi::TileServerOptions_glyphsVersionPrefix(&self.0)) }
    }

    /// Sets the template for tiles.
    ///
    /// If set, the URL domain must contain the specified string to be matched as canonical tile URL.
    pub fn with_tile_template(
        &mut self,
        tile_template: &str,
        domain_name: &str,
        version_prefix: Option<CString>,
    ) -> &mut Self {
        unsafe {
            // Slightly inemaplibre_fficient because CString allocation is than copied to std::string
            ffi::TileServerOptions_withTileTemplate(
                self.0.pin_mut(),
                tile_template,
                domain_name,
                opt_to_ptr(version_prefix.as_ref()),
            );
        }
        self
    }

    /// Gets the tile template.
    pub fn tile_template(self: &TileServerOptions) -> &CStr {
        self.0.tileTemplate().as_c_str()
    }

    /// Gets the tile domain name.
    pub fn tile_domain_name(self: &TileServerOptions) -> &CStr {
        self.0.tileDomainName().as_c_str()
    }

    /// Gets the tile version prefix.
    pub fn tile_version_prefix(&self) -> Option<&CStr> {
        unsafe { self.to_opt(ffi::TileServerOptions_tileVersionPrefix(&self.0)) }
    }

    /// Sets the access token parameter name.
    pub fn with_api_key_parameter_name(&mut self, api_key_parameter_name: &str) -> &mut Self {
        unsafe {
            ffi::TileServerOptions_withApiKeyParameterName(
                self.0.pin_mut(),
                api_key_parameter_name,
            );
        }
        self
    }

    /// Gets the API key parameter name.
    pub fn api_key_parameter_name(self: &TileServerOptions) -> &CStr {
        self.0.apiKeyParameterName().as_c_str()
    }

    /// Sets if an API key is required.
    pub fn set_requires_api_key(&mut self, api_key_required: bool) -> &mut Self {
        ffi::TileServerOptions_setRequiresApiKey(self.0.pin_mut(), api_key_required);
        self
    }

    /// Checks if an API key is required.
    pub fn requires_api_key(self: &TileServerOptions) -> bool {
        self.0.requiresApiKey()
    }

    // /// Gets the default styles.
    //// #[cxx_name = "defaultStyles"]
    // const std::vector<mbgl::util::DefaultStyle> defaultStyles() const;

    // /// Sets the collection default styles.
    //// #[cxx_name = "withDefaultStyles"]
    // TileServerOptions& withDefaultStyles(std::vector<mbgl::util::DefaultStyle> styles);

    /// Sets the default style by name. The style name must exist in defaultStyles collection.
    pub fn with_default_style(&mut self, default_style: &str) -> &mut Self {
        unsafe {
            ffi::TileServerOptions_withDefaultStyle(self.0.pin_mut(), default_style);
        }
        self
    }

    /// Gets the default style.
    pub fn default_style(self: &TileServerOptions) -> &CStr {
        self.0.defaultStyle().as_c_str()
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

impl Default for TileServerOptions {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use insta::assert_debug_snapshot;

    use super::*;

    #[test]
    fn test_setters() {
        let mut opts = TileServerOptions::new();
        opts.with_base_url("https://example.com")
            .with_uri_scheme_alias("example")
            .with_source_template("/tiles/{domain}.json", "source_example", None)
            .with_style_template("{path}.json", "style_example", None)
            .with_sprites_template("/{path}/sprite{scale}.{format}", "sprites_example", None)
            .with_glyphs_template(
                "/font/{fontstack}/{start}-{end}.pbf",
                "glyphs_example",
                None,
            )
            .with_tile_template("/{path}", "tiles_example", None)
            .with_api_key_parameter_name("api-key")
            .set_requires_api_key(true)
            .with_default_style("abc");

        assert_debug_snapshot!(opts, @r#"
        TileServerOptions {
            base_url: "https://example.com",
            uri_scheme_alias: "example",
            source_template: "/tiles/{domain}.json",
            source_domain_name: "source_example",
            source_version_prefix: None,
            style_template: "{path}.json",
            style_domain_name: "style_example",
            style_version_prefix: None,
            sprites_template: "/{path}/sprite{scale}.{format}",
            sprites_domain_name: "sprites_example",
            sprites_version_prefix: None,
            glyphs_template: "/font/{fontstack}/{start}-{end}.pbf",
            glyphs_domain_name: "glyphs_example",
            glyphs_version_prefix: None,
            tile_template: "/{path}",
            tile_domain_name: "tiles_example",
            tile_version_prefix: None,
            api_key_parameter_name: "api-key",
            requires_api_key: true,
            default_style: "abc",
        }
        "#);

        opts.with_source_template(
            "/tiles/{domain}.json",
            "source_example",
            Some(CString::new("/source1").unwrap()),
        )
        .with_style_template(
            "{path}.json",
            "style_example",
            Some(CString::new("/style1").unwrap()),
        )
        .with_sprites_template(
            "/{path}/sprite{scale}.{format}",
            "sprites_example",
            Some(CString::new("/sprites1").unwrap()),
        )
        .with_glyphs_template(
            "/font/{fontstack}/{start}-{end}.pbf",
            "glyphs_example",
            Some(CString::new("/glyphs1").unwrap()),
        )
        .with_tile_template(
            "/{path}",
            "tiles_example",
            Some(CString::new("/tiles1").unwrap()),
        );

        assert_debug_snapshot!(opts, @r#"
        TileServerOptions {
            base_url: "https://example.com",
            uri_scheme_alias: "example",
            source_template: "/tiles/{domain}.json",
            source_domain_name: "source_example",
            source_version_prefix: Some(
                "/source1",
            ),
            style_template: "{path}.json",
            style_domain_name: "style_example",
            style_version_prefix: Some(
                "/style1",
            ),
            sprites_template: "/{path}/sprite{scale}.{format}",
            sprites_domain_name: "sprites_example",
            sprites_version_prefix: Some(
                "/sprites1",
            ),
            glyphs_template: "/font/{fontstack}/{start}-{end}.pbf",
            glyphs_domain_name: "glyphs_example",
            glyphs_version_prefix: Some(
                "/glyphs1",
            ),
            tile_template: "/{path}",
            tile_domain_name: "tiles_example",
            tile_version_prefix: Some(
                "/tiles1",
            ),
            api_key_parameter_name: "api-key",
            requires_api_key: true,
            default_style: "abc",
        }
        "#);
    }

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
}
