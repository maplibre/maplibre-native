#pragma once

// TEMPORARY terrain/drape diagnostics.
//
// Everything added for the macOS/Metal terrain draping investigation is gated on
// the single switch below, so the whole thing reverts by deleting this file and
// the `#include`s of it (each guarded block is marked MLN_TERRAIN_DIAG).
//
// Set to 0 to silence the logging and disable the drape dumps without removing
// any code; set to 1 to turn it back on.
#define MLN_TERRAIN_DIAG 1

// Write each terrain drape target to a PNG next to the render-test output, so CI
// can upload them as artifacts. This answers the question logging cannot: whether
// the drape texture itself contains the draped map (so the bug is in how terrain
// samples it) or is blank/black (so the bug is in the draping). Off by default
// frame. DISABLED: the dumps proved the drape targets are correct on Metal
// (pixel-identical to OpenGL), so draping is not the bug - set to 1 only if that
// needs re-checking.
#ifndef MLN_TERRAIN_DUMP_DRAPES
#define MLN_TERRAIN_DUMP_DRAPES 0
#endif

// Terrain fragment-shader probe (Metal). Everything outside the draw measures
// correct - the drape texture is opaque and pixel-identical to OpenGL, bound at
// slot 1 / location 1 - yet the surface renders black, so the next thing to
// isolate is the fragment invocation itself:
//   0 = off, sample the drape texture normally
//   1 = flat red. CONFIRMED red everywhere -> the fragment runs and the
//       problem is the sample; stays black -> the fragment is not executing or
//       its output is being discarded/overwritten.
//   2 = visualise UV as (u, v, 0, 1). CONFIRMED: u or v pinned to exactly 0
//       over the black regions - the constant-edge UV a skirt vertex carries.
//   3 = paint skirt fragments blue, sample normally elsewhere. If the blue
//       covers what used to be black, the skirts are drawing over the surface
//       instead of being hidden behind it. Sane UVs give a red/green gradient per
//       tile; black or garbage in the failing regions localises it to the UVs.
#ifndef MLN_TERRAIN_FRAG_PROBE
#define MLN_TERRAIN_FRAG_PROBE 3
#endif
