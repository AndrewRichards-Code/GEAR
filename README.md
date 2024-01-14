# GEAR
Game Engine for Windows(x64) using D3D12 or Vulkan via MIRU.

![GEAR_LOGO](/Branding/GEAR_logo_dark.png)

GEAR (Game Engine Andrew Richards) is a 3D game engine, that I am developing to improve both my C++ and graphic programming, using my own Graphics (MIRU) and Mathematics (MARS) libraries. 
Inspired by Yan Chernikov's (aka TheCheno) videos on C++, OpenGL and Game Engines. https://www.youtube.com/c/TheChernoProject

This repository is under active development and is not currently intended for commerical release or use.

## GEARBOX Demo
GEARBOX Editor - Work in Progress
![gearbox_screenshot](/Branding/Screenshots/gearbox_screenshot_2.png)

## Features (Work in Progress)
* D3D12 or Vulkan Graphics API with [MIRU](https://github.com/AndrewRichards-Code/MIRU).
* Render Grpah system for resource management.
* PBR and IBL Rendering.
* Shadows for Directional, Spot and Point lights.
* Bloom Post-Processing and HDR tone-mapping.
* Project, Scene and ECS systems.
* GEARBOX Editor with ImGui and ImGuizmo.
* Runtime Shader and Pipeline recompilation.

# Projects
## GEAR_CORE
Contains the core functionality of the game engine. Build as a dynamic library; Dynamic Runtime Linking (MD).

## GEAR_NATIVE_SCRTIPT
Contains the native script that are dynamically loaded by GEAR_CORE. Build as a dynamic library; Dynamic Runtime Linking (MD).

## GEARBOX
A simple level editor using ImGui. Build as executable; Dynamic Runtime Linking (MD).

## External Projects

* [MIRU](https://github.com/AndrewRichards-Code/MIRU)
* [MARS](https://github.com/AndrewRichards-Code/MARS)
* [assimp](https://github.com/assimp/assimp)
* [entt](https://github.com/skypjack/entt)
* [FreeType](https://freetype.org/)
* [GLFW](https://github.com/glfw/glfw)
* [ImGui](https://github.com/ocornut/imgui)
* [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo)


# CMake and  Visual Studio
## Windows x64:
- Microsoft Visual Studio 2022
- Toolset: v143 
- Windows SDK: 10.0.22621.0
- ISO C++ 20
