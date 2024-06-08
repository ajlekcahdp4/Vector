{ pkgs, stdenv, ... }:
stdenv.mkDerivation {
  src = ../.;
  pname = "Vector";
  version = "1.0.0";
  nativeBuildInputs = with pkgs; [
    cmake
    gtest
  ];
  installPhase = ''
    mkdir -p $out/include
    install -Dm=rw-r-r $src/lib/include/* $out/include/
  '';
  doCheck = true;
}
