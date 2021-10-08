# GEAR
Game Engine for Windows(x64) using D3D12 or Vulkan via MIRU.

![GEAR_LOGO](/Branding/GEAR_logo_dark.png)

GEAR (Game Engine Andrew Richards) is a 3D game engine, that I am developing to improve both my C++ and graphic programming, using my own Graphics (MIRU) and Mathematics (MARS) libraries. 
Inspired by Yan Chernikov's (aka TheCheno) videos on C++, OpenGL and Game Engines. https://www.youtube.com/c/TheChernoProject

This repository is under active development and is not currently intended for commerical release or use.

## PBR Rendering Demo
PBR and IBL Render
![pbr_ibl_render](/Branding/Screenshots/pbr_render_textured.jpg)

## GEARBOX Demo
GEARBOX Editor - Work in Progress
![gearbox_screenshot](/Branding/Screenshots/gearbox_screenshot.png)

# Projects:
## GEAR_CORE: 
Contains the core functionality of the game engine. Build as a static library; Dynamic Runtime Linking (MD).

## GEAR_MIPMAP:
Offline GPU-accelerated Mipmap generator. Build as executable; Dynamic Runtime Linking (MD).

## GEAR_TEST: 
Simple test application for development, test and demonstration. Build as executable; Dynamic Runtime Linking (MD).

## GEARBOX: 
A simple level editor using ImGui. Build as executable; Dynamic Runtime Linking (MD).

# Build Tools with Visual Studio:
## Windows x64:
- Microsoft Visual Studio 2019
- Toolset: v142 
- Windows SDK: 10.0.19401
- ISO C++ 17
