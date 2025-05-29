# cladokit

[![CMake on a single platform](https://github.com/4ment/cladokit/actions/workflows/cmake-single-platform.yml/badge.svg)](https://github.com/4ment/cladokit/actions/workflows/cmake-single-platform.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)


**cladokit** is a simple C++ library for manipulating and analyzing tree-like data structures. It is designed to be modular, efficient, and easy to integrate into larger projects.

## Features

- Tree data structures and utilities
- Node manipulation and traversal
- Utility functions for common operations
- Minimal dependencies and straightforward integration
- Unit tests for core components

## Getting Started

### Prerequisites

- CMake 3.15 or higher
- A C++17 compatible compiler

### Building

```sh
cmake -S . -B build
cmake --build build
```

### Installing

```sh
cmake --build build/ --target install
```

## Code Style

This project follows the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) with minor customizations.

Tools used:

- clang-format for formatting
- clang-tidy and cpplint for static analysis and linting

See [CONTRIBUTING.md](CONTRIBUTING.md) for more information on contributing and style requirements.

## Contributing

Contributions are welcome! Please see the [CONTRIBUTING.md](CONTRIBUTING.md) file for guidelines and instructions.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
