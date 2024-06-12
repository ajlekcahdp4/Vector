{ pkgs, stdenv, ... }:
stdenv.mkDerivation {
  src = ../.;
  pname = "Vector";
  version = "1.0.0";
  nativeBuildInputs = with pkgs; [
    cmake
    gtest
  ];
  doCheck = true;
}
