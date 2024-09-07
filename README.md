## NOT READY!

## lunas

    A syncing cli tool that can handle more than two directories locally and remotely

## installation

    $ make
    # make install

## un-installation

    $ make clean
    # make uninstall

## installation with cmake

    $ cmake -B build
    $ cd build
    $ make
    # make install

## un-installation with cmake

    $ cd build
    # make uninstall
    $ cd ..
    $ rm -rf build

## installation with nix flakes

    $ nix profile install github:nodeluna/lunas

## dependencies

    lunas need libssh >= 0.11.* to build and run. the package may be named libssh, libssh-dev or libssh-devel on your distro

    lunas can be compiled without libssh, with local syncing only, as follows

    $ make local
    # make install

    remove the old 'build' directory first if exists

## usage

    read the man page and/or --help statement

## files

    when running "$ make" compiles the program and the binary is placed in the repo's main directory and build files in ./build
    when running "# make install" the binary file is copied to /usr/bin/lunas , and the man page to /usr/share/man/man1/lunas.1

    when running "$ make clean" the ./build is removed. And the binary is removed from the repo's directory but not from /usr/bin/lunas
    when running "# make uninstall" the binary file is removed from /usr/bin/lunas as well as /usr/share/man/man1/lunas.1

## remote syncing

    thanks to libssh, lunas can sync remote directories with local or/and remote ones using sftp. Read the --help statement

## author

    nodeluna - nodeluna@proton.me
