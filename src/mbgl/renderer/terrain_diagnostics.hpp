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
