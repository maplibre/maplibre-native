# MapLibre GL Native for Android - Docker Container for CI

This directory contains files to build the Docker container which is used to test and build MapLibre GL Native for Android on CI. [Nix](https://nixos.org/) is used which is a declarative language and package manager for reproducible builds.

```
nix build --extra-experimental-features nix-command --extra-experimental-features flakes
docker load < result
```

## Updating

All dependencies are pinned in the lock file. To update all inputs

```
nix flake update --commit-lock-file
```

Individual inputs can also be updated. See [nix flake update(https://nixos.org/manual/nix/stable/command-ref/new-cli/nix3-flake-update.html).


