## this branch (module) ISN'T READY

this is an attempt to rewrite lunas using cpp modules

## lunas

A syncing cli tool that can handle more than two directories locally and remotely

## features

* multi-directory syncing
* async I/O in local and remote syncing for faster transfer
* capable of syncing between multiples of local to local, remote to local, local to remote, and remote to remote directories.
* can sync two-ways between directories (from A to B and from B to A), unless source and destination options are used.
* sync files that do not already exist in destination directories or/and to overwrite older or newer files, --update or --rollback can be used
* for convenient syncing of recurring operations, lunas has a simple-to-use config file where presets can be defined
* sync file attributes
* reliable --resume for interrupted transfers with mtime check mechanism to avoid resuming with src/dest mismatch. check the man page
* and more, check the man page to know about them

## usage (this branch (module) ISN'T READY)

* lunas -p dir1 -p dir2 -r user@ip:dir3     # -p, -r are for two-ways syncing "-p" for local dirs, "-r" for remote dirs
* lunas -s dir1 -rd user@ip:dir2            # -s local source, -rs remote src, -d local dest, -rd remote dest
* lunas -s dir1 -rd user@ip:dir2 -u         # -u/--update overwrite older files with newer ones based on their mtime
* lunas -s dir1 -rd user@ip:dir2 -rb        # -rb/--rollback overwrite newer files with older ones based on their mtime
* lunas -c preset                           # -c/--config preset with predefined options to be synced. check the man page 'CONFIG FILE' section
* read the man page and/or --help statement for more options

## file conflicts

* by default, lunas only syncs files that do not already exist in destination directories, to overwrite older or newer files, --update or --rollback can be used.
* if files with same relative path were found in different input dirs but with different file types, an error is printed and it won't be synced

## dependencies

* (clang >= 17) OR (clang >= 18.1.2 and libc++) for faster compliation

* lunas need libssh >= 0.11.* to build and run. the package may be named libssh, libssh-dev or libssh-devel on your distro

## build

* make

or

* cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_CXX_COMPILER=clang++
* ninja -C build

## remote syncing

thanks to libssh, lunas can sync remote directories with local or/and remote ones using sftp. Read the --help statement

## author

nodeluna - nodeluna@proton.me

## license
    This program is free software: you can redistribute it and/or modify it under the terms of
    the GNU General Public License as published by the Free Software Foundation version 3 of
    the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
    without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU General Public License for more details.

