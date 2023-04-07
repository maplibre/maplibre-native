#!/usr/bin/env bash

set -u

LIST=(
    # paris
    "tiles/v3/11/1037/704.pbf"
    "tiles/v3/11/1037/705.pbf"
    "tiles/v3/11/1038/704.pbf"
    "tiles/v3/11/1038/705.pbf"
    "tiles/v3/11/1036/704.pbf"
    "tiles/v3/11/1037/703.pbf"
    "tiles/v3/11/1036/705.pbf"
    "tiles/v3/11/1038/703.pbf"
    "tiles/v3/11/1036/703.pbf"

    # paris2
    "tiles/v3/13/4150/2819.pbf"
    "tiles/v3/13/4149/2819.pbf"
    "tiles/v3/13/4150/2818.pbf"
    "tiles/v3/13/4148/2819.pbf"
    "tiles/v3/13/4149/2818.pbf"
    "tiles/v3/13/4148/2818.pbf"
    "tiles/v3/13/4150/2820.pbf"
    "tiles/v3/13/4149/2820.pbf"
    "tiles/v3/13/4149/2817.pbf"
    "tiles/v3/13/4148/2817.pbf"

    # alps
    "tiles/v3/6/34/23.pbf"
    "tiles/v3/6/34/22.pbf"
    "tiles/v3/6/33/23.pbf"
    "tiles/v3/6/33/22.pbf"
    "tiles/v3/6/34/21.pbf"
    "tiles/v3/6/32/23.pbf"
    "tiles/v3/6/32/22.pbf"
    "tiles/v3/6/33/21.pbf"
    "tiles/v3/6/32/21.pbf"

    # us east
    "tiles/v3/5/9/12.pbf"
    "tiles/v3/5/8/12.pbf"
    "tiles/v3/5/9/13.pbf"
    "tiles/v3/5/8/13.pbf"
    "tiles/v3/5/9/11.pbf"
    "tiles/v3/5/7/12.pbf"
    "tiles/v3/5/8/11.pbf"
    "tiles/v3/5/7/13.pbf"
    "tiles/v3/5/7/11.pbf"

    # greater la
    "tiles/v3/9/88/204.pbf"
    "tiles/v3/9/88/205.pbf"
    "tiles/v3/9/89/204.pbf"
    "tiles/v3/9/89/205.pbf"
    "tiles/v3/9/87/204.pbf"
    "tiles/v3/9/88/203.pbf"
    "tiles/v3/9/87/205.pbf"
    "tiles/v3/9/89/203.pbf"
    "tiles/v3/9/87/203.pbf"

    # sf
    "tiles/v3/14/2621/6333.pbf"
    "tiles/v3/14/2620/6333.pbf"
    "tiles/v3/14/2621/6334.pbf"
    "tiles/v3/14/2620/6334.pbf"
    "tiles/v3/14/2621/6332.pbf"
    "tiles/v3/14/2619/6333.pbf"
    "tiles/v3/14/2620/6332.pbf"
    "tiles/v3/14/2619/6334.pbf"
    "tiles/v3/14/2619/6332.pbf"

    # oakland
    "tiles/v3/12/657/1582.pbf"
    "tiles/v3/12/657/1583.pbf"
    "tiles/v3/12/658/1582.pbf"
    "tiles/v3/12/658/1583.pbf"
    "tiles/v3/12/656/1582.pbf"
    "tiles/v3/12/657/1581.pbf"
    "tiles/v3/12/656/1583.pbf"
    "tiles/v3/12/658/1581.pbf"
    "tiles/v3/12/656/1581.pbf"

    # germany
    "tiles/v3/6/34/20.pbf"
    "tiles/v3/6/33/20.pbf"
    "tiles/v3/6/32/20.pbf"
)

for OUTPUT in ${LIST[@]} ; do
    if [ ! -f "${OUTPUT}" ] ; then
        mkdir -p "`dirname "${OUTPUT}"`"
        echo "Downloading tile '${OUTPUT}'"
        echo "https://api.maptiler.com/${OUTPUT}?key=${MLN_API_KEY}"
        curl -H "Accept-Encoding: gzip" -# "https://api.maptiler.com/${OUTPUT}?key=${MLN_API_KEY}" | gunzip > "${OUTPUT}"
    fi
done
