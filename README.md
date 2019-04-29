# Basilisk

This project is my effort to learn basics of compiler design and LLVM.
It is an LLVM frontend for a simple C-like language with no particular purpose.
I will be adding features to the language as I try to learn them and how they can be implemented.

## Built With

- [LLVM](http://llvm.org/),
- [Boost](https://www.boost.org/),
- [CMake](https://cmake.org/),
- [Doxygen](http://www.stack.nl/~dimitri/doxygen/),
- [CLang](https://clang.llvm.org/)

## Usage

The main executable is `basilisk`, located in the `tools` directory.
This executable handles the full compilation from a source file to an object file native for the host machine.
It supports input and output through standard streams and files, and output can be generated at any stage of the process by using command line options (`--lex`, `--parse`, ...).
For full usage description, run `basilisk -h` to display the help screen.

## Building

### Build Requirements

- Boost >= 1.69.0,
- LLVM (developed with 8.0.0svn)

The main supported compiler is Clang, but GCC should also work with minimal adjustments.
The recommended linker is LLVM's lld, mainly for the easy to understand warning messages.

### CMake Options

- `basilisk_BUILD_TEST` &mdash; build tests (default: ON),
- `basilisk_BUILD_DOC` &mdash; build documentation (default: ON),
- `basilisk_LLVM` &mdash; LLVM build directory (default: /opt/llvm),
- `basilisk_BOOST` &mdash; Boost directory (default: /opt/boost)

### Compilation Warnings
The compiler warnings enabled are all of `-Wall`, `-Wextra` and `-Wpedantic`.
There is also a configuration file and run script for clang-tidy.
The library itself should produce no warnings with either of these if at all possible.

## Contributing

Please read the [Contributing Guide](CONTRIBUTING.md) for details on the contribution process.

## Versioning

This project uses [Semantic Versioning](https://semver.org/).

## Authors

- [Filip Smola](https://smola.me)

## Licence

This project is licensed under the MIT licence - see [LICENCE.md](LICENCE.md) for details.
