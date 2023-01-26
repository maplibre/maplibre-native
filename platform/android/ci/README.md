# MapLibre GL Native for Android - CI Environment

This directory contains files to set up the CI environment which is used to test and build MapLibre GL Native for Android on CI. [Nix](https://nixos.org/) is used which is a declarative language and package manager for reproducible builds. Note however that Nix is not used for the build itself.

You can also re-create the CI environment locally.

First [install Nix](https://nixos.org/download.html); Linux, macOS and Windows ([WSL2](https://learn.microsoft.com/en-us/windows/wsl/install) are supported. If you want to learn how Nix works, a good introduction is [Zero to Nix](https://zero-to-nix.com/).

## Nix Flake

A *Nix flake* is used to set up the environment, which is a description of dependencies pinned to a specific version (see `flake.nix` and `flake.lock` in this directory). Flakes are an an experimental feature of Nix which needs to be enabled in your settings or via flags passed to the commands below.  You need to add the flags `--extra-experimental-features nix-command --extra-experimental-features flakes` to the commands below or add:

```
experimental-features = nix-command flakes
```

to your `~/.config/nix/nix.conf`.

See also [Nix Flakes - Enable Flakes](https://nixos.wiki/wiki/Flakes#Enable_flakes).


## Shell

To enter a shell with the CI environment, use (in this directory):

```
nix develop
```

## Docker container

To create a Docker image which contains the CI environment:

```
nix build .#dockerImage
docker load < result
```

Please create an issue if you are interested in having this container made available without having to create it yourself.

## Updating

All dependencies are pinned in the lock file. To update all inputs

```
nix flake update --commit-lock-file
```

Individual inputs can also be updated. See [`nix flake update`](https://nixos.org/manual/nix/stable/command-ref/new-cli/nix3-flake-update.html).


