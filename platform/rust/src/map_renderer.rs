use std::marker::PhantomData;
use std::path::Path;
use cxx::{CxxString, UniquePtr};

use crate::ffi;
use crate::ffi::MapMode;

/// A rendered map image.
///
/// The image is stored as a PNG byte array in a buffer allocated by the C++ code.
pub struct Image(UniquePtr<CxxString>);

impl Image {
    pub fn as_slice(&self) -> &[u8] {
        self.0.as_bytes()
    }
}

#[derive(Debug, Clone)]
pub struct ImageRendererOptions {
    width: u32,
    height: u32,
    pixel_ratio: f32,
    cache_path: String,
    asset_root: String,
    api_key: String,
}

impl Default for ImageRendererOptions {
    fn default() -> Self {
        Self::new()
    }
}

impl ImageRendererOptions {
    pub fn new() -> Self {
        Self {
            width: 512,
            height: 512,
            pixel_ratio: 1.0,
            cache_path: "cache.sqlite".to_string(),
            asset_root: ".".to_string(),
            api_key: "".to_string(),
        }
    }

    pub fn with_size(&mut self, width: u32, height: u32) -> &mut Self {
        self.width = width;
        self.height = height;
        self
    }

    pub fn with_pixel_ratio(&mut self, pixel_ratio: f32) -> &mut Self {
        self.pixel_ratio = pixel_ratio;
        self
    }

    pub fn with_cache_path(&mut self, cache_path: String) -> &mut Self {
        self.cache_path = cache_path;
        self
    }

    pub fn with_asset_root(&mut self, asset_root: String) -> &mut Self {
        self.asset_root = asset_root;
        self
    }

    pub fn with_api_key(&mut self, api_key: String) -> &mut Self {
        self.api_key = api_key;
        self
    }

    pub fn build_static_renderer(self) -> ImageRenderer<Static> {
        // TODO: Should the width/height be passed in here, or have another `build_static_with_size` method?
        ImageRenderer::new(MapMode::Static, self)
    }

    pub fn build_tile_renderer(self) -> ImageRenderer<Tile> {
        // TODO: Is the width/height used for this mode?
        ImageRenderer::new(MapMode::Tile, self)
    }
}

/// Internal state type to render a static map image.
pub struct Static;
/// Internal state type to render a map tile.
pub struct Tile;

/// Configuration options for a tile server.
pub struct ImageRenderer<S>(UniquePtr<ffi::MapRenderer>, PhantomData<S>);

impl<S> ImageRenderer<S> {
    /// Private constructor.
    fn new(map_mode: MapMode, opts: ImageRendererOptions) -> Self {
        let map = ffi::MapRenderer_new(
            map_mode,
            opts.width,
            opts.height,
            opts.pixel_ratio,
            &opts.cache_path,
            &opts.asset_root,
            &opts.api_key,
        );
        Self(map, PhantomData)
    }

    pub fn set_style_url(&mut self, url: &str) {
        assert!(url.contains("://"));
        ffi::MapRenderer_setStyleUrl(self.0.pin_mut(), url);
    }

    pub fn set_style_path(&mut self, path: impl AsRef<Path>) {
        let path = path.as_ref().to_str().expect("Path is not valid UTF-8");
        ffi::MapRenderer_setStyleUrl(self.0.pin_mut(), &format!("file://{path}"));
    }
}

impl ImageRenderer<Static> {
    pub fn render_static(&mut self) -> Image {
        Image(ffi::MapRenderer_render(self.0.pin_mut()))
    }
}

impl ImageRenderer<Tile> {
    pub fn render_tile(&mut self, zoom: f64, x: u64, y: u64) -> Image {
        // TODO: set tile location
        Image(ffi::MapRenderer_render(self.0.pin_mut()))
    }
}
