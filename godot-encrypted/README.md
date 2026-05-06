# Godot Engine (Custom Build: godot-encrypted)

<p align="center">
  <a href="https://godotengine.org">
    <img src="logo_outlined.svg" width="400" alt="Godot Engine logo">
  </a>
</p>

## ⚠️ Custom Security Edition

**This version of the Godot Engine has been modified to include a custom cryptographic security layer.**

While it retains all the powerful features of the standard [Godot Engine](https://godotengine.org), this specific build (`godot-encrypted`) integrates a native C++ security module designed to protect game assets and network traffic through advanced encryption.

### 🔐 Custom Cryptographic Implementation
Unlike the vanilla version of the engine, this project includes:
*   **AES-256 / RSA Core Integration:** The engine's resource loader is modified to handle encrypted `.pck` files using an AES-256/RSA hybrid system.
*   **On-the-fly Decryption:** Assets are decrypted directly in memory during the loading process, preventing sensitive data from being written to the disk in plain text.
*   **Native OpenSSL Support:** Full integration of OpenSSL within the core for securing both local storage and network communication.
*   **Custom SCons Build Options:** New build flags have been added to facilitate the compilation of the security module:
    ```bash
    scons platform=windows module_openssl_enabled=yes target=editor
    ```

---

## 2D and 3D cross-platform game engine

**Godot Engine is a feature-packed, cross-platform game engine to create 2D and 3D games from a unified interface.** It provides a comprehensive set of [common tools](https://godotengine.org/features), so that users can focus on making games without having to reinvent the wheel.

## Free, open source and community-driven

Godot is completely free and open source under the very permissive [MIT license](https://godotengine.org/license). No strings attached, no royalties, nothing. The users' games are theirs, down to the last line of engine code.
