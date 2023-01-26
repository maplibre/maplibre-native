{
  description = "MapLibre GL Native for Android CI Docker Container";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
    android.url = "github:tadfisher/android-nixpkgs";
  };

  outputs = { self, nixpkgs, flake-utils, android }:
    {
      overlay = final: prev: {
        inherit (self.packages.${final.system}) android-sdk android-studio;
      };
    }
    //
    flake-utils.lib.eachSystem [ "x86_64-linux" ] (system:
      let
        pkgs = import nixpkgs {
          inherit system;
          config.allowUnfree = true;
          overlays = [
            self.overlay
          ];
        };
        env = let androidHome = "${android-sdk}/share/android-sdk"; in
          {
            JAVA_HOME = jdk.home;
            ANDROID_HOME = androidHome;
            ANDROID_SDK_ROOT = androidHome;
            GRADLE_OPTS = "-Dorg.gradle.project.android.aapt2FromMavenOverride=${android-sdk}/share/android-sdk/build-tools/31.0.0/aapt2 -Dorg.gradle.daemon=false";
          };
        jdk = pkgs.jdk;
        android-sdk = android.sdk.${system} (sdkPkgs: with sdkPkgs; [
          # Useful packages for building and testing.
          build-tools-31-0-0
          cmdline-tools-latest
          # emulator
          platform-tools
          platforms-android-33
          ndk-25-1-8937393

          # Other useful packages for a development environment.
          # sources-android-30
          # system-images-android-30-google-apis-x86
          # system-images-android-30-google-apis-playstore-x86
        ]);
        paths = with pkgs; [
          cmake
          ninja
          bash
          nodejs
          gnumake
          coreutils
          gnugrep
          gnused
          git
          gradle
          google-cloud-sdk
          ccache
          /*   pkgs.androidStudioPackages.stable; */
          /*   pkgs.androidStudioPackages.beta; */
          /*   pkgs.androidStudioPackages.preview; */
          /*   pkgs.androidStudioPackage.canary; */
        ];
        dockerImage = pkgs.dockerTools.buildLayeredImage {
          name = "maplibre-gl-native-android-builder";
          created = "now";
          extraCommands = ''mkdir tmp
					chmod 1777 tmp'';
          config = {
            Env = map (name: "${name}=${env.${name}}") (builtins.attrNames env);
          };
          contents = paths ++ [ pkgs.dockerTools.usrBinEnv ];
        };
      in
      {
        devShells = {
          default = pkgs.mkShell {
            inherit env;
            packages = paths;
          };
        };
        packages = {
          dockerImage = dockerImage;
        };
      }
    );
}

