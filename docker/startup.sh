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


if [ ! -f "$CARGO_HOME/env" ]; then
    echo "Downloading and installing Rust..."
    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y --profile minimal
fi
. "$CARGO_HOME/env"



if ! command -v cxxbridge > /dev/null; then
    echo "Installing cxxbridge..."
    cargo install cxxbridge-cmd
fi


exec "$@"
