use std::marker::PhantomData;

use cxx::{CxxVector, UniquePtr};

use crate::ffi;
use crate::ffi::MapRenderer_setSize;

#[derive(Debug, Clone, Default)]
pub struct ImageRendererOptions {
    pixel_ratio: f32,
}

impl ImageRendererOptions {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn build_static_renderer(self) -> ImageRenderer<Static> {
        ImageRenderer::new(self.pixel_ratio)
    }

    pub fn build_tile_renderer(self) -> ImageRenderer<Static> {
        ImageRenderer::new(self.pixel_ratio)
    }
}

/// Internal state type to render a static map image.
pub struct Static;
/// Internal state type to render a map tile.
pub struct Tile;

/// Configuration options for a tile server.
pub struct ImageRenderer<State>(UniquePtr<ffi::MapRenderer>, PhantomData<State>);

impl<State> ImageRenderer<State> {
    /// Private constructor.
    fn new(_pixel_ratio: f32 /*, mode: Mode*/) -> Self {
        let map = ffi::MapRenderer_new();
        // map.set_pixel_ratio(self.pixel_ratio);
        // map.set_mode(self.mode);
        Self(map, PhantomData)
    }
}

impl ImageRenderer<Static> {
    pub fn render_static(&mut self, width: u32, height: u32) -> Image {
        MapRenderer_setSize(self.0.pin_mut(), width, height);
        Image(ffi::MapRenderer_render(self.0.pin_mut()))
    }
}

impl ImageRenderer<Tile> {
    pub fn render_tile(&mut self, zoom: f64, x: u64, y: u64) -> Image {
        MapRenderer_setSize(self.0.pin_mut(), 512, 512);
        // TODO: set tile location
        Image(ffi::MapRenderer_render(self.0.pin_mut()))
    }
}

pub struct Image(UniquePtr<CxxVector<u8>>);

impl Image {
    pub fn as_slice(&self) -> &[u8] {
        self.0.as_ref().unwrap().as_slice()
    }
}
