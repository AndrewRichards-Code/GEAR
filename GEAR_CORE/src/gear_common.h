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
//OpenGL
#ifdef GEAR_OPENGL

#include "GLEW/include/GL/glew.h"

#ifdef _M_IX86
#include "GLFW/x86/include/GLFW/glfw3.h"
#elif defined _M_X64
#include "GLFW/x64/include/GLFW/glfw3.h"
#endif
#endif

//OpenAL
#include "OPENAL/include/AL/al.h"
#include "OPENAL/include/AL/alc.h"

//FreeType
#include "FREETYPE/include/ft2build.h"
#include FT_FREETYPE_H

//STB Image
#include "STBI/stb_image.h"

//Assimp
#ifdef _M_X64
#include "ASSIMP/include/assimp/Importer.hpp"
#include "ASSIMP/include/assimp/scene.h"
#include "ASSIMP/include/assimp/postprocess.h"
#endif