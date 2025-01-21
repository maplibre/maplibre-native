use csscolorparser::Color;

#[cxx::bridge(namespace = "rustutils")]
mod ffi {
    struct ParsedColor {
        pub success: bool,
        pub r: f32,
        pub g: f32,
        pub b: f32,
        pub a: f32,
    }

    extern "Rust" {
        fn parse_css_color(css_str: &str) -> ParsedColor;
    }
}

pub fn parse_css_color(css_str: &str) -> ffi::ParsedColor {
    css_str
        .parse::<Color>()
        .map(|color| ffi::ParsedColor {
            success: true,
            r: color.r as f32,
            g: color.g as f32,
            b: color.b as f32,
            a: color.a as f32,
        })
        .unwrap_or_else(|_| ffi::ParsedColor {
            success: false,
            r: 0.0,
            g: 0.0,
            b: 0.0,
            a: 0.0,
        })
}
