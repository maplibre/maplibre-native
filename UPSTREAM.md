# Upstream: MapLibre Native

This repository is a **GitHub fork** of [MapLibre Native](https://github.com/maplibre/maplibre-native).

| | Official upstream | This fork |
|---|-------------------|-----------|
| Repository | [maplibre/maplibre-native](https://github.com/maplibre/maplibre-native) | [sdivjot/custom-maplibre-native](https://github.com/sdivjot/custom-maplibre-native) |
| Docs | [maplibre.org/maplibre-native](https://maplibre.org/maplibre-native/) | [ACCOMPLISHMENTS.md](./ACCOMPLISHMENTS.md) |
| Maintained by | MapLibre organization | Personal / experimental |
| Android SDK base | — | **13.0.0** (see `platform/android/VERSION`) |

This fork is **not** affiliated with or endorsed by the MapLibre organization.

## Git remotes

```bash
origin    → https://github.com/sdivjot/custom-maplibre-native.git
upstream  → https://github.com/maplibre/maplibre-native.git
```

One-time setup:

```bash
git remote add upstream https://github.com/maplibre/maplibre-native.git   # if missing
git fetch upstream
git fetch origin
```

## Your commits on top of upstream

```bash
git log upstream/main..main --oneline
```

Expected:

```
a35e353 Update UPSTREAM.md for fork-based workflow and shared git history.
b117004 Add Android 3D rendering extensions on MapLibre Native.
```

## Compare your changes vs official MapLibre Native

**All changes on `main`:**

https://github.com/maplibre/maplibre-native/compare/main...sdivjot:custom-maplibre-native:main

**Feature branch only:**

https://github.com/maplibre/maplibre-native/compare/main...sdivjot:custom-maplibre-native:feature/android-3d-rendering

## Open a Pull Request to MapLibre Native

Your branch appears on the official repo when you open a PR from your fork:

https://github.com/sdivjot/custom-maplibre-native/compare/main...feature/android-3d-rendering

Or cross-repo:

https://github.com/maplibre/maplibre-native/compare/main...sdivjot:custom-maplibre-native:feature/android-3d-rendering

## Sync with upstream (ongoing)

```bash
git fetch upstream
git merge upstream/main
git push origin main
```

Inspect your 3D diff anytime:

```bash
./scripts/upstream-status.sh
git diff upstream/main --stat
```

## GitHub repo settings (About section)

In https://github.com/sdivjot/custom-maplibre-native → **Settings → General → About**:

- **Description:** `Experimental MapLibre Native fork: glTF 3D models, horizon view, sky layer, Android rendering`
- **Website:** `https://github.com/maplibre/maplibre-native`
- **Topics:** `maplibre`, `maplibre-native`, `android`, `3d-maps`, `gltf`, `fork`

Or after `gh auth login`:

```bash
gh repo edit sdivjot/custom-maplibre-native \
  --description "Experimental MapLibre Native fork: glTF 3D models, horizon view, sky layer, Android rendering" \
  --homepage "https://github.com/maplibre/maplibre-native" \
  --add-topic maplibre --add-topic maplibre-native --add-topic android \
  --add-topic 3d-maps --add-topic gltf --add-topic fork
```

## License

Same [BSD 2-Clause License](./LICENSE.md) as MapLibre Native. See [FORK.md](./FORK.md) for lineage.
