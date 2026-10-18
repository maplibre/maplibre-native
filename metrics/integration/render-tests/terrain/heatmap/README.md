# terrain/heatmap

Guards that heatmap kernels are lifted onto the terrain instead of being drawn at
sea level.

Same Innsbruck `terrain-shading` DEM block and numbered raster as
`terrain/fill-extrusion`, and the same map centre, which sits on the east-west
ridge north of the Inn valley (about 2,400 m). The camera looks north at pitch
60, zoom 12, exaggeration 1. Three clusters of four heatmap points are spread
across the slope: one on the ridge at the centre and one on each side of it, at
the places the `terrain/fill-extrusion` buildings stand, with the default colour
ramp, radius 24 and intensity 1.5. The numbered raster is draped over the surface
so the relief is visible in the image.

Expected: each blob is registered to the ground under its centre. If the kernels
are not lifted by the DEM, they are projected as if the ground were at sea level,
so a blob at 2,400 m is drawn about a ridge-height lower on screen than its
ground position, off the slope it belongs to.

The heatmap's colour-ramp composite is a screen-space pass and still draws over
the terrain, so this test does not look at occlusion by ridges.

The points are **inline GeoJSON**, so the test needs no fixtures beyond the DEM
(`local://tiles/terrain-shading/{z}-{x}-{y}.terrain.png`) and numbered raster
(`local://tiles/number/{z}.png`) tiles already in `metrics/cache-style.db`.

## expected.png

Not committed yet. Generate it with the **OpenGL** render-test runner:

    mbgl-render-test-runner -p metrics/linux-opengl.json -u default -f "terrain/heatmap"

then check by eye that each blob sits on its slope before committing it. There is
no maplibre-gl-js counterpart for a pitched native frame, so this baseline is
native's own; like the other terrain tests it uses a sea-level centre, because
the centre clamp is off in still renders.
