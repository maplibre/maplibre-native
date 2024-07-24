#!/bin/sh

if [ ! -d /app/.github ] || [ ! -d /home/user/.cache ]; then
    echo " "
    echo "ERROR: Docker container was not started properly."
    echo "       From the root of this repo, run the following command."
    echo "       You may add any command to perform in the container at the end of this command."
    echo " "
    echo '  docker run --rm -it -v "$PWD:/app/" -v "$PWD/docker/.cache:/home/user/.cache" maplibre-native-image'
    exit 1
fi

exec "$@"
