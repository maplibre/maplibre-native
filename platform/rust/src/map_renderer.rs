use std::ffi::{c_char, CStr, CString};
use std::fmt;
use std::fmt::Debug;

use cxx::{CxxVector, UniquePtr};

use crate::ffi;

/// Configuration options for a tile server.
pub struct MapRenderer(UniquePtr<ffi::MapRenderer>);

impl MapRenderer {
    /// Create a new default configuration
    pub fn new() -> Self {
        Self(ffi::MapRenderer_new())
    }

    pub fn set_size(&mut self, width: u32, height: u32) -> &mut Self {
        ffi::MapRenderer_setSize(self.0.pin_mut(), width, height);
        self
    }

    pub fn render(&mut self) -> UniquePtr<CxxVector<u8>> {
        ffi::MapRenderer_render(self.0.pin_mut())
    }
}

impl Default for MapRenderer {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use insta::assert_debug_snapshot;

    use super::*;
}
