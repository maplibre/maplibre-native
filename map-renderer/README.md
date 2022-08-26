# Map Renderer
Command line utility to render a Maplibre map (defined via a style json) to a raster image.

## Build
```
cmake -S . -B build -GNinja -DMBGL_WITH_MAP_RENDERER=ON
cmake --build build --target map-renderer
```

## Install
Easiest way to build and install map-renderer on MacOS is via homebrew:
```
brew tap pointrlabs/vendor
brew install maplibre-map-renderer
```

If you are on another platform or want to build from source, follow the steps in "Build" and then:
```
cmake --install build
```
(sudo may be required on M1 macs)

## Usage
```
Renders MapLibre maps to raster images. Either center/zoom or bounds should be provided.
Usage:
  map-renderer [OPTION...]

  -s, --style arg     Path to style json file
  -o, --output arg    [Optional] Output png file path (default: map.png)
      --metadata arg  [Optional] Path to metadata json file containing
                      extra info about the generated map image
      --center arg    Center of map <longitude,latitude> (no spaces)
      --zoom arg      Zoom level (default: 19)
      --bounds arg    Map bounds <lon-west,lat-south,lon-east,lat-north>
                      (no spaces)
      --padding arg   [Optional] Padding in pixels (only applicable when
                      bounds are provided) (default: 0)
      --width arg     [Optional] Image width (default: 1024)
      --height arg    [Optional] Image height (default: 1024)
      --ratio arg     [Optional] Pixel ratio (default: 4)
      --source arg    [Optional] Vector tiles url for source_ptr
      --bid arg       [Optional] Building id to filter style
      --lvl arg       [Optional] Level index to filter style
      --verbose       Verbose mode
  -h, --help          Print usage
```
