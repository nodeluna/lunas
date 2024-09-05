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
                        version = "0.11.0";
                        src = prev.fetchurl {
                        url = "https://www.libssh.org/files/0.11/libssh-0.11.1.tar.xz";
                        sha256="0y8v5ihrqnjxchvjhz8fcczndchaaxxim64bqm8q3q4i5v3xrdql";
                        };
                        });
                };
                pkgs = import nixpkgs { inherit system; overlays = [ overlay ]; };

                in
                {
                defaultPackage = pkgs.stdenv.mkDerivation {
                pname = "lunas";
                version = "no-version";
                src = ./.;
                buildInputs = with pkgs; [ gcc libssh ];
                nativeBuildInputs = with pkgs; [ cmake gnumake ];
                buildPhase = ''
                    make -j
                    make install
                    '';
                };
                devShell = with pkgs; mkShell {
                    buildInputs = with pkgs; [ gcc libssh ];
                };
                }
    );
}

