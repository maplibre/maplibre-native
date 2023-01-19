# MapLibre GL Native for Android - CI Environment

This directory contains files to set up the CI environment which is used to test and build MapLibre GL Native for Android on CI. [Nix](https://nixos.org/) is used which is a declarative language and package manager for reproducible builds. Note however that Nix is not used for the build itself.

Nix Flakes are used, which is an experimental feature which needs to be enabled in your settings or via flags passed to the commands below. See [Nix Flakes - Enable Flakes](https://nixos.wiki/wiki/Flakes#Enable_flakes).

## Shell

To enter a shell with the CI environment, use:

```
nix develop .#shell
```

## Docker container

To create a Docker image which contains the CI environment:

```
nix build .#dockerImage
docker load < result
```

## Updating

All dependencies are pinned in the lock file. To update all inputs

```
nix flake update --commit-lock-file
```

Individual inputs can also be updated. See [`nix flake update`](https://nixos.org/manual/nix/stable/command-ref/new-cli/nix3-flake-update.html).


