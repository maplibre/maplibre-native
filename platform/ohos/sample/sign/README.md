# Local debug signing

Place your AGC debug files here (gitignored):

- `maplibre-debug.p12` — private keystore (generated locally with `hap-sign-tool`)
- `maplibre-debug.cer` — debug certificate from AppGallery Connect
- `maplibre-debug.p7b` — debug profile for bundle `org.maplibre.native.demo`
- `profile.json` — local unsigned debug profile; copy `profile.example.json` if needed

Keep signing passwords outside the repository.

## Build and sign from the command line

`hvigor` expects encrypted 32+ character passwords in `build-profile.json5`, so sign the
unsigned HAP manually after `assembleApp`:

```sh
cd platform/ohos/sample
/path/to/command-line-tools/bin/hvigorw assembleApp --no-daemon

SIGN_LIB=/path/to/command-line-tools/sdk/default/openharmony/native/../toolchains/lib
# or: sdk/default/openharmony/toolchains/lib/hap-sign-tool.jar
java -jar "$SIGN_LIB/hap-sign-tool.jar" sign-app \
  -mode localSign \
  -keyAlias maplibre_debug \
  -keyPwd 'YOUR_PASSWORD' \
  -signAlg SHA256withECDSA \
  -appCertFile ./sign/maplibre-debug.cer \
  -profileFile ./sign/maplibre-debug.p7b \
  -inFile entry/build/default/outputs/default/entry-default-unsigned.hap \
  -keystoreFile ./sign/maplibre-debug.p12 \
  -keystorePwd 'YOUR_PASSWORD' \
  -compatibleVersion 21 \
  -outFile ./sign/entry-default-signed.hap

HDC=/path/to/command-line-tools/sdk/default/openharmony/toolchains/hdc
$HDC install -r ./sign/entry-default-signed.hap
$HDC shell aa start -b org.maplibre.native.demo -a EntryAbility
```
