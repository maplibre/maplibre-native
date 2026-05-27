# Upstream: MapLibre Native

This repository is an **independent experimental fork** of the official [MapLibre Native](https://github.com/maplibre/maplibre-native) project.

| | Official upstream | This fork |
|---|-------------------|-----------|
| Repository | [maplibre/maplibre-native](https://github.com/maplibre/maplibre-native) | [sdivjot/custom-maplibre-rendering-engine](https://github.com/sdivjot/custom-maplibre-rendering-engine) |
| Docs | [maplibre.org/maplibre-native](https://maplibre.org/maplibre-native/) | [ACCOMPLISHMENTS.md](./ACCOMPLISHMENTS.md) |
| Maintained by | MapLibre organization | Personal / experimental |
| Android SDK base | — | **13.0.0** (see `platform/android/VERSION`) |

This fork is **not** affiliated with or endorsed by the MapLibre organization. It adds Android-focused 3D rendering features on top of the upstream codebase.

## Git remotes

```bash
origin    → https://github.com/sdivjot/custom-maplibre-rendering-engine.git  (your public repo)
upstream  → https://github.com/maplibre/maplibre-native.git                 (official MapLibre Native)
```

If `upstream` is missing, add it:

```bash
git remote add upstream https://github.com/maplibre/maplibre-native.git
```

## Git history

This repo is based on **upstream `main`** with **1 commit** of 3D rendering changes on top:

```bash
git log upstream/main..main --oneline
# b117004 Add Android 3D rendering extensions on MapLibre Native.
```

GitHub compare URL (after push):

https://github.com/maplibre/maplibre-native/compare/main...sdivjot:custom-maplibre-rendering-engine:main

## Get the GitHub “forked from” badge

GitHub only shows **“forked from maplibre/maplibre-native”** if the repo was created with the **Fork** button. To get that badge:

1. Go to [maplibre/maplibre-native](https://github.com/maplibre/maplibre-native) → **Fork** → create under your account.
2. Rename the fork to `custom-maplibre-rendering-engine` (Settings → General → Repository name).
3. Force-push this repo (full history + your commit):

```bash
git push origin main --force-with-lease
git push origin feature/android-3d-rendering
```

After that, GitHub shows **“1 commit ahead of maplibre:main”** and a file diff vs upstream.

## Show your branch on MapLibre Native (Pull Request)

You **cannot** push a branch directly to `maplibre/maplibre-native` without maintainer access. Instead:

1. Push your feature branch to **your fork**:

```bash
git push -u origin feature/android-3d-rendering
```

2. Open a Pull Request:

https://github.com/maplibre/maplibre-native/compare/main...sdivjot:custom-maplibre-rendering-engine:feature/android-3d-rendering

Your branch then appears in the PR on the official repo (for review/discussion), even if it is not merged.

## Sync workflow

### 1. Fetch latest upstream

```bash
git fetch upstream
./scripts/upstream-status.sh    # optional helper
```

### 2. See what changed upstream

```bash
# Latest upstream commit
git log upstream/main -1 --oneline

# Compare one of your modified files against upstream
git diff upstream/main -- src/mbgl/map/transform_state.cpp
git diff upstream/main -- platform/android/MapLibreAndroid/src/cpp/style/layers/gltf_model_layer_host.cpp

# List files that differ (can be large)
git diff upstream/main --stat
```

### 3. Merge upstream updates (normal merge works now)

```bash
git fetch upstream
git merge upstream/main
```

Resolve any conflicts, then push. Your 3D commit stays on top of the latest MapLibre Native.

## Link this repo on GitHub

Set these in your repo **Settings → General → About**:

- **Website:** `https://github.com/maplibre/maplibre-native`
- **Topics:** `maplibre`, `maplibre-native`, `android`, `3d-maps`, `gltf`, `fork`

Or via GitHub CLI (after `gh auth login`):

```bash
gh repo edit sdivjot/custom-maplibre-rendering-engine \
  --description "Experimental MapLibre Native fork: glTF 3D models, horizon view, sky layer, Android rendering" \
  --homepage "https://github.com/maplibre/maplibre-native" \
  --add-topic maplibre --add-topic maplibre-native --add-topic android \
  --add-topic 3d-maps --add-topic gltf --add-topic fork
```

## Contributing back to MapLibre Native

If you want features from this fork (e.g. `GltfModelLayer`, sky rendering) considered for upstream:

1. Read [MapLibre Native contributing guidelines](https://github.com/maplibre/maplibre-native/blob/main/.github/CONTRIBUTING.md).
2. Open a [Discussion](https://github.com/maplibre/maplibre-native/discussions) describing the feature and link to this repo.
3. Prepare focused PRs against `maplibre/maplibre-native` — one feature per PR, with tests.

## License

Same [BSD 2-Clause License](./LICENSE.md) as MapLibre Native. See [FORK.md](./FORK.md) for full lineage and attribution.
