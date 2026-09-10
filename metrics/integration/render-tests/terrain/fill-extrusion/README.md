# terrain/fill-extrusion

Guards that fill-extrusion geometry is raised by the terrain elevation instead of
being drawn at sea level.

The camera looks 45 degrees down at sloping ground near Innsbruck (the same
cached `terrain-shading` DEM fixture block the other terrain tests use, z12
tiles 2178-2179 / 1433-1435) with three tall (300m) red buildings spread across
the slope and the numbered debug raster draped over the surface so the terrain
relief is visible in the image. If a backend does not sample the DEM for
fill-extrusion, the buildings sink into / float over the hillside rather than
resting on it, which is plainly visible in the diff.

The buildings are **inline GeoJSON**, so this test needs no fixtures beyond the
DEM (`local://tiles/terrain-shading/{z}-{x}-{y}.terrain.png`) and numbered
raster (`local://tiles/number/{z}.png`) tiles already in `metrics/cache-style.db`
(render tests read tiles from that SQLite cache, offline - not from the flat
files under `tiles/`).

## expected.png

Generated with the **OpenGL** render-test runner (`-u default`), which is the
backend known to elevate fill-extrusion correctly today - a Vulkan/Metal-generated
baseline would bake the floating-buildings bug in as "expected" (see the backend
parity section of TERRAIN.md).

    mbgl-render-test-runner -p metrics/linux-opengl.json -u default         -f "terrain/fill-extrusion"

The committed baseline was produced on Windows/OpenGL (AMD); treat it as
provisional until Linux CI regenerates or confirms it.

Note: generating any terrain baseline requires the first-frame terrain fix in
`RenderTerrain::prepareSource` - before it, still renders (the render tests)
always drew terrain blank because the DEM source was bound only after the
frame's drape-target pool was built.
