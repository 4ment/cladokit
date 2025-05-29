# Contributing to cladokit

Thank you for your interest in contributing! Please follow these guidelines to help us review your changes quickly.

## Getting Started

1. **Fork the repository** and clone your fork.
2. **Create a new branch** for your feature or bugfix.
3. **Install dependencies** using the instructions in the README.

## Language Standard

We use **C++17** throughout the project. All contributions must conform to this standard.

Make sure your compiler and tools are configured appropriately. For example, compile with `-std=c++17` when using `clang`, `gcc`, or `clang-tidy`.

## Code Style

cladokit follows the **PascalCase** naming convention for classes and methods while **camelCase** is used for variables.

Examples of **PascalCase** classes:
- ✅ `TreeMetric`, `NexusFile`,  `Node`
- ❌ `treemetric`, `nexus_file`, `node`

Examples of **camelCase** variables:
- ✅ `treeMetric`, `nexusFile`, `tree`
- ❌ `Treemetric`, `nexus_file`, `Tree`

Please follow existing naming patterns in the codebase for consistency.

## Making Changes

- Write clear, concise commit messages.
- Follow the project's coding style.
- Add or update tests as needed.
- **Testing:** Ensure all tests pass before submitting your changes. Add new tests for new features or bug fixes.
- **Code Quality:** Run `clang-tidy` and `cpplint` to check for code issues and style violations.
- **Formatting:** Use `clang-format` to automatically format your code according to the project's style guidelines.

## Formatting

You can run `clang-format` manually on multiple files:

```sh
clang-format -i src/**/**.cpp
```

Alternatively, you can run `clang-format` via CMake:

```sh
cmake --build build/ --target format
```

This will format the source files in `src` according to the project's configuration.

## Linting

We use **cpplint** to enforce coding style and conventions. Run cpplint on your changes before submitting a pull request:

```sh
cpplint src/**/*.cpp src/**/*.hpp tests/*.cpp
```

Address any warnings or errors reported by cpplint. For more details, refer to the [cpplint documentation](https://github.com/cpplint/cpplint).

## Running Tests

To ensure your changes do not break existing functionality, always run the test suite before submitting a pull request.

You can run all tests using CMake and ctest:

```sh
cmake -S . -B build -DBUILD_TESTING=on
cmake --build build
ctest --test-dir build
```

Or, if you prefer, you can run the test binaries directly in the `build` directory:

```sh
./build/run_tests
```


## Pull Requests

- Ensure your branch is up-to-date with `main`.
- Describe your changes in the pull request.
- Reference related issues if applicable.

## Code of Conduct

Please be respectful.

---

Thank you for helping improve cladokit!