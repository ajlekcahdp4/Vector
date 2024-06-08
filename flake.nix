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
    let
      overlayVector = import ./nix/overlays/vector.nix;
    in
    flake-parts.lib.mkFlake { inherit inputs; } {
      imports = [ treefmt-nix.flakeModule ];

      systems = [ "x86_64-linux" ];

      flake.overlays.default = overlayVector;
      perSystem =
        { pkgs, system, ... }:
        {
          _module.args.pkgs = import inputs.nixpkgs {
            inherit system;
            overlays = [ overlayVector ];
          };
          imports = [ ./nix/treefmt.nix ];
          packages = {
            default = pkgs.Vector;
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
