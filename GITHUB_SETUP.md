# GitHub setup checklist

Your fork: **https://github.com/sdivjot/custom-maplibre-native**

## Already done

- [x] Fork created from [maplibre/maplibre-native](https://github.com/maplibre/maplibre-native)
- [x] Local git history rebuilt on top of upstream `main`
- [x] `main` pushed (2 commits ahead of upstream)
- [x] `feature/android-3d-rendering` branch pushed
- [x] Remotes: `origin` → fork, `upstream` → official

Verify locally:

```bash
git fetch origin upstream
git status -sb                    # should show: ## main...origin/main
git log upstream/main..main --oneline   # 2 commits
```

---

## Do these on GitHub (5 minutes)

### 1. Confirm fork badge

Open https://github.com/sdivjot/custom-maplibre-native

You should see **“forked from maplibre/maplibre-native”** and **“2 commits ahead of maplibre:main”**.

### 2. Set About / description / topics

**Settings → General → About** (gear icon on repo home):

| Field | Value |
|-------|--------|
| Description | `Experimental MapLibre Native fork: glTF 3D models, horizon view, sky layer, Android rendering` |
| Website | `https://github.com/maplibre/maplibre-native` |
| Topics | `maplibre`, `maplibre-native`, `android`, `3d-maps`, `gltf`, `fork` |

Or in terminal (after `gh auth login`):

```bash
gh auth login
gh repo edit sdivjot/custom-maplibre-native \
  --description "Experimental MapLibre Native fork: glTF 3D models, horizon view, sky layer, Android rendering" \
  --homepage "https://github.com/maplibre/maplibre-native" \
  --add-topic maplibre --add-topic maplibre-native --add-topic android \
  --add-topic 3d-maps --add-topic gltf --add-topic fork
```

### 3. View your diff vs official MapLibre

https://github.com/maplibre/maplibre-native/compare/main...sdivjot:custom-maplibre-native:main

This shows only **your** file changes on top of the official codebase.

### 4. Open a Pull Request (optional)

To show your branch on the official MapLibre repo:

1. Open https://github.com/sdivjot/custom-maplibre-native/compare/main...feature/android-3d-rendering
2. Click **Create pull request**
3. Set base repo to `maplibre/maplibre-native` → `main` (if not auto-selected)
4. Title example: `Android 3D rendering: glTF layers, horizon view, sky, fill-extrusion bevel`
5. Link to [ACCOMPLISHMENTS.md](./ACCOMPLISHMENTS.md) in the description

Or via CLI:

```bash
gh pr create \
  --repo maplibre/maplibre-native \
  --head sdivjot:feature/android-3d-rendering \
  --base main \
  --title "Android 3D rendering: glTF layers, horizon view, sky, fill-extrusion bevel" \
  --body "$(cat <<'EOF'
Experimental fork with Android-focused 3D rendering improvements.

Summary: https://github.com/sdivjot/custom-maplibre-native/blob/main/ACCOMPLISHMENTS.md

- glTF/GLB model layers (`GltfModelLayer`)
- Horizon view (FoV 0.95, 70° pitch)
- Sky band above horizon when pitched
- Soft bevel on fill-extrusion roofs
- Navigation puck fix at high zoom / pitch

Not expecting full merge as-is — opening for visibility and discussion.
EOF
)"
```

### 5. Archive old standalone repo (optional)

If you still have https://github.com/sdivjot/custom-maplibre-rendering-engine:

**Settings → General → Danger zone → Archive this repository**

Add a note in the old README pointing to `custom-maplibre-native`.

### 6. Share in MapLibre Discussions (optional)

https://github.com/maplibre/maplibre-native/discussions/new?category=show-and-tell

Brief post + link to your fork and ACCOMPLISHMENTS.md.

---

## Daily git commands

```bash
# Push new work
git push origin main

# Pull official MapLibre updates
git fetch upstream
git merge upstream/main
git push origin main

# See your custom diff
./scripts/upstream-status.sh
```

---

## Troubleshooting

**`stale info` on push:** Run `git fetch origin` first, then `git push origin main` (no force needed if you are ahead).

**`refusing to merge unrelated histories`:** Your local repo lost upstream parent — re-clone the fork or follow [UPSTREAM.md](./UPSTREAM.md).

**Push rejected (non-fast-forward):** Only use force if you intentionally rewrote history:

```bash
git fetch origin
git push origin main --force-with-lease
```
