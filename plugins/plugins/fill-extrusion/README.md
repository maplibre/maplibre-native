# Fill extrusion plugin

The built-in `fill-extrusion` layer as the plugin layer type `org.maplibre.fill-extrusion`, on Vulkan and Metal. Register
it with `mln_fill_extrusion_register` before loading a style that uses the type.

Its render tests are the repository's own: every test in `metrics/integration` with a `fill-extrusion` layer, run with
`org.maplibre.fill-extrusion` in its place.
