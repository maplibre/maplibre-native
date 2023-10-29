#!/usr/bin/env bash

set -u

(cd glyphs && ./download.sh)
(cd tiles && ./download.sh)

