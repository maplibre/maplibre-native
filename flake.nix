{
  description = "Development environments for MapLibre Native";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs =
    { nixpkgs, ... }:
    let
      inherit (nixpkgs) lib;

      forAllSystems =
        f:
        lib.genAttrs [ "x86_64-linux" "aarch64-linux" ] (
          system:
          f (
            import nixpkgs {
              inherit system;
              # The Android SDK is unfree and comes with its own license.
              config = {
                allowUnfree = true;
                android_sdk.accept_license = true;
              };
            }
          )
        );
    in
    {
      devShells = forAllSystems (
        pkgs:
        let
          # Same version as the clang-format hook in .pre-commit-config.yaml.
          llvm = pkgs.llvmPackages_20;

          linux = pkgs.mkShell.override { inherit (llvm) stdenv; } {
            packages = with pkgs; [
              cmake
              ninja
              ccache
              pkg-config
              llvm.clang-tools
              nodejs_24
              vulkan-tools
            ];

            buildInputs = with pkgs; [
              curl
              icu
              libjpeg
              libpng
              libuv
              libwebp
              zlib
              glfw
              libGL
              libx11
            ];

            # Loaded with dlopen() at runtime.
            LD_LIBRARY_PATH = lib.makeLibraryPath [ pkgs.vulkan-loader ];
            VK_ADD_LAYER_PATH = "${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d";
          };
        in
        {
          default = linux;
          inherit linux;
        }
        // lib.optionalAttrs (pkgs.stdenv.hostPlatform.system == "x86_64-linux") {
          android =
            let
              ndkVersion = lib.head (
                builtins.match ''.*ndkVersion = "([^"]+)".*'' (
                  builtins.readFile ./platform/android/buildSrc/src/main/kotlin/Versions.kt
                )
              );
              buildToolsVersion = "36.0.0";
              androidComposition = pkgs.androidenv.composeAndroidPackages {
                platformVersions = [
                  "34"
                  "35"
                  "36"
                ];
                buildToolsVersions = [
                  "35.0.0"
                  buildToolsVersion
                ];
                cmakeVersions = [ "3.31.6" ];
                includeNDK = true;
                ndkVersions = [ ndkVersion ];
              };
              androidSdk = "${androidComposition.androidsdk}/libexec/android-sdk";
            in
            pkgs.mkShell {
              packages = with pkgs; [
                androidComposition.androidsdk
                jdk17
                nodejs_24
                llvm.clang-tools
              ];

              ANDROID_HOME = androidSdk;
              ANDROID_SDK_ROOT = androidSdk;
              ANDROID_NDK_ROOT = "${androidSdk}/ndk/${ndkVersion}";
              JAVA_HOME = pkgs.jdk17.home;
              # The aapt2 that Gradle downloads from Maven does not run on NixOS.
              GRADLE_OPTS = "-Dorg.gradle.project.android.aapt2FromMavenOverride=${androidSdk}/build-tools/${buildToolsVersion}/aapt2";
            };
        }
      );

      formatter = forAllSystems (pkgs: pkgs.nixfmt);
    };
}
