# MapLibre Maps SDK for Qt

This is a community maintained MapLibre SDK for usage in Qt apps.
Both Qt5 (minimal 5.6) and Qt6 are supported.
Note that only OpenGL rendering is supported at the moment.

## Supported platforms

- macOS
- iOS
- Android
- Linux
- Windows

## Building

This project uses out of source build. CMake 3.10 or later is used to generate
make files. Ninja is recommended.

Before the project can be built Qt installation needs to be added to the `PATH`:

```shell
export PATH=<path_to_Qt>/Qt/<Qt_version>/<Qt_platform>/bin:$PATH
```

A minimal set of commands to build and install is

```shell
mkdir build && cd build
cmake ../maplibre-gl-native/ \
  -GNinja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=<installation_prefix> \
  -DMBGL_WITH_QT=ON
ninja
ninja install
```

`<installation_prefix>` above should be replaced with a wished installation path.
Files installed there can later be used as the dependency for your app.

For platform-specific details look at build scripts provided
in the [scripts](scripts) folder. Each script requires (at least) paths
to source and installation locations.
