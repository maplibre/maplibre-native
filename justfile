#!/usr/bin/env just --justfile

# Always require bash. If we allow it to be default, it may work incorrectly on Windows
set shell := ["bash", "-c"]

@_default:
    {{just_executable()}} --list

# interactively clean-up git repository, keeping IDE files
git-clean:
    git clean -dxfi -e .idea -e .clwb -e .vscode

# (re-)build `maplibre-native-image` docker image for the current user
init-docker:
    docker build -t maplibre-native-image --build-arg USER_UID=$(id -u) --build-arg USER_GID=$(id -g) -f docker/Dockerfile docker

# run command with docker, e.g. `just docker bazel build //:mbgl-core`, or open docker shell with `just docker`
docker *ARGS:
    docker run --rm -it -v "$PWD:/app/" -v "$PWD/docker/.cache:/home/user/.cache" maplibre-native-image {{ARGS}}
