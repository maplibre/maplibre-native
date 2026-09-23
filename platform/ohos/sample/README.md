# MapLibre Native HarmonyOS Sample

Open this directory in DevEco Studio and let it sync the project.

The default product builds the Vulkan renderer. Select the `opengl` product in
DevEco Studio to build the EGL/GLES renderer instead.

## Signing

Before installing on a device, place your AppGallery Connect debug signing files
in `sign/` with these names:

- `maplibre-debug.p12` — private keystore
- `maplibre-debug.cer` — debug certificate
- `maplibre-debug.p7b` — debug profile for bundle `org.maplibre.native.demo`

Generate the local Hvigor signing config:

```sh
node sign/generate-signing-config.mjs
```

The script writes `sign/signing.local.json` and `sign/material/`. Keep both
uncommitted.

If DevEco Studio writes a signing config into `build-profile.json5`, revert
`build-profile.json5` before sending a PR.

Keep signing passwords outside the repository.
