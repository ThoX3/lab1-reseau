# GodotPP

**GodotPP** is a high-performance networking GDExtension for Godot 4.x. It leverages the safety and speed of **Rust** via the **SNL (Simple Network Library)** and integrates it into a **C++** GDExtension using **Corrosion** and **CMake**.

This project features a "Superbuild" workflow designed for education, allowing students to manage the entire lifecycle—from engine setup to cross-platform packaging—directly within CMake.

## 🚀 Quick Start

### Prerequisites
* **C++17 Compiler**: Clang (macOS), MSVC (Windows), or GCC (Linux).
* **Rust & Cargo**: Required to compile the SNL crate.
* **CMake 3.16+**: Orchestrates the build.
* **Python 3 & SCons**: Required to build the `godot-cpp` bindings.
* **Ninja**: (Recommended) Fast build generator.

### Initial Setup
Clone the repository (including submodules) and run the setup to download the Godot Engine binary:

# Configure the project and build custom targets for the editor, game, cheat, and server.
```bash
cmake -S . -B build `
  "-DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake" `
  -DVCPKG_MANIFEST_MODE=ON `
  -DVCPKG_TARGET_TRIPLET=x64-windows
```

### Development Workflow

The build system handles the Rust compilation, header generation via cbindgen, and C++ linking automatically.
- Build everything: `cmake --build build`
- Launch Godot Editor: `cmake --build build --target editor`
- Run the Game: `cmake --build build --target play`
- Run the Cheat: `cmake --build build --target cheat`
- Run the Server: `cmake --build build --target server`

## 🛠 Project Architecture

- **SNL (Rust)**: High-level networking logic using non-blocking UDP sockets.
- **GDExtension (C++)**: Acts as the bridge, exposing Rust functions as native Godot nodes (e.g., MyNetworkNode).
- **CMake Superbuild**: Uses FetchContent to manage godot-cpp and Corrosion to manage the Rust toolchain.

## 📦 Packaging for Release
The project includes a platform-aware packaging target. It ensures the distribution directory exists and calls the Godot headless exporter.

```bash
cmake --build build --target package
```

The exported artifacts (e.g., .exe for Windows, .zip for macOS, or .x86_64 for Linux) will be located in `build/`.

## 🤖 CI/CD Automation
This repository includes a GitHub Action that automates the build for Windows and Linux. The workflow:
- Installs Rust, Python, and SCons.
- Caches and installs Godot Export Templates in the correct OS directories (%APPDATA% on Windows, ~/.local on Linux).
- Compiles the SNL library and GDExtension.
- Packages the final game as a downloadable artifact.

## ⚖️ License

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or distribute this software, either in source code form or as a compiled binary, for any purpose, commercial or non-commercial, and by any means.

For more information, please refer to the `LICENSE` file or <http://unlicense.org/>.
