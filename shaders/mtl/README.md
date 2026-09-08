# Metal shader sources

`manifest.json` maps each built-in shader to its Metal source, shared prelude,
and C++ reflection header. Edit the `.metal` files to change shader code; add a
manifest entry when adding a shader. Attribute and texture reflection remains in
`include/mln/shaders/mtl` and `src/mln/shaders/mtl`.

Both Bazel and CMake run `shaders/generate_metal_shader_code.mjs` during a Metal
build. It stores each shared fragment once, compresses the concatenation with
zlib, and emits C++ source accessors. Generated C++ belongs in the build tree.
To inspect the output manually:

```sh
node shaders/generate_metal_shader_code.mjs --out /tmp/metal-shader-source.cpp
node --test shaders/generate_metal_shader_code.test.mjs
```

The generator records each fragment's UTF-8 byte offset and length in the bundle.
The first shader access decompresses the bundle once; subsequent accesses return
views into the same buffer. At runtime, the common prelude, shader prelude, and
shader source are concatenated before being passed to Metal.

The `validate-scripts` CI workflow runs the generator tests.
