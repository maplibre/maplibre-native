# Metal shader sources

`manifest.json` maps each built-in shader to its Metal source, shared prelude,
and C++ reflection header. Attribute and texture reflection remains in
`include/mln/shaders/mtl` and `src/mln/shaders/mtl`.

Both Bazel and CMake run `shaders/generate_metal_shader_code.mjs` during a Metal
build. It stores each shared fragment once, compresses the concatenation with
zlib, and emits C++ source accessors. Generated C++ belongs in the build tree.
To inspect the output manually:

```sh
node shaders/generate_metal_shader_code.mjs --out /tmp/metal-shader-source.cpp
node --test shaders/generate_metal_shader_code.test.mjs
```

Shader text is preserved byte for byte, including comments, whitespace, and
preprocessor directives. Offsets are UTF-8 byte offsets. The first shader access
decompresses the bundle once using a thread-safe function-local static; subsequent
accesses return views into the immutable buffer. This retains 198 KiB of source data for a smaller installed binary (about
256 KiB of string capacity with the measured Apple libc++ toolchain). Source assembly
still produces a null-terminated string before calling Metal.
