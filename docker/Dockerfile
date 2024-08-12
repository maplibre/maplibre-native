FROM ubuntu:22.04

# Install build tools and dependencies
RUN    apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y \
        apt-transport-https \
        curl \
        gnupg \
        sudo `# allows dev to install more packages without switching to root or rebuilding container` \
        build-essential \
        libsqlite3-dev \
        libcurl4-openssl-dev \
        libglfw3-dev \
        libuv1-dev \
        libpng-dev \
        libicu-dev \
        libjpeg-turbo8-dev \
        libwebp-dev \
        xvfb \
        clang \
        git \
        cmake \
        ccache \
        ninja-build \
        pkg-config \
        clang-tidy \
    && : # end of the RUN cmd - easier to keep a colon at the end of the list, than to keep the backslashes in check

# This could also be `.../releases/latest/download/bazelisk-linux-amd64` for the latest version, but for predictability better hardcode it
# Detect if current CPU is x64 or ARM64 and download the appropriate binary
RUN if [ "$(uname -m)" = "aarch64" ]; then \
        curl -fsSL https://github.com/bazelbuild/bazelisk/releases/download/v1.20.0/bazelisk-linux-arm64 -o /usr/local/bin/bazel ;\
    else \
        curl -fsSL https://github.com/bazelbuild/bazelisk/releases/download/v1.20.0/bazelisk-linux-amd64 -o /usr/local/bin/bazel ;\
    fi \
    && chmod +x /usr/local/bin/bazel \
    && :

WORKDIR /app

ARG USERNAME=user
ARG USER_UID=1000
ARG USER_GID=$USER_UID

# Create docker user wuth sudo rights as passed in by the build command
# This was modeled on https://code.visualstudio.com/remote/advancedcontainers/add-nonroot-user
# On a Mac, USER_GID might already exist, so ignore it if it fails (--force)
RUN groupadd --force --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

COPY startup.sh /usr/local/bin/startup.sh
RUN chmod +x /usr/local/bin/startup.sh

# This allows users to `docker run` without specifying -u and -g
USER $USERNAME

ENTRYPOINT ["/usr/local/bin/startup.sh"]
CMD ["bash"]
