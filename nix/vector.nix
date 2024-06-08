{ pkgs, stdenv, ... }:
stdenv.mkDerivation {
  src = ../.;
  pname = "Vector";
  version = "1.0.0";
  nativeBuildInputs = with pkgs; [
    cmake
    gdb
    valgrind
    gtest
  ];
  # header-only
  dontConfigure = true;
  dontBuild = true;
  dontFixup = true;
  installPhase = ''
    mkdir -p $out/include
    runHook preInstall
    install -Dm=rw-r-r $src/lib/include/* $out/include/
    runHook postInstall
  '';
  checkPhase = ''
    ctest --test-dir=build
  '';
}
