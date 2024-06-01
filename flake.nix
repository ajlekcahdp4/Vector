{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
    treefmt-nix = {
      url = "github:numtide/treefmt-nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };
  outputs =
    { flake-parts, treefmt-nix, ... }@inputs:
    flake-parts.lib.mkFlake { inherit inputs; } {
      imports = [ treefmt-nix.flakeModule ];
      systems = [ "x86_64-linux" ];

      perSystem =
        { pkgs, lib, ... }:
        {
          imports = [ ./nix/treefmt.nix ];
          packages = {
            default = pkgs.stdenv.mkDerivation {
              src = ./.;
              pname = "Vector";
              version = "1.0.0";
              nativeBuildInputs = with pkgs; [
                cmake
                gdb
                valgrind
                gtest
              ];
              buildInputs = [ ];
              doInstallCheck = true;
              installPhase = ''
                mkdir -p $out/include
                cp -r $src/lib/include/* $out/include
              '';
              installCheckPhase = ''
                ctest --test-dir=build
              '';
            };
          };
          devShells.default = pkgs.mkShell {
            nativeBuildInputs = with pkgs; [
              cmake
              gdb
              valgrind
              gtest
            ];
          };
        };
    };
}
