# Monix

Demonstration for CSCI262 Assignment 3 - Group 1

## Dependencies

Ubuntu 24.04:

```sh
sudo apt install -y build-essential cmake libssl-dev
```

## Components

`core`: The core Monix operating system.
 - `core\commands`: Pre-installed Monix commands.

`modules`: Extension modules for the Monix OS.
 - `modules\hello`: A sample module which prints `"Hello World!"`.
 - `modules\horse`: A Trojan horse from a compromised trusted publisher. Maliciously patches the
 system and loads a backdoor.
 - `modules\tobira`: A backdoor module from an untrusted publisher. Allows remote attackers to
 access a Monix shell from port `6969`. The name is based on the word 扉, which menas "door".

## Building

- Install the dependencies listed above.
- Build the project using `cmake`. From the project root:

```sh
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=.
make install -j$(nproc)
```

## Running

The built binaries will be loaded in the `build/bin` directory.

From the project root:

```sh
build/bin/monix
```

To run in unguarded mode, use:

```sh
build/bin/monix -u
```

A log file (`monix.log`) will be created in the current directory.

```
less monix.log
```
