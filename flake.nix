{
  description = "MapLibre Native – development shells and build packages for Linux";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.11";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };

        # ── armerge (not in nixpkgs) ────────────────────────────────────
        armerge = pkgs.rustPlatform.buildRustPackage rec {
          pname = "armerge";
          version = "2.2.2";
          src = pkgs.fetchFromGitHub {
            owner = "tux3";
            repo = "armerge";
            rev = "v${version}";
            hash = "sha256-AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=";  # TODO: replace after first `nix build`
          };
          cargoHash = "sha256-AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=";  # TODO: replace after first `nix build`
        };

        # ── Common build inputs shared across all configurations ────────
        commonNativeBuildInputs = with pkgs; [
          cmake
          ninja
          pkg-config
          clang
          ccache
        ];

        commonBuildInputs = with pkgs; [
          curl
          libjpeg
          libpng
          libwebp
          libuv
          zlib
          bzip2
          icu
          glfw
        ];

        x11Inputs = with pkgs; [
          xorg.libX11
          xorg.libXinerama
          xorg.libXcursor
          xorg.libXi
          xorg.libxcb
        ];

        openglInputs = with pkgs; [
          libGL
          libGLU
        ];

        vulkanInputs = with pkgs; [
          vulkan-headers
          vulkan-loader
          glslang
          spirv-tools
        ];

        waylandInputs = with pkgs; [
          wayland
          wayland-protocols
          libxkbcommon
        ];

        # ── Helper: invoke cmake + ninja for a given preset ─────────────
        mkMbglCore = { preset, extraCmakeFlags ? [] }:
          pkgs.stdenv.mkDerivation {
            pname = "mbgl-core-${preset}";
            version = self.shortRev or self.dirtyShortRev or "dev";
            src = self;

            nativeBuildInputs = commonNativeBuildInputs;
            buildInputs = commonBuildInputs
              ++ x11Inputs
              ++ (if builtins.match ".*vulkan.*" preset != null
                  then vulkanInputs
                  else openglInputs);

            cmakeFlags = [
              "-G" "Ninja"
              "-DCMAKE_BUILD_TYPE=RelWithDebInfo"
            ] ++ extraCmakeFlags;

            # The repo's presets set compilers and dirs; override only what
            # Nix needs (paths are already handled by buildInputs).
            buildPhase = ''
              cmake --preset ${preset} \
                -DCMAKE_CXX_COMPILER_LAUNCHER="" \
                $cmakeFlags
              cmake --build build-${preset} --target mbgl-core
            '';

            installPhase = ''
              mkdir -p $out/lib
              cp build-${preset}/libmbgl-core.a $out/lib/
              if [ -f build-${preset}/libmbgl-core-amalgam.a ]; then
                cp build-${preset}/libmbgl-core-amalgam.a $out/lib/
              fi
            '';
          };

      in {
        # ── Dev shells ──────────────────────────────────────────────────
        devShells = {
          # Kitchen-sink shell: everything you need for any Linux build.
          default = pkgs.mkShell {
            nativeBuildInputs = commonNativeBuildInputs ++ [ armerge ];
            buildInputs = commonBuildInputs
              ++ x11Inputs
              ++ openglInputs
              ++ vulkanInputs
              ++ waylandInputs;

            shellHook = ''
              echo "maplibre-native dev shell ready"
              echo "  cmake --preset linux-opengl && cmake --build build-linux-opengl"
            '';
          };

          # Minimal shell for amalgamation builds only.
          amalgamation = pkgs.mkShell {
            nativeBuildInputs = commonNativeBuildInputs ++ [ armerge ];
            buildInputs = commonBuildInputs ++ x11Inputs ++ openglInputs;

            shellHook = ''
              echo "amalgamation shell – use -DMLN_CREATE_AMALGAMATION=ON"
            '';
          };
        };

        # ── Packages (build mbgl-core under Nix) ───────────────────────
        packages = {
          mbgl-core-opengl = mkMbglCore {
            preset = "linux-opengl";
          };

          mbgl-core-vulkan = mkMbglCore {
            preset = "linux-vulkan";
          };

          mbgl-core-opengl-amalgamation = mkMbglCore {
            preset = "linux-opengl";
            extraCmakeFlags = [ "-DMLN_CREATE_AMALGAMATION=ON" ];
          };

          default = self.packages.${system}.mbgl-core-opengl;
        };

        # ── Checks (CI-friendly build validation) ───────────────────────
        checks = {
          build-opengl = self.packages.${system}.mbgl-core-opengl;
          build-vulkan = self.packages.${system}.mbgl-core-vulkan;
          build-amalgamation = self.packages.${system}.mbgl-core-opengl-amalgamation;
        };
      });
}
