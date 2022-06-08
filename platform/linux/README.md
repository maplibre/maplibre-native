# Linux

A simple executable and test suite for Linux based on [MapLibre GL Native](../../README.md).

This guide focuses on Debian and Ubuntu distributions.

The build process should give you a set of `.a` files that you can use to include MapLibre GL Native in other C++ projects, as well as a set of executables that you can run to render map tile images and test the project.

## Prerequisites

The following dependencies are required to build MapLibre GL Native on Debian 11.

```bash
apt install ccache cmake ninja-build pkg-config xvfb libcurl4-openssl-dev libglfw3-dev libuv1-dev g++-10 libc++-9-dev libc++abi-9-dev libpng-dev libgl1-mesa-dev libgl1-mesa-dri
```

There are two required packages that are not available in the standard Debian package repository: `libjpeg-turbo8` and `libicu66`. You can install them by downloading and installing the `.deb` packages.

```bash
wget http://archive.ubuntu.com/ubuntu/pool/main/libj/libjpeg-turbo/libjpeg-turbo8_2.0.3-0ubuntu1_amd64.deb
apt install ./libjpeg-turbo8_2.0.3-0ubuntu1_amd64.deb
wget http://archive.ubuntu.com/ubuntu/pool/main/i/icu/libicu66_66.1-2ubuntu2_amd64.deb
apt install ./libicu66_66.1-2ubuntu2_amd64.deb
```

On an Ubuntu based distribution, you may be able to install these using `apt`.

```bash
apt install libjpeg-turbo8 libicu66
```

## Build

First, clone the repository. This repository uses git submodules, which are also required to build the project.

```bash
git clone --recurse-submodules -j8 https://github.com/maplibre/maplibre-gl-native.git
cd maplibre-gl-native
```

To create the build, run the following commands.

```bash
cmake . -B build -G Ninja -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_C_COMPILER=gcc-10 -DCMAKE_CXX_COMPILER=g++-10
cmake --build build -j $(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null)
```

If all went well, there should now be a `maplibre-gl-native/build/bin/mbgl-render` binary that you can run to generate map tile images. To test that it is working properly, run the following command.

```bash
./build/bin/mbgl-render --style https://raw.githubusercontent.com/maplibre/demotiles/gh-pages/style.json --output out.png
```

> I get an error `Error: Failed to open X display.` when I run this command.

If you're setting up MapLibre GL Native on a headless server (i.e. one without a display), you'll need to simulate an X server to do any rendering.

```bash
xvfb-run -a ./build/bin/mbgl-render --style https://raw.githubusercontent.com/maplibre/demotiles/gh-pages/style.json --output out.png
```

This should produce an `out.png` map tile image with the default MapLibre styling from [the MapLibre demo](https://maplibre.org/).

![Sample image of world from mbgl-render command](/misc/sample-maplibre-style-mbgl-render-out.png)

### Using your own style/tiles 

You can also use the `mbgl-render` command to render images from your own style or tile set. To do so, you will need a data source and a style JSON file.

For the purposes of this exercise, you can use the `zurich_switzerland.mbtiles` from [here](https://github.com/acalcutt/tileserver-gl/releases/download/test_data/zurich_switzerland.mbtiles), and the following `style.json` file.

```json
{
  "version": 8,
  "name": "Test style",
  "center": [
    8.54806714892635,
    47.37180823552663
  ],
  "sources": {
    "test": {
      "type": "vector",
      "url": "mbtiles:///path/to/zurich_switzerland.mbtiles"
    }
  },
  "layers": [
    {
      "id": "background",
      "type": "background",
      "paint": {
        "background-color": "hsl(47, 26%, 88%)"
      }
    },
    {
      "id": "water",
      "type": "fill",
      "source": "test",
      "source-layer": "water",
      "filter": [
        "==",
        "$type",
        "Polygon"
      ],
      "paint": {
        "fill-color": "hsl(205, 56%, 73%)"
      }
    },
    {
      "id": "admin_country",
      "type": "line",
      "source": "test",
      "source-layer": "boundary",
      "filter": [
        "all",
        [
          "<=",
          "admin_level",
          2
        ],
        [
          "==",
          "$type",
          "LineString"
        ]
      ],
      "layout": {
        "line-cap": "round",
        "line-join": "round"
      },
      "paint": {
        "line-color": "hsla(0, 8%, 22%, 0.51)",
        "line-width": {
          "base": 1.3,
          "stops": [
            [
              3,
              0.5
            ],
            [
              22,
              15
            ]
          ]
        }
      }
    }
  ]
}
```

Note that this style is totally inadequate for any real use beyond testing your custom setup. Don't forget to replace the source URL `"mbtiles:///path/to/zurich_switzerland.mbtiles"` with the actual path to your mbtiles file.

From your `maplibre-gl-native/` dir, run the following command.

```bash
./build/bin/mbgl-render --style /path/to/style.json --output out.png
```

This should produce an `out.png` image in your current directory with a barebones image of the world.

![Sample image of world from mbgl-render command](/misc/sample-barebones-mbgl-render-out.png)

