# Self-Signed Provisioning Profile

This directory contains a self-signed provisioning profile.

```
security import MapLibre.cer
security cms -S -N 'iOS Team Provisioning Profile: *' -i MapLibre.plist -o MapLibre.mobileprovision
```