# Vendored style specification

`v8.json` is a **verbatim copy** of the style spec shipped by the
[`@maplibre/maplibre-gl-style-spec`](https://www.npmjs.com/package/@maplibre/maplibre-gl-style-spec)
npm package. It is refreshed by:

```
npm run copy-style-spec
```

which is literally `cp node_modules/@maplibre/maplibre-gl-style-spec/dist/latest.json
scripts/style-spec-reference/v8.json` — it **replaces the file wholesale**. Anything hand-written
in `v8.json` is silently and irrecoverably lost the next time someone runs it.

## Do not hand-edit `v8.json`

Fork-local additions and overrides belong in [`../style-spec.mjs`](../style-spec.mjs), which every
generator imports instead of the raw JSON:

- `scripts/generate-style-code.mjs`
- `platform/darwin/scripts/generate-style-code.mjs`
- `platform/android/scripts/generate-style-code.mjs` (via `android-style-spec-overrides.mjs`)

`style-spec.mjs` already deletes spec entries, patches `requires` lists, injects the whole
`location-indicator` layer, and adds internal-use properties — follow those patterns. Keeping
`v8.json` byte-identical to the npm package also keeps `git diff` against upstream meaningful.

## Known existing violations

Two fork-local changes predate this note and still live in `v8.json`. Both would be destroyed by
`copy-style-spec`:

| Location in `v8.json` | Divergence from npm |
| --- | --- |
| `layout_fill-extrusion` → `fill-extrusion-rounded-corner-distance` | Only exists here; absent from the npm package. |
| `paint_fill-extrusion` → `fill-extrusion-vertical-gradient` | `"type"` is `"verticalGradient"` here, `"boolean"` upstream. |

Migrating these into `style-spec.mjs` would let us add a CI check asserting `v8.json` matches the
npm package exactly, which would close this hole for good. Until then, treat `copy-style-spec` as a
command that needs a careful `git diff` afterwards.

## Guarding against a silent clobber

`test/style/conversion/layer.test.cpp` references the generated `fill-extrusion-shadow-*` symbols by
name, so the test target stops compiling if those properties disappear from the spec, the generator
or the templates. Prefer that kind of compile-time guard over a runtime assertion on the JSON: it
also catches regressions in the codegen itself, not just in the spec file.
