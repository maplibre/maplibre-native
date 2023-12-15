# Linux

This guide explains how to get started building and running MapLibre Native on Linux. The guide focusses on a Ubuntu 22.04 clean docker image, but should be adaptible to other distributions. The build process should give you a set of `.a` files that you can use to include MapLibre Native in other C++ projects, as well as a set of executables that you can run to render map tile images and test the project.

## Requirements

The following system libraries need to be installed.

```bash
apt install libcurl4-openssl-dev libglfw3-dev libuv1-dev libpng-dev libicu-dev libjpeg8-dev libwebp-dev xvfb
```

Optional: `libsqlite3-dev` (also available as vendored dependency).

The following tools need to be available.

```bash
apt install clang git cmake ccache ninja-build pkg-config
```

## Docker

You can use a Docker container to build MapLibre Native. A `Dockerfile` that installes the required dependencies when the image is built is provided in this directory.

Build image with:
```bash
# in platform/linux
docker build -t maplibre-native-image .
```

Run image with:
```bash
# in repo root directory
docker run --rm -it -v "$(pwd)":/root/ maplibre-native-image
```

## Build

First, clone the repository. This repository uses [git submodules](https://git-scm.com/book/en/v2/Git-Tools-Submodules), that are required to build the project.

```bash
git clone --recurse-submodules -j8 https://github.com/maplibre/maplibre-native.git
cd maplibre-native
```

To create the build, run the following commands from the root of the project:

```bash
cmake -B build -GNinja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=RelWithDebInfo -DMLN_WITH_CLANG_TIDY=OFF -DMLN_WITH_COVERAGE=OFF -DMLN_DRAWABLE_RENDERER=ON -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON
```

```bash
cmake --build build --target mbgl-render -j $(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null)
```

## `mbgl-render`

If all went well, there should now be a `./build/bin/mbgl-render` binary that you can run to generate map tile images. To test that it is working properly, run the following command.

```bash
./build/bin/mbgl-render --style https://raw.githubusercontent.com/maplibre/demotiles/gh-pages/style.json --output out.png
```

> I get an error `Error: Failed to open X display.` when I run this command.

If you're setting up MapLibre Native on a headless server (i.e. one without a display), you'll need to simulate an X server to do any rendering. Install `xvfb` and `xauth` and run the following command:

```bash
xvfb-run -a ./build/bin/mbgl-render --style https://raw.githubusercontent.com/maplibre/demotiles/gh-pages/style.json --output out.png
```

This should produce an `out.png` map tile image with the default MapLibre styling from [the MapLibre demo](https://maplibre.org/).

![Sample image of world from mbgl-render command](/misc/sample-maplibre-style-mbgl-render-out.png)

### Using your own style/tiles 

You can also use the `mbgl-render` command to render images from your own style or tile set. To do so, you will need a data source and a style JSON file.

For the purposes of this exercise, you can use the `zurich_switzerland.mbtiles` from [here](https://github.com/acalcutt/tileserver-gl/releases/download/test_data/zurich_switzerland.mbtiles), and [this](https://gist.github.com/louwers/d7607270cbd6e3faa05222a09bcb8f7d) following `style.json` file. Download both by running the commands below.

```
wget https://github.com/acalcutt/tileserver-gl/releases/download/test_data/zurich_switzerland.mbtiles
wget https://gist.githubusercontent.com/louwers/d7607270cbd6e3faa05222a09bcb8f7d/raw/4e9532e1760717865df8aeff08f9bcf100f9e8c4/style.json
```

Note that this style is totally inadequate for any real use beyond testing your custom setup. Replace the source URL `mbtiles:///path/to/zurich_switzerland.mbtiles` with the actual path to your `.mbtiles` file. You can use this command if you downloaded both files to the working directory:

```bash
sed -i "s#/path/to#$PWD#" style.json 
```

Next, run the following command.

```bash
./build/bin/mbgl-render --style style.json --output out.png
```

This should produce an `out.png` image in your current directory with a barebones image of the world.

![Sample image of world from mbgl-render command](/misc/sample-barebones-mbgl-render-out.png)