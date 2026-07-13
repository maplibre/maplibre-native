# Tracy profiling

#### Introduction

MapLibre Native integrates [Tracy profiler](https://github.com/wolfpld/tracy) which offers an easy way to understand and optimize your application's CPU and GPU performance.
Tracy mainly consists in manually adding markup instrumentation in the code to log performance events. These events can then be analyzed and visualized using the [Tracy Profiler tool](https://github.com/wolfpld/tracy/tree/master/profiler).

Instrumentation is generally the first step in profiling applications that use MapLibre. Once slow inner-loop routines are identified, additional hardware vendor specific tools can be used to collect hardware counters and optimize low level CPU and GPU code.

#### Tracy client

Tracy client consists of an API to mark CPU and GPU performance zones. A zone is a code section where the start and end timestamps are recorded.

#### Tracy server

The server is the Tracy profiler that allows the analysis and visualization of the client recorded data.
The server can be downloaded from [Tracy release page](https://github.com/wolfpld/tracy/releases) or it can be easily built from [sources](https://github.com/wolfpld/tracy/tree/master/profiler) on Linux, Windows or Mac using CMake

#### Enabling instrumentation in MapLibre Native

Instrumentation is enabled by turning `ON` the CMake option `MLN_USE_TRACY`.
Tracy computational overhead is very low but by default it keeps all instrumentation events that are not consumed by the server in system memory. This can have a negative effect on platforms with low memory. To prevent high memory usage, `TRACY_ON_DEMAND` macro should defined. This way instrumentation data is only stored when the server is connected to the application.

#### Instrumentation in MapLibre

The file `include/mbgl/util/instrumentation.hpp` defines the following instrumentation macros:

##### `MLN_TRACE_ZONE(label)`
The macro records the timestamps at the start and end of the code scope. The parameter label is a user defined name for the zone. Example:

~~~cpp
// code is not instrumented
{
  MLN_TRACE_ZONE(EmptyZone) // Records from here until the end of the scope
  // code here is instrumented
}
// other here not instrumented
~~~

##### `MLN_TRACE_FUNC()`
The macro is meant to be placed at the start of a function and expands to:
~~~cpp
MLN_TRACE_ZONE(__FUNCTION__)
~~~

##### GPU instrumentation

OpenGL is also supported in MapLibre native. Tracy support is currently missing for other APIs such as Metal and need to be added separately.

##### `MLN_TRACE_GL_ZONE(label)`
This macro is similar to `MLN_TRACE_ZONE` except that [OpenGL timestamp queries](https://www.khronos.org/opengl/wiki/Query_Object) are inserted in the GPU command buffer instead of recording CPU time.

##### `MLN_TRACE_FUNC_GL(label)`
This macro is similar to `MLN_TRACE_FUNC` except that [OpenGL timestamp queries](https://www.khronos.org/opengl/wiki/Query_Object) are inserted in the GPU command buffer instead of recording CPU time.

##### Other macros

The above macros can be added inside MapLibre code and also in the application code that calls MapLibre.

The following macros should only be used if there are changes to MapLibre internals:

##### `MLN_END_FRAME()`
Mark the end of a frame.

##### `MLN_TRACE_GL_CONTEXT()`
Placed after an OpenGL context is created.

##### `MLN_TRACE_ALLOC_TEXTURE(id, size)` and `MLN_TRACE_FREE_TEXTURE(id)`
Record a read-only texture allocation and deallocation

##### `MLN_TRACE_ALLOC_RT(id, size)` and `MLN_TRACE_FREE_RT(id)`
Record a render target texture allocation and deallocation

##### `MLN_TRACE_ALLOC_VERTEX_BUFFER(id, size)` and `MLN_TRACE_FREE_VERTEX_BUFFER(id)`
Record a buffer allocation and deallocation that is intended to be used as a read-only vertex buffer

##### `MLN_TRACE_ALLOC_INDEX_BUFFER(id, size)` and `MLN_TRACE_FREE_INDEX_BUFFER(id)`
Record a buffer allocation and deallocation that is intended to be used as a read-only index buffer

##### `MLN_TRACE_ALLOC_CONST_BUFFER(id, size)` and `MLN_TRACE_FREE_CONST_BUFFER(id)`
Record a buffer allocation and deallocation that is intended to be used as a constant buffer


#### Usage example on Linux and Windows

Download or build the Tracy profiler (server) and run it.

Make sure you generate the MapLibre project with the option `MLN_USE_TRACY` enabled.

As an example, the glfw sample is used.

With CMake, in MapLibre repository root do
~~~bash
# generate project
cmake -B build -GNinja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=RelWithDebInfo -DMLN_WITH_CLANG_TIDY=OFF -DMLN_WITH_COVERAGE=OFF -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON -DMLN_USE_TRACY=ON
# build
cmake --build build --target mbgl-glfw -j 8
# run
./build/platform/glfw/mbgl-glfw --style https://raw.githubusercontent.com/maplibre/demotiles/gh-pages/style.json --benchmark
~~~
with Bazel
~~~bash
# build and run
bazel run //platform/glfw:glfw_app -- --style https://raw.githubusercontent.com/maplibre/demotiles/gh-pages/style.json --benchmark
~~~

In the Tracy Profiler hit the connect button (or select the glfw application from the list of applications that are running Tracy Client). Profile then optimize the code.

#### Connecting the profiler to a MapLibre Android application
The Android application communicates instrumentation data to the profiler (Tracy server) on the network using port 8086 by default. You can expose the port to the profiler using Android Debug Bridge by running the command:
~~~bash
adb forward tcp:8086 tcp:8086
~~~

#### More information and advanced usage in Tracy

- [Tracy Github page](https://github.com/wolfpld/tracy/)
- [Tracy user guide](https://github.com/wolfpld/tracy/releases/latest/download/tracy.pdf)
- [Tracy demo on Youtube](https://www.youtube.com/watch?v=fB5B46lbapc)
- [Tracy CppCon presentation on Youtube](https://www.youtube.com/watch?v=ghXk3Bk5F2U&t=37s)
