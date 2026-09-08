# Incremental plugin introduction

The implementation is reviewed as a linear stack of draft PRs. Each PR targets
the preceding branch, not `main`, and has a budget of approximately 2,000 changed
lines of code. Generated fixtures are called out separately. This document is
the implementation sequence, not a promise of a stable, released ABI.

## Scope and invariants

- Plugins register **new layer types**. They cannot add properties to, intercept,
  or borrow private GPU geometry from built-in layers.
- Built-in circle, heatmap, and hillshade remain available and unchanged.
- C++ is the internal extension mechanism; an owned, validated C descriptor is an
  adapter to it. No STL objects, virtual tables, or allocations cross the C ABI.
- The host owns sources, tile scheduling, drawables, GPU uploads, shader variants,
  resource binding, transitions, and dynamic paint-property binders.
- Plugins own layout algorithms and shader sources. Callbacks receive borrowed
  inputs; results are copied before callback-owned storage can be destroyed.
- Registration precedes dependent styles, is process-wide, and cannot be undone.
  Identical registration succeeds; conflicting registrations fail atomically.

## Review sequence

1. **Review infrastructure.** Skip draft PR jobs before allocating build runners;
   enable `ready_for_review` so normal checks start when a PR leaves draft.
2. **C++ factory foundation.** Add owned runtime factory registration to
   `LayerManager`. Use the existing factory path for style, layout, bucket, and
   render-layer construction; test ownership, conflicts, and lookup.
3. **Retire experimental interfaces.** Remove the earlier plugin-layer and custom
   drawable-layer APIs in separately reviewable platform, implementation, shader,
   and fixture changes. Do not remove shaders used by ordinary built-in layers.
4. **C descriptor vocabulary.** Introduce the C-only layer/property/geometry and
   shader descriptors, explicit ownership, validation, and registration tests.
5. **Plugin style values.** Add a dedicated plugin style implementation with typed
   constants, expressions, transitions, clone/serialization, and error tests.
   Built-in `Layer::Impl` does not acquire a plugin property bag.
6. **Worker layout and buckets.** Adapt source tiles to callbacks and copy validated
   meshes into host buckets. Add asynchronous resource loading and query hooks.
7. **Drawable and shader adapter.** Register backend shader sources and resource
   layouts, build/update/remove host drawables, and bind camera/tile uniforms.
8. **Render graph.** Add generic offscreen targets, raster/DEM inputs, sampled
   textures, and ordered passes. Heatmap and hillshade are external consumers,
   not special cases in the host.
9. **Dynamic paint binding.** Reuse expression evaluation, zoom interpolation,
   feature state, transitions, and vertex ranges; validate uniform/attribute
   layouts. Pack small endpoint pairs to stay within mobile attribute limits.
10. **Platform packaging.** Export the C API on Android and Darwin, package the
    header, and expose generic platform layer wrappers and registration queries.
11. **Integration and regression coverage.** Wire builds and shared render-test
    registration, verify the retained examples, and document consumption.

Large implementation steps are split at compilation-unit boundaries into smaller
PRs. Declaration-only foundations are allowed, but no intermediate PR advertises
an available plugin layer before its host implementation is linked. Each PR
description records its prerequisite, changed-line count, and validation scope.

## Companion plugin repository

The companion stack first removes `fill-extrusion-shadows` and all sample,
publication, and render-test references to it. The remaining source-bound
examples keep their behavior and migrate to the simplified descriptor.

The new `ngon` plugin uses one quad per point and analytic convex-polygon coverage.
Changing corner count or rotation does not rebuild tile geometry. Tests cover
constant, feature-driven, composite, and feature-state paint; fractional zoom;
fill/stroke/opacity; translation; and pitched map/viewport alignment. Android,
Vulkan, OpenGL, and Metal share the same property and layout implementation.

## CI and landing

Use `gh stack submit --auto` without `--open`. The first PR's draft guards are
inherited by every later PR. Manual dispatch and main-branch jobs remain enabled;
no global workflow disabling or repository settings changes are required.
Lightweight GitHub metadata automation may still run. Promote and merge in order,
running normal CI on each ready PR and full backend render suites on the top.
