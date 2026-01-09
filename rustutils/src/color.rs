use csscolorparser::Color;

#[cxx::bridge(namespace = "rustutils")]
mod ffi {
    // TODO: Use #[cfg_attr(test, derive(...))] once supported
    // See https://github.com/dtolnay/cxx/issues/1022
    #[derive(Debug, PartialEq)]
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
            r: color.r,
            g: color.g,
            b: color.b,
            a: color.a,
        })
        .unwrap_or_else(|_| ffi::ParsedColor {
            success: false,
            r: 0.0,
            g: 0.0,
            b: 0.0,
            a: 0.0,
        })
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_css_color() {
        assert_eq!(
            parse_css_color("rgb(255, 0, 0)"),
            ffi::ParsedColor {
                success: true,
                r: 1.0,
                g: 0.0,
                b: 0.0,
                a: 1.0,
            }
        );
        assert_eq!(
            parse_css_color("rgba(255, 0, 0, 0.5)"),
            ffi::ParsedColor {
                success: true,
                r: 1.0,
                g: 0.0,
                b: 0.0,
                a: 0.5,
            }
        );
        assert_eq!(
            parse_css_color("invalid"),
            ffi::ParsedColor {
                success: false,
                r: 0.0,
                g: 0.0,
                b: 0.0,
                a: 0.0,
            }
        );
    }
}
