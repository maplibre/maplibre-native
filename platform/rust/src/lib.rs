#[cxx::bridge]
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

        // A function defined in the C++ rust wrapper rather than the core lib.
        #[namespace = "ml::rust"]
        pub fn get_42() -> u32;
    }
}

// Re-export native functions that do not need safety wrappers.
pub use ffi::{get_42, ceil_log2};

#[cfg(test)]
mod tests {
    use super::*;

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
