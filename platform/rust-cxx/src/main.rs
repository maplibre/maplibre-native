// cxx wrapper for maplibre-native
#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        include!("maplibre-native/include/maplibre-native.h");

        type MaplibreNative;

        fn new_maplibre_native() -> UniquePtr<MaplibreNative>;
        fn render(&self, style: &str, width: u32, height: u32) -> Vec<u8>;
    }
}
