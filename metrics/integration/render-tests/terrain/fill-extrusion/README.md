# terrain/fill-extrusion

Guards that fill-extrusion geometry is raised by the terrain elevation instead of
being drawn at sea level.

The camera is pitched 60 degrees over sloping ground near Innsbruck with three
tall (300m) red buildings spread across the slope. If a backend does not sample
the DEM for fill-extrusion, the buildings sink into / float over the hillside
rather than resting on it, which is plainly visible in the diff.

The buildings are **inline GeoJSON**, so this test needs no fixtures beyond the
terrain DEM tiles the neighbouring terrain tests already use
(`tiles/terrain-shading/{z}-{x}-{y}.terrain.png`).

## Generating expected.png

There is no baseline committed yet - it must be produced by a renderer that is
known to handle this correctly, which today means **OpenGL** (Metal and Vulkan
use the instanced fill-extrusion path, which does not sample the DEM - see the
backend parity section of TERRAIN.md).

    mbgl-render-test-runner --update-results \
        --manifest-path metrics/integration/render-tests/... 

Two cautions learned from the existing terrain tests:

1. **Generate it with an OpenGL build.** A Vulkan/Metal-generated baseline would
   bake the floating-buildings bug in as "expected".
2. **Generate it on the platform CI runs** (Linux), not on a dev machine. The
   other terrain tests' baselines were captured on-device and currently fail on
   Linux CI, partly for this reason.

Until a baseline exists the test will report as failing/missing, which is the
intended state: it documents a real defect on Metal/Vulkan.
