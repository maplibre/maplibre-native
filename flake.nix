{
  description = "Dev shell with CMake and Clang";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs =
    { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        packages = with pkgs; [
          cmake
          clang
          curl
          pkg-config
          libjpeg
          libpng
          libwebp
          libuv
          icu
          mesa
          libGL
          glfw
          ccache
        ];

        shellHook = ''
          echo "Entering dev shell with CMake and Clang"
        '';
      };
    };
}
