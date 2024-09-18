{
    description = "A syncing cli tool that can handle more than two directories locally and remotely";

    inputs = {
        nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
        utils.url = "github:numtide/flake-utils";
    };

    outputs = { self, nixpkgs, utils}:
        utils.lib.eachDefaultSystem (system:
                let
                overlay = final: prev: {
                libssh = prev.libssh.overrideAttrs (oldAttrs: rec {
                        version = "0.11.1";
                        src = prev.fetchurl {
                        url = "https://www.libssh.org/files/0.11/libssh-0.11.1.tar.xz";
                        sha256="0y8v5ihrqnjxchvjhz8fcczndchaaxxim64bqm8q3q4i5v3xrdql";
                        };
                        });
                };
                pkgs = import nixpkgs { inherit system; overlays = [ overlay ]; };
                pname = "lunas";
                version = "2.0.3";
                in
                {

                packages.lunas = pkgs.stdenv.mkDerivation {
                    inherit pname;
                    inherit version;
                    src = ./.;
                    buildInputs = with pkgs; [ gcc pkg-config libssh ];
                    nativeBuildInputs = with pkgs; [ gnumake cmake ];
                    buildPhase = ''
                        make -j
                        make install
                        '';
                };

                packages.lunas-local = pkgs.stdenv.mkDerivation {
                    inherit pname;
                    version = "local-" + version;
                    src = ./.;
                    buildInputs = with pkgs; [ gcc pkg-config ];
                    nativeBuildInputs = with pkgs; [ gnumake cmake ];
                    buildPhase = ''
                        make local -j
                        make install
                        '';
                };
                packages.default = self.packages.${system}.lunas;

                devShells.lunas = with pkgs; mkShell {
                    buildInputs = with pkgs; [ gcc pkg-config libssh ];
                };
                devShells.lunas-local = with pkgs; mkShell {
                    buildInputs = with pkgs; [ gcc pkg-config ];
                };
                devShells.default = self.devShells.${system}.lunas;
                }
    );
}

