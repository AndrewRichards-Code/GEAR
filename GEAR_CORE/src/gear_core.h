#pragma once

//Animation
#include "Animation/Animation.h"
#include "Animation/Animator.h"

//Audio
#include "Audio/AudioInterfaces.h"
#include "Audio/AudioSource.h"
#include "Audio/AudioListener.h"

//Build
#include "Build/Project.h"

//Core
#include "Core/Application.h"
#include "Core/ApplicationContext.h"
#include "Core/AssetFile.h"
#include "Core/CommandLineOptions.h"
#include "Core/ConfigFile.h"
#include "Core/FileDialog.h"
#include "Core/FontLibrary.h"
#include "Core/Hashing.h"
#include "Core/JsonFileHelper.h"
#include "Core/PlatformMacros.h"
#include "Core/Sequencer.h"
#include "Core/Timer.h"
#include "Core/TypesCppHlsl.h"
#include "Core/UUID.h"

//Graphics
#include "Graphics/Rendering/Pass.h"
#include "Graphics/Rendering/PassParameters.h"
#include "Graphics/Rendering/Renderer.h"
#include "Graphics/Rendering/RenderGraph.h"
#include "Graphics/Rendering/Resource.h"
#include "Graphics/AllocatorManager.h"
#include "Graphics/Colour.h"
#include "Graphics/Frustum.h"
#include "Graphics/DebugRender.h"
#include "Graphics/Indexbuffer.h"
#include "Graphics/Picker.h"
#include "Graphics/RenderPipeline.h"
#include "Graphics/RenderSurface.h"
#include "Graphics/Storagebuffer.h"
#include "Graphics/Texture.h"
#include "Graphics/Uniformbuffer.h"
#include "Graphics/Vertexbuffer.h"
#include "Graphics/Window.h"

//Input
#include "Input/InputManager.h"
#include "Input/InputInterfaces.h"

//Objects
#include "Objects/Camera.h"
#include "Objects/Light.h"
#include "Objects/Material.h"
#include "Objects/Mesh.h"
#include "Objects/Model.h"
#include "Objects/Probe.h"
#include "Objects/Skybox.h"
#include "Objects/Text.h"
#include "Objects/Transform.h"

//Scene
#include "Scene/Components.h"
#include "Scene/Entity.h"
#include "Scene/NativeScript.h"
#include "Scene/NativeScriptManager.h"
#include "Scene/Scene.h"

//UI
#include "UI/MenuBar.h"
#include "UI/UIContext.h"
#include "UI/ComponentUI/ComponentUIs.h"
#include "UI/Panels/Panels.h"

//Utils
#include "Utils/FileUtils.h"
#include "Utils/ModelLoader.h"