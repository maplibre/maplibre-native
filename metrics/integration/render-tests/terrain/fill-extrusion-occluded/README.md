# terrain/fill-extrusion-occluded

Guards that terrain in front of a fill-extrusion hides it.

Same Innsbruck `terrain-shading` DEM block and numbered raster as
`terrain/fill-extrusion`, and the same map centre, which sits on the east-west
ridge north of the Inn valley (about 2,400 m). The camera looks north from over
the valley at pitch 60, zoom 12, exaggeration 1. One red block, 600 m by 450 m
and 1,300 m tall, stands about a kilometre behind the crest in the valley
beyond (ground about 1,400 m), so its top (about 2,700 m) clears the ridge but
its lower part is behind it. The camera sits about 5 km above sea level, 8.6 km
south of the centre, well above everything in view.

Expected: the ridge cuts the block, leaving only its upper third above the
skyline, with a small peak in front of the block notching its left side. If the
extrusion is not depth-tested against the terrain mesh, the whole block is
painted over the ridge face instead, roughly three times as many red pixels.

The block is inline GeoJSON, so the test needs no fixtures beyond the DEM and
numbered raster that `terrain/fill-extrusion` already uses from
`metrics/cache-style.db`, plus seven z12 DEM tiles that the camera also asks for
and that were missing: `terrain-shading/12-{2177..2180}-1432` and
`12-{2178..2180}-1433`, the ground north of the ridge. They were added to the
database, each resampled from its z11 parent already in it (which matches the
existing z12 tiles to within about a metre on average), because no fixture set has them.

## expected.png

Generate with the OpenGL render-test runner:

    mbgl-render-test-runner -p metrics/linux-opengl.json -u default -f "terrain/fill-extrusion-occluded"

then check by eye that the block is cut by the ridge before committing it. There
is no maplibre-gl-js counterpart, so this baseline is native's own: like the
other terrain tests it uses a sea-level centre, because the centre clamp is
off in still renders. The committed image was rendered by the `linux-opengl`
runner on Mesa llvmpipe (software GL) under `xvfb-run`.
