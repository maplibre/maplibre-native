{
  description = "MapLibre Native – development shells and build packages for Linux";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachSystem [ "x86_64-linux" "aarch64-linux" ] (system:
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
            hash = "sha256-0b6hDyH1rq8DuVJY9L3U3Lk3uEKMjPARzp5mCt5hfp8=";
          };
          cargoHash = "sha256-oTeV16+aZSbu6D5bmJgiU4DWYuD//5glL2mqRISH7yY=";
        };

        # ── Static library overrides for amalgamation builds ───────────
        # Nixpkgs doesn't ship .a files by default. Build static variants
        # with -fPIC for the amalgamation path.
        staticOverride = pkg: pkg.overrideAttrs (old: {
          dontDisableStatic = true;
          configureFlags = (old.configureFlags or []) ++ [ "--enable-static" ];
          cmakeFlags = (old.cmakeFlags or []) ++ [
            "-DBUILD_SHARED_LIBS=OFF"
            "-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
          ];
          mesonFlags = (old.mesonFlags or []) ++ [ "-Ddefault_library=both" ];
        });

        staticLibs = {
          png = pkgs.libpng.overrideAttrs (old: {
            dontDisableStatic = true;
            cmakeFlags = (old.cmakeFlags or []) ++ [
              "-DPNG_SHARED=OFF"
              "-DPNG_STATIC=ON"
              "-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
            ];
          });
          jpeg = pkgs.libjpeg.overrideAttrs (old: {
            dontDisableStatic = true;
            cmakeFlags = (old.cmakeFlags or []) ++ [
              "-DENABLE_STATIC=ON"
              "-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
            ];
          });
          webp = pkgs.libwebp.overrideAttrs (old: {
            dontDisableStatic = true;
            cmakeFlags = (old.cmakeFlags or []) ++ [
              "-DBUILD_SHARED_LIBS=OFF"
              "-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
            ];
          });
          uv = pkgs.libuv.overrideAttrs (old: {
            dontDisableStatic = true;
            cmakeFlags = (old.cmakeFlags or []) ++ [
              "-DLIBUV_BUILD_SHARED=OFF"
              "-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
            ];
          });
          icu = pkgs.icu.overrideAttrs (old: {
            dontDisableStatic = true;
            configureFlags = (old.configureFlags or []) ++ [ "--enable-static" ];
          });
          ssl = (pkgs.openssl.override { static = true; });
          z = pkgs.zlib.static;
          bz2 = pkgs.bzip2.overrideAttrs (old: {
            dontDisableStatic = true;
            # bzip2 in nixpkgs uses libtool; create a static .a from the objects
            postInstall = (old.postInstall or "") + ''
              if [ ! -f $out/lib/libbz2.a ]; then
                ar rcs $out/lib/libbz2.a .libs/*.o
              fi
            '';
          });
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
          openssl
        ];

        x11Inputs = with pkgs; [
          libX11
          libXinerama
          libXcursor
          libXi
          libxcb
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
        mkMbglCore = { preset, amalgamation ? false, extraCmakeFlags ? [] }:
          pkgs.stdenv.mkDerivation {
            pname = "mbgl-core-${preset}${if amalgamation then "-amalgamation" else ""}";
            version = self.shortRev or self.dirtyShortRev or "dev";
            src = self;

            nativeBuildInputs = commonNativeBuildInputs
              ++ (if amalgamation then [ armerge ] else []);
            buildInputs = commonBuildInputs
              ++ x11Inputs
              ++ (if builtins.match ".*vulkan.*" preset != null
                  then vulkanInputs
                  else openglInputs)
              ++ (if amalgamation then builtins.attrValues staticLibs else []);

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

          # Amalgamation shell: includes static libraries for find_static_library.
          amalgamation = pkgs.mkShell {
            nativeBuildInputs = commonNativeBuildInputs ++ [ armerge ];
            buildInputs = commonBuildInputs ++ x11Inputs ++ openglInputs
              ++ (builtins.attrValues staticLibs);

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
            amalgamation = true;
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
