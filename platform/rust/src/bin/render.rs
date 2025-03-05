use std::fs;

use clap::Parser;
use maplibre_native::ImageRendererOptions;

/// MapLibre Native render tool
#[derive(Parser, Debug)]
struct Args {
    /// Rendering backend
    #[arg(long)]
    backend: Option<String>,

    /// API key
    #[arg(short = 't', long = "apikey", env = "MLN_API_KEY")]
    apikey: Option<String>,

    /// Map stylesheet
    #[arg(short = 's', long = "style")]
    style: Option<String>,

    /// Output file name
    #[arg(short = 'o', long = "output", default_value = "out.png")]
    output: String,

    /// Cache database file name
    #[arg(short = 'c', long = "cache", default_value = "cache.sqlite")]
    cache: String,

    /// Directory to which asset:// URLs will resolve
    #[arg(short = 'a', long = "assets", default_value = ".")]
    assets: String,

    /// Debug mode
    #[arg(long)]
    debug: bool,

    /// Image scale factor
    #[arg(short = 'r', long = "ratio", default_value_t = 1.0)]
    ratio: f64,

    /// Zoom level
    #[arg(short = 'z', long = "zoom", default_value_t = 0.0)]
    zoom: f64,

    /// Longitude
    #[arg(short = 'x', long = "lon", default_value_t = 0.0)]
    lon: f64,

    /// Latitude
    #[arg(short = 'y', long = "lat", default_value_t = 0.0)]
    lat: f64,

    /// Bearing
    #[arg(short = 'b', long = "bearing", default_value_t = 0.0)]
    bearing: f64,

    /// Pitch
    #[arg(short = 'p', long = "pitch", default_value_t = 0.0)]
    pitch: f64,

    /// Image width
    #[arg(long = "width", default_value_t = 512)]
    width: u32,

    /// Image height
    #[arg(long = "height", default_value_t = 512)]
    height: u32,

    /// Map mode (e.g. 'static', 'tile', 'continuous')
    #[arg(short = 'm', long = "mode")]
    mode: Option<String>,
}

fn main() {
    // let args = Args::parse();
    // println!("Parsed arguments: {:?}", args);

    // let s = TileServerOptions::default_maplibre();
    // let val = s.source_version_prefix();
    // println!("Parsed arguments: {:?}", val);

    let mut map = ImageRendererOptions::new().build_tile_renderer();
    map.set_style_url("https://demotiles.maplibre.org/style.json");
    let data = map.render_tile(1.0, 0, 0);
    // let data = map.render_static();
    fs::write("out.png", data.as_slice()).unwrap();
}
