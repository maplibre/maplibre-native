#!/usr/bin/env bash

set -e
set -o pipefail
set -u

cat >foo <<EOF
     %echo Generating a basic OpenPGP key
     Key-Type: RSA
     Key-Length: 4096
     Subkey-Type: RSA
     Subkey-Length: 4096
     Name-Real: Petr Pokorny
     Name-Comment: MapLibre publish
     Name-Email: petr.pokorny@maptiler.com
     Expire-Date: 0
     Passphrase: < Put your password here >
     # Do a commit here, so that we can later print "done" :-)
     %commit
     %echo done
EOF

gpg --batch --generate-key foo
rm foo
gpg --list-secret-keys

# gpg --keyserver hkp://ipv4.pool.sks-keyservers.net --send-keys < your key here - i.e. 6BCC07D1, last 8 characters >
# gpg --keyserver hkps://keys.openpgp.org --send-key 6BCC07D1
# gpg --export-secret-keys < your key here - i.e. 6BCC07D1, last 8 characters > /Volumes/Src/src/MapTiler/Native/maplibre-gl-native/platform/android/signing-key.gpg
# base64 /Volumes/Src/src/MapTiler/Native/maplibre-gl-native/platform/android/signing-key.gpg > /Volumes/Src/src/MapTiler/Native/maplibre-gl-native/platform/android/signing-key.text
