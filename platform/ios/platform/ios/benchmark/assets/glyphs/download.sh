#!/usr/bin/env bash

set -u

LIST=(
    "Noto%20Sans%20Regular/0-255.pbf"
    "Roboto%20Condensed%20Italic,Noto%20Sans%20Italic/0-255.pbf"
    "Roboto%20Condensed%20Italic,Noto%20Sans%20Italic/256-511.pbf"
    "Roboto%20Medium,Noto%20Sans%20Regular/0-255.pbf"
    "Roboto%20Medium,Noto%20Sans%20Regular/65024-65279.pbf"
    "Roboto%20Medium,Noto%20Sans%20Regular/11520-11775.pbf"
    "Roboto%20Regular,Noto%20Sans%20Regular/0-255.pbf"
    "Roboto%20Regular,Noto%20Sans%20Regular/256-511.pbf"
    "Roboto%20Regular,Noto%20Sans%20Regular/512-767.pbf"
    "Roboto%20Regular,Noto%20Sans%20Regular/768-1023.pbf"
    "Roboto%20Regular,Noto%20Sans%20Regular/1024-1279.pbf"
    "Roboto%20Regular,Noto%20Sans%20Regular/8192-8447.pbf"
    "Roboto%20Medium,Noto%20Sans%20Regular/256-511.pbf"
    "Roboto%20Medium,Noto%20Sans%20Regular/512-767.pbf"
    "Roboto%20Medium,Noto%20Sans%20Regular/768-1023.pbf"
    "Roboto%20Medium,Noto%20Sans%20Regular/1024-1279.pbf"
)


# from https://gist.github.com/cdown/1163649
urldecode() {
    local url_encoded="${1//+/ }"
    printf '%b' "${url_encoded//%/\\x}"
}

for i in ${LIST[@]} ; do
    OUTPUT=`urldecode "$i"`
    if [ ! -f "${OUTPUT}" ] ; then
        mkdir -p "`dirname "${OUTPUT}"`"
        echo "Downloading glyph '${OUTPUT}'"
        curl --compressed --progress-bar --output "${OUTPUT}" "https://api.maptiler.com/fonts/${i}?key=${MLN_API_KEY}"
    fi
done
