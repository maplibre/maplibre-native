# Building with Docker

These steps will allow you to compile code as described [here](./README.md) using a Docker container. All the steps should be executed from the root of the repository.

> [!IMPORTANT]
> Not all platform builds are currently supported. Docker builds are a work in progress.

> [!IMPORTANT]
> You cannot build MapLibre native using both Docker and host methods at the same time. If you want to switch, you need to clean the repository first, e.g. by using this command:
>
> ```bash
> git clean -dxfi -e .idea -e .clwb -e .vscode
> ```

### Build Docker Image

You must build your own Docker image, specific with your user and group IDs to ensure file permissions stay correct.

```bash
# Build docker image from the repo __root__
# Specifying USER_UID and USER_GID allows container to create files with the same owner as the host user,
# and avoids having to pass -u $(id -u):$(id -g) to docker run.
docker build \
  -t maplibre-native-image \
  --build-arg USER_UID=$(id -u) \
  --build-arg USER_GID=$(id -g) \
  -f docker/Dockerfile \
  docker
```

## Run Docker Container

```bash
# Run all build commands using the docker container.
# You can also execute build commands from inside the docker container by starting it without the build command.
docker run --rm -it -v "$PWD:/app/" -v "$PWD/docker/.cache:/home/ubuntu/.cache" maplibre-native-image
```

You can also use the container to run just one specific commands, e.g. `cmake` or `bazel`. Any downloaded dependencies will be cached in the `docker/.cache` directory.

```bash
docker run --rm -it -v "$PWD:/app/" -v "$PWD/docker/.cache:/home/ubuntu/.cache" maplibre-native-image cmake ...
```
