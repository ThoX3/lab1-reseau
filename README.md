# 🌐 Network Lab - GodotPP (C++ & Security)

This repository contains the source code for a multiplayer networking lab developed with the **Godot** engine and **C++**. 
The project implements a robust Client-Server architecture, integrating a custom-compiled Godot engine and a cryptographic security system (RSA/AES) to protect game files and communications.

## ✨ Main Features

*   **GDExtension (C++) Architecture:** Game logic and server developed in native C++ for optimal performance.
*   **Standalone Server:** A 100% dedicated C++ server (`GodotPPServer.exe`) independent of the Godot executable.
*   **"Cheat" Test Client:** A parallel environment (`game_cheat`) allowing testing of network vulnerabilities and server protections.
*   **Encrypted Network Traffic:** All data packets sent between the client and the server are **encrypted using OpenSSL**, ensuring secure communication and preventing packet sniffing or man-in-the-middle attacks.
*   **Custom Engine & Encryption:** Use of a modified version of the Godot engine (`godot-encrypted`) capable of compiling `.pck` files and building clients that can encrypt/decrypt assets on the fly.
*   **Automated CMake Pipeline:** A comprehensive build script managing the C++ code compilation, engine assembly, exportation, and final Python encryption.

---

## 📂 Initial Project Structure
```text
lab1-reseau/
├── GodotPP/
│   ├── CMakeLists.txt        # Main build orchestrator
│   ├── src/                  # C++ source code (Client, Server, Extensions)
│   ├── externals/            # External dependencies (e.g., OpenSSL)
│   └── game/                 # Clean Godot project (Normal client)
├── godot-encrypted/          # Source code for the custom Godot engine (with RSA keys)
├── snl/                      # Stupid Networking Library
├── vcpkg.json/               # Vcpkg manifest for dependency management
└── vcpkg/                    # Package manager (for OpenSSL)
