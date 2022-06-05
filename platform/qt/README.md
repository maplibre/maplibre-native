# MapLibre Maps SDK for Qt

## Developing

This is the foundation for the [Mapbox GL plugin](https://doc.qt.io/qt-5/location-plugin-mapboxgl.html)
available in the Qt SDK since Qt 5.9. Use the [Qt bugtracker](https://bugreports.qt.io) for bugs related
to the plugin and this GitHub repository for bugs related to MapLibre GL Native and the Qt bindings.

You should build this repository if you want to develop/contribute using the low level Qt C++ bindings or
want to contribute to the MapLibre GL Native project using the Qt port for debugging.

### Build dependencies

#### Linux

For Linux (tested on Ubuntu) desktop, together with these [build instructions](../linux/README.md), you also need:

```
$ sudo apt-get install qt5-default
```

#### macOS

For macOS desktop, you can install Qt 5 via [Homebrew](https://brew.sh):

```
$ brew install qt5
```

Since Homebrew doesn't add Qt to the path, you'll have to do that manually before running any Make target:

```
export PATH=/usr/local/opt/qt5/bin:$PATH
```

### Windows

The Windows build will assume you have installed and on the default path:

- Microsoft Visual Studio 2019
- [CMake 3.10.1+](https://cmake.org/download/)
- [Ninja](https://github.com/ninja-build/ninja/releases)
- [Qt 5.4+](https://www.qt.io/download) with "msvc2019" (or later) support.

At runtime, you will also need installed:

- [OpenSSL 1.0.2+](https://slproweb.com/products/Win32OpenSSL.html)
- DirectX

#### QNX 7.0

To build for QNX 7.0, you need to install the QNX Software Development Platform (SDP) on a Linux host.

http://www.qnx.com/developers/docs/7.0.0/#com.qnx.doc.qnxsdp.nav/topic/bookset.html

After installing the SDP, you need to source the QNX environment script:

```
source <SDP_DIRECTORY>/qnxsdp-env.sh
```

You also need to Build Qt for QNX.

http://wiki.qt.io/Building_Qt_for_QNX_Neutrino_OS

After building Qt for QNX, you need to add Qt installation's bin directory to the path.

```
export PATH=<INSTALLFOLDER>/bin:$PATH
```

### Build instructions

Public API headers can be found in the [platform/qt/include](include) directory.

#### Linux and macOS

#### QMapboxGL example application

```
$ make qt-lib      # Will build libqmapboxgl.so
$ make qt-app      # Will build the test app and libqmapboxgl.so if not built yet
$ make run-qt-app  # Will build and run the test app
```

#### Windows

```
$ mkdir build
$ cd build
$ cmake -G Ninja -DMBGL_WITH_QT=ON -DCMAKE_BUILD_TYPE=Release ..
$ ninja
```

You may need to specify the path to Qt:

```
$ cmake -G Ninja -DMBGL_WITH_QT=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:\Qt\5.15.2\msvc2019_64\lib\cmake ..
```


#### QNX 7.0

Building the repository for QNX 7.0 is very similar to other platforms (e.g. Linux and macOS).

```
$ make qnx-qt-lib      # Will build libqmapboxgl.so
```
