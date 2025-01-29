// platform/rust/src/lib.rs

#[cxx::bridge(namespace = "mbgl::util")]
mod ffi {
    // cxx allows including C++ headers directly:
    unsafe extern "C++" {
        include!("mbgl/math/log2.hpp");

        // We specify the C++ namespace and the free function name exactly.
        // cxx can bind free functions directly if they have a compatible signature.
        // The signature must match what's in log2.hpp:
        //   "uint32_t ceil_log2(uint64_t x);"
        //
        // We'll express that to Rust as (u64 -> u32).
        pub fn ceil_log2(x: u64) -> u32;
    }
}

/// A safe Rust wrapper so you can call `ceil_log2` from your code:
pub fn our_log(x: u64) -> u32 {
    ffi::ceil_log2(x)
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn test_log2() {
        let result = our_log(1);
        assert_eq!(result, 0, "log2(1) = 0 bits needed");

        let result = our_log(2);
        assert_eq!(result, 1, "log2(2) = 1 bit needed");

        let result = our_log(3);
        assert_eq!(result, 2, "log2(3) -> 2 bits needed");
    }
}
