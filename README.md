# Volumetric cloud :) (WIP)

**This project goal is to do some simple volumetric cloud rendering**

[//]: # (<img src="assets/readme/game_map_demo.gif"  height=400>)

---

## 📂 Project Structure

- `engine/` — Core rendering engine and Vulkan abstractions.
- `executables/volumetric_sphere/` — The main implementation is inside this folder.
- `shared/` — Common utilities and shared components.
- `assets/` — Fonts, images, shaders, and other media assets.
- `submodules/` — External dependencies (e.g., Eigen).

---

## 🛠️ Build Instructions

First, ensure you have installed the Vulkan SDK and CMake (version 3.10+).

1. Clone the repository with submodules:
   ```bash
   git clone --recurse-submodules https://github.com/MohammadFakhreddin/VolumetricCloud.git

2. Create a build directory:
   ```bash
   mkdir build
   cd build
3. Configure the project with CMake:
   ```bash
   cmake ..
4. Build the project:
   ```bash
   cmake --build .

---

## 📖 Resources
// WIP