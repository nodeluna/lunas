{
    description = "A syncing cli tool that can handle more than two directories locally and remotely";

    inputs = {
        nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
        utils.url = "github:numtide/flake-utils";
    };

    outputs = { self, nixpkgs, utils}:
        utils.lib.eachDefaultSystem (system:
                let
                pkgs = import nixpkgs { inherit system; };
                pname = "lunas";
                version = "2.0.7";
                in
                {

                packages.lunas = pkgs.stdenv.mkDerivation {
                    inherit pname;
                    inherit version;
                    src = ./.;
                    buildInputs = with pkgs; [ gcc pkg-config libssh ];
                    nativeBuildInputs = with pkgs; [ gnumake cmake ];
                    buildPhase = ''
                        make -j8
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
                        make local -j8
                        make install
                        '';
                };
                packages.default = self.packages.${system}.lunas;

                devShells.lunas = with pkgs; mkShell {
                    buildInputs = with pkgs; [ gcc pkg-config libssh go ];
                };
                devShells.lunas-local = with pkgs; mkShell {
                    buildInputs = with pkgs; [ gcc pkg-config go ];
                };
                devShells.default = self.devShells.${system}.lunas;
                }
    );
}

