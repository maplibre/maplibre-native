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

## Why `git merge upstream/main` fails

This repo was published with a **fresh git history** (single initial commit). Upstream has years of commits. Git treats them as **unrelated histories** and refuses a normal merge.

**Do not run** `git merge upstream/main --allow-unrelated-histories` — that attempts to merge two entire codebases and produces massive conflicts.

## Sync workflow (recommended)

Use `upstream` to **fetch, compare, and selectively port** changes — not to merge wholesale.

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

### 3. Port a specific upstream fix

```bash
git fetch upstream
git cherry-pick <commit-sha>    # only if the patch applies cleanly
```

If cherry-pick fails, open the upstream commit on GitHub and apply the change manually to the matching files in this repo.

### 4. Rebase onto a newer upstream release (major upgrade)

When jumping many versions (e.g. 13.0.0 → 13.2.0):

1. Clone official MapLibre Native at the target tag in a separate directory.
2. Copy your custom files / diffs from this repo (glTF layer, transform changes, shaders, test app).
3. Build and fix conflicts manually.
4. Update `platform/android/VERSION` and note the new base in this file.

There is no one-command merge because of the unrelated histories.

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
