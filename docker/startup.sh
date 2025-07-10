#!/bin/sh

if [ ! -d /app/.github ] || [ ! -d ~/.cache ]; then
    echo " "
    echo "ERROR: Docker container was not started properly."
    echo "       From the root of this repo, run the following command."
    echo "       You may add any command to perform in the container at the end of this command."
    echo " "
    # shellcheck disable=SC2016
    echo '  docker run --rm -it -v "$PWD:/app/" -v "$PWD/docker/.cache:'"$HOME"'/.cache" maplibre-native-image'
    exit 1
fi

export PATH="$PATH:~/.local/bin/"


# Work in progress:  install and configure Swift and pre-commit
# Detect if current CPU is x64 or ARM64 and download the appropriate binary
#RUN echo "Download and install SWIFT" \
#    && if [ "$(uname -m)" = "aarch64" ]; then \
#        curl -fsSL https://download.swift.org/swift-5.10.1-release/ubuntu2204-aarch64/swift-5.10.1-RELEASE/swift-5.10.1-RELEASE-ubuntu22.04-aarch64.tar.gz \
#                -o /tmp/swift.tar.gz ;\
#    else \
#        curl -fsSL https://download.swift.org/swift-5.10.1-release/ubuntu2204/swift-5.10.1-RELEASE/swift-5.10.1-RELEASE-ubuntu22.04.tar.gz \
#                -o /tmp/swift.tar.gz ;\
#    fi \
#    && tar -xzf /tmp/swift.tar.gz -C / --strip-components=1 \
#    && rm /tmp/swift.tar.gz \
#    && :
#if [ ! -f "/app/.git/hooks/pre-commit" ]; then
#    echo "Configuring pre-commit git hooks by creating a .git/hooks/pre-commit file..."
#    ~/.local/bin/pre-commit install
#fi



if [ ! -f "$CARGO_HOME/env" ]; then
    echo "Downloading and installing Rust..."
    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y --profile minimal
fi
. "$CARGO_HOME/env"



if ! command -v cxxbridge > /dev/null; then
    echo "Installing cxxbridge..."
    cargo install cxxbridge-cmd@1.0.157 --locked
fi



exec "$@"
