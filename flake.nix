{
  description = "MapLibre Native development shell";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-26.05";
  };

  outputs =
    { nixpkgs, ... }:
    let
      systems = [
        "x86_64-linux"
        "aarch64-linux"
      ];
      forEachSystem = nixpkgs.lib.genAttrs systems;
    in
    {
      devShells = forEachSystem (
        system:
        let
          pkgs = import nixpkgs { inherit system; };
        in
        {
          default = pkgs.mkShell {
            packages = with pkgs; [
              ccache
              clang
              cmake
              curl
              git
              glfw
              icu
              libGL
              libGLU
              libjpeg
              libpng
              libuv
              libwebp
              libxkbcommon
              mesa
              ninja
              pkg-config
              wayland
              wayland-protocols
              xorg.libX11
              xorg.libXcursor
              xorg.libXdamage
              xorg.libXext
              xorg.libXfixes
              xorg.libXi
              xorg.libXinerama
              xorg.libXrandr
              xorg.libxcb
            ];

            CC = "clang";
            CXX = "clang++";
            CMAKE_C_COMPILER_LAUNCHER = "ccache";
            CMAKE_CXX_COMPILER_LAUNCHER = "ccache";

            shellHook = ''
              export CCACHE_DIR="''${CCACHE_DIR:-$PWD/.ccache}"
              export CMAKE_C_COMPILER_LAUNCHER=ccache
              export CMAKE_CXX_COMPILER_LAUNCHER=ccache
            '';
          };
        }
      );
    };
}
