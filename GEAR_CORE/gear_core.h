#pragma once

//Animation
#include "Animation/Animation.h"
#include "Animation/Animator.h"

//Asset
#include "Asset/Asset.h"
#include "Asset/AssetDataBuffer.h"
#include "Asset/AssetFile.h"
#include "Asset/AssetMetadata.h"
#include "Asset/AssetRegistry.h"
#include "Asset/Manager/AssetManager.h"
#include "Asset/Manager/EditorAssetManager.h"

//Audio
#include "Audio/AudioInterfaces.h"
#include "Audio/AudioSource.h"
#include "Audio/AudioListener.h"

//Core
#include "Core/Application.h"
#include "Core/ApplicationContext.h"
#include "Core/CommandLineOptions.h"
#include "Core/ConfigFile.h"
#include "Core/FileDialog.h"
#include "Core/Hashing.h"
#include "Core/JsonFileHelper.h"
#include "Core/PlatformMacros.h"
#include "Core/Sequencer.h"
#include "Core/Timer.h"
#include "Core/TypesCppHlsl.h"
#include "Core/UUID.h"

//Graphics
#include "Graphics/AllocatorManager.h"
#include "Graphics/Colour.h"
#include "Graphics/Indexbuffer.h"
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
#include "Objects/Frustum.h"
#include "Objects/Font.h"
#include "Objects/Light.h"
#include "Objects/Material.h"
#include "Objects/Mesh.h"
#include "Objects/Model.h"
#include "Objects/Picker.h"
#include "Objects/Probe.h"
#include "Objects/Skybox.h"
#include "Objects/Text.h"
#include "Objects/Transform.h"

//Project
#include "Project/Project.h"

//Rendering
#include "Rendering/DebugRender.h"
#include "Rendering/Pass.h"
#include "Rendering/PassParameters.h"
#include "Rendering/Renderer.h"
#include "Rendering/RenderGraph.h"
#include "Rendering/Resource.h"

//Scene
#include "Scene/Components.h"
#include "Scene/Entity.h"
#include "Scene/Scene.h"

//Scripting
#include "Scripting/NativeScript.h"
#include "Scripting/NativeScriptManager.h"

//UI
#include "UI/MenuBar.h"
#include "UI/UIContext.h"
#include "UI/ComponentUI/ComponentUIs.h"
#include "UI/Panels/Panels.h"