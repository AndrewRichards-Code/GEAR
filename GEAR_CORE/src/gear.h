#pragma once

//Maths Libratry
#include "maths/ARMLib.h"

//Audio
#include "audio/listener.h"
#include "audio/audiosource.h"

//Graphics
//Crossplatform
#include "graphics/crossplatform/light.h"
#include "graphics/crossplatform/camera.h"

//OpenGL
#if GEAR_OPENGL
#if _DEBUG
#include "graphics/opengl/debugopengl.h"
#endif

#include "graphics/opengl/buffer/buffermanager.h"
#include "graphics/opengl/buffer/framebuffer.h"
#include "graphics/opengl/buffer/indexbuffer.h"
#include "graphics/opengl/buffer/shaderstoragebuffer.h"
#include "graphics/opengl/buffer/uniformbuffer.h"
#include "graphics/opengl/buffer/vertexarray.h"
#include "graphics/opengl/buffer/vertexbuffer.h"

#include "graphics/opengl/renderer/batchrenderer2d.h"
#include "graphics/opengl/renderer/compute.h"
#include "graphics/opengl/renderer/renderer.h"

#include "graphics/opengl/shader/computeshader.h"
#include "graphics/opengl/shader/shader.h"

#include "graphics/opengl/window.h"
#include "graphics/opengl/texture.h"
#include "graphics/opengl/font.h"
#endif

//Input
#include "input/inputmanager.h"

//Utils
#include "utils/fileutils.h"