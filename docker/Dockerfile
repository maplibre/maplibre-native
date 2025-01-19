FROM ubuntu:24.04

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
        python3 \
        python3-pip \
        python-is-python3 \
        clang-tidy \
    && : # end of the RUN cmd - easier to keep a colon at the end of the list, than to keep the backslashes in check

# This could also be `.../releases/latest/download/bazelisk-linux-amd64` for the latest version, but for predictability better hardcode it
# Detect if current CPU is x64 or ARM64 and download the appropriate binary
RUN echo "Download and install Bazel" \
    && if [ "$(uname -m)" = "aarch64" ]; then \
        curl -fsSL https://github.com/bazelbuild/bazelisk/releases/download/v1.20.0/bazelisk-linux-arm64 -o /usr/local/bin/bazel ;\
    else \
        curl -fsSL https://github.com/bazelbuild/bazelisk/releases/download/v1.20.0/bazelisk-linux-amd64 -o /usr/local/bin/bazel ;\
    fi \
    && chmod +x /usr/local/bin/bazel \
    && :

# This username is hardcoded in several places, and should simply match whatever base image uses
ARG USERNAME=ubuntu
ARG USER_UID=1000
ARG USER_GID=$USER_UID

# Create docker user wuth sudo rights as passed in by the build command
# This was modeled on https://code.visualstudio.com/remote/advancedcontainers/add-nonroot-user
# On a Mac, USER_GID might already exist, so ignore it if it fails (--force)
RUN groupmod --gid $USER_GID $USERNAME \
    && usermod --uid $USER_UID --gid $USER_GID $USERNAME \
    && chown -R $USER_UID:$USER_GID /home/$USERNAME \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

# This allows users to `docker run` without specifying -u and -g
USER $USERNAME

#RUN pip install pre-commit

ENV RUSTUP_HOME=/home/$USERNAME/.cache/.rustup \
    CARGO_HOME=/home/$USERNAME/.cache/.cargo \
    PATH=/home/$USERNAME/.cache/.cargo/bin:$PATH

# As the very last step, copy the startup script
USER root
COPY startup.sh /usr/local/bin/startup.sh
RUN chmod +x /usr/local/bin/startup.sh
USER $USERNAME

WORKDIR /app
ENTRYPOINT ["/usr/local/bin/startup.sh"]
CMD ["bash"]
