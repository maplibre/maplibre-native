# Fill extrusion plugin

Replaces the built-in `fill-extrusion` layer on Vulkan and Metal. Register it with `mln_fill_extrusion_register` before
loading a style; styles keep the type `fill-extrusion`.

Its render tests are the repository's own: every test in `metrics/integration` with a `fill-extrusion` layer.
