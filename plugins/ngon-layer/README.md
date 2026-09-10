# N-gon layer

Regular convex polygons on GeoJSON or vector-tile point sources. All paint
properties support feature expressions, feature state, camera expressions, and
composite zoom interpolation. Numeric and color properties support transitions.

```json
{
  "id": "points",
  "type": "ngon",
  "source": "points",
  "paint": {
    "ngon-radius": ["interpolate", ["linear"], ["zoom"], 4, 8, 12, 28],
    "ngon-corners": ["get", "corners"],
    "ngon-rotate": ["get", "angle"],
    "ngon-color": ["get", "color"],
    "ngon-stroke-color": "#ffffff",
    "ngon-stroke-width": 2
  }
}
```

Properties: `ngon-radius`, `ngon-corners` (3–360),
`ngon-rotate` (clockwise degrees), `ngon-color`, `ngon-opacity`,
`ngon-blur`, `ngon-stroke-width`, `ngon-stroke-color`,
`ngon-stroke-opacity`, `ngon-translate`, `ngon-translate-anchor`,
`ngon-pitch-alignment`, and `ngon-pitch-scale`.
Distances are logical pixels; anchor/alignment/scale values are `map` or
`viewport`. Stroke grows outward from the polygon edge.

The host owns paint evaluation and GPU buffers; the plugin emits one quad per
point and signed-distance shaders perform polygon coverage. Layout excludes
buffered points outside the owning tile. Rendered-feature queries use the
projected polygon rather than its bounding quad.

See [build and GLFW instructions](../README.md). The 13 render fixtures cover
all feature-driven properties, fractional zoom, runtime paint updates,
feature-state changes, pitch/scale combinations, translations, tile boundaries,
blur/opacity, and zero radius.

The three checked-in backend shader sources come from one generator:

```sh
node plugins/ngon-layer/scripts/generate-shaders.mjs --check
```
