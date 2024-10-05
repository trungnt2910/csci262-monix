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

## Important Files

These files are important for demonstrating the security vulnerability and the defense.

- `modules/horse/horse.cpp`: Contains code for a Trojan horse that patches the core system in
unauthorized ways.
- `core/AsyncPatchProtection.cpp`: Contains code for the demonstrated defense.

## Technical Details

`monix` is a Linux executable that exports some symbols. Modules (Linux shared object (`.so`) files)
can be loaded into a `monix` process and access the exported symbols to provide functionality.
These modules are loaded through `dlopen`. `dlsym` is used to fetch a structure containing essential
module information and a pointer to an initialization function.

Not all `monix` symbols are exported. Many are hidden (`MX_HIDDEN_FROM_MODULES`) and only accessible
to files in the core.

Some critical data blobs are both hidden and protected using page protections. For simplicity of the
exploit, `monix` puts these data together in a list of protected regions with a visible magic
header. The `horse` module scans for this exact magic and makes its malicious patches on the found
region. This usually is not the case for real operating systems, where rootkits often rely on debug
symbols to reveal private functions and data structures.

The async patch protection subsystem periodically checks these critical data blobs and compares them
with some stored SHA3-512 hashes. If corrupted regions are detected and the corruption's severity
is beyond what simple repairs can handle, `monix` immediately calls `abort` to prevent further
damage.
