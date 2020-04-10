#pragma once

//C Standard Libraries
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <chrono>

//STL
#include <vector>
#include <array>
#include <deque>
#include <map>
#include <algorithm>

//Smart Poiners
#include <memory>

//Dependencies
//MIRU
#include "miru_core.h"

//GLFW
#include "GLFW/x64/include/GLFW/glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/x64/include/GLFW/glfw3native.h"

//OpenAL
#include "OPENAL/include/AL/al.h"
#include "OPENAL/include/AL/alc.h"

//FreeType
#include "FREETYPE/include/ft2build.h"
#include FT_FREETYPE_H

//STB Image
#include "STBI/stb_image.h"

//Assimp
#include "ASSIMP/include/assimp/Importer.hpp"
#include "ASSIMP/include/assimp/scene.h"
#include "ASSIMP/include/assimp/postprocess.h"