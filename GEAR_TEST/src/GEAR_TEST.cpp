#include "GEAR_TEST.h"
#include "gear_core.h"

using namespace gear;
using namespace animation;
using namespace audio;
using namespace core;
using namespace graphics;
using namespace objects;
using namespace scene;

using namespace miru;
using namespace miru::crossplatform;

using namespace mars;

Ref<Application> CreateApplication(int argc, char** argv)
{
	return CreateRef<GEAR_TEST>();
}

void GEAR_TEST::Run()
{
	input::InputInterface::CreateInfo iCI = { input::InputInterface::API::XINPUT, 0 };
	input::InputInterface input(&iCI);
	GEAR_SET_ERROR_CODE_TO_STRING_FUNCTION;

#if 0
	AudioListener::CreateInfo listenerCI;
	listenerCI.audioAPI = AudioListenerInterface::API::XAUDIO2;
	listenerCI.endPointDevice = AudioListenerInterface::EndPointDevice::HEADPHONES_XBOX_CONTROLLER;
	Ref<AudioListener> listener = CreateRef<AudioListener>(&listenerCI);

	AudioSource::CreateInfo rainbowRaodCI;
	rainbowRaodCI.filepath = "res/wav/Rainbow Road.wav";
	rainbowRaodCI.pAudioListener = listener;
	Ref<AudioSource> rainbowRoad = CreateRef<AudioSource>(&rainbowRaodCI);
	rainbowRoad->SetPitch(0.0f);
	rainbowRoad->SetVolume(0.0f);
	rainbowRoad->Stream();
#endif
	Scene::CreateInfo sceneCI;
	sceneCI.debugName = "GEAR_TEST_Main_Scene";
	sceneCI.filepath = "res/scenes/current_scene.gsf.json";
	sceneCI.nativeScriptDir = "res/scripts/";
	Ref<Scene> activeScene = CreateRef<Scene>(&sceneCI);

	Window::CreateInfo windowCI;
	windowCI.api = GraphicsAPI::API::D3D12;
	//windowCI.api = GraphicsAPI::API::VULKAN;
	windowCI.title = "GEAR_TEST";
	windowCI.width = 1920;
	windowCI.height = 1080;
	windowCI.fullscreen = false;
	windowCI.fullscreenMonitorIndex = 1;
	windowCI.maximised = false;
	windowCI.vSync = true;
	windowCI.samples = Image::SampleCountBit::SAMPLE_COUNT_2_BIT;
	windowCI.graphicsDebugger = debug::GraphicsDebugger::DebuggerType::RENDER_DOC;
	Ref<Window> window = CreateRef<Window>(&windowCI);

	AllocatorManager::CreateInfo mbmCI;
	mbmCI.pContext = window->GetContext();
	mbmCI.defaultBlockSize = Allocator::BlockSize::BLOCK_SIZE_128MB;
	AllocatorManager::Initialise(&mbmCI);

	Ref<FontLibrary> fontLib = CreateRef<FontLibrary>();
	FontLibrary::LoadInfo fontLI;
	fontLI.GI.filepath = "res/font/Source_Code_Pro/SourceCodePro-Regular.ttf";
	fontLI.GI.fontHeightPx = 16;
	fontLI.GI.generatedTextureSize = 1024;
	fontLI.GI.savePNGandBINfiles = true;
	fontLI.regenerateTextureAtlas = false;
	Ref<FontLibrary::Font> font = fontLib->LoadFont(&fontLI);

	Text::CreateInfo fontRendererCI;
	fontRendererCI.device = window->GetDevice();
	fontRendererCI.viewportWidth = window->GetWidth();
	fontRendererCI.viewportHeight = window->GetHeight();
	Entity textEntity = activeScene->CreateEntity();
	textEntity.AddComponent<TextComponent>(std::move(CreateRef<Text>(&fontRendererCI)));
	uint32_t textRowHeight = 20, render_doc_offset = 40;
	auto& text = textEntity.GetComponent<TextComponent>().text;
	text->AddLine(font, window->GetDeviceName(), { 0, render_doc_offset + 0 * textRowHeight }, Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	text->AddLine(font, window->GetGraphicsAPIVersion(), { 0, render_doc_offset + 1 * textRowHeight }, GraphicsAPI::IsD3D12() ? Vec4(0.0f, 1.0f, 0.0f, 1.0) : Vec4(1.0f, 0.0f, 0.0f, 1.0f));
	text->AddLine(font, "FPS: " + window->GetFPSString<uint32_t>(), { 0, render_doc_offset + 2 * textRowHeight }, Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	text->AddLine(font, "MSAA: " + window->GetAntiAliasingValue() + "x", { 0, render_doc_offset + 3 * textRowHeight }, Vec4(1.0f, 1.0f, 1.0f, 1.0f));

	Skybox::CreateInfo skyboxCI;
	skyboxCI.debugName = "Skybox-HDR";
	skyboxCI.device = window->GetDevice();
	//skyboxCI.filepaths = { "res/img/snowy_park_01_2k.hdr" };
	skyboxCI.filepaths = { "res/img/kloppenheim_06_2k.hdr" };
	skyboxCI.generatedCubemapSize = 1024;
	skyboxCI.transform.translation = Vec3(0, 0, 0);
	skyboxCI.transform.orientation = Quat(1, 0, 0, 0);
	skyboxCI.transform.scale = Vec3(500.0f, 500.0f, 500.0f);
	Entity skyboxEntity = activeScene->CreateEntity();
	skyboxEntity.AddComponent<SkyboxComponent>(std::move(CreateRef<Skybox>(&skyboxCI)));

	auto LoadTexture = [](Ref<Window> window, const std::string& filepath, const std::string& debugName, bool linear = false) -> Ref<graphics::Texture>
	{
		Texture::CreateInfo texCI;
		texCI.debugName = debugName.c_str();
		texCI.device = window->GetDevice();
		texCI.dataType = Texture::DataType::FILE;
		texCI.file.filepaths = { filepath };
		texCI.mipLevels = GEAR_TEXTURE_MAX_MIP_LEVEL;
		texCI.arrayLayers = 1;
		texCI.type = miru::crossplatform::Image::Type::TYPE_2D;
		texCI.format = linear ? miru::crossplatform::Image::Format::R32G32B32A32_SFLOAT : miru::crossplatform::Image::Format::R8G8B8A8_UNORM;
		texCI.samples = miru::crossplatform::Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
		texCI.usage = miru::crossplatform::Image::UsageBit(0);
		texCI.generateMipMaps = true;
		texCI.gammaSpace = linear ? GammaSpace::LINEAR : GammaSpace::SRGB;
		return std::move(CreateRef<Texture>(&texCI));
	};

	//std::future<Ref<graphics::Texture>> rustIronAlbedo = std::async(std::launch::async, LoadTexture, window, std::string("res/img/rustediron2-Unreal-Engine/rustediron2_basecolor.png"), std::string("UE4 Rust Iron: Albedo"));
	//std::future<Ref<graphics::Texture>> rustIronMetallic = std::async(std::launch::async, LoadTexture, window, std::string("res/img/rustediron2-Unreal-Engine/rustediron2_metallic.png"), std::string("UE4 Rust Iron: Metallic"));
	//std::future<Ref<graphics::Texture>> rustIronNormal = std::async(std::launch::async, LoadTexture, window, std::string("res/img/rustediron2-Unreal-Engine/rustediron2_normal.png"), std::string("UE4 Rust Iron: Normal"));
	//std::future<Ref<graphics::Texture>> rustIronRoughness = std::async(std::launch::async, LoadTexture, window, std::string("res/img/rustediron2-Unreal-Engine/rustediron2_roughness.png"), std::string("UE4 Rust Iron: Roughness"));

	//std::future<Ref<graphics::Texture>> slateAlbedo = std::async(std::launch::async, LoadTexture, window, std::string("res/img/slate2-tiled-ue4/slate2-tiled-albedo2.png"), std::string("UE4 Slate: Albedo"));
	//std::future<Ref<graphics::Texture>> slateMetallic = std::async(std::launch::async, LoadTexture, window, std::string("res/img/slate2-tiled-ue4/slate2-tiled-metalness.png"), std::string("UE4 Slate: Metallic"));
	//std::future<Ref<graphics::Texture>> slateNormal = std::async(std::launch::async, LoadTexture, window, std::string("res/img/slate2-tiled-ue4/slate2-tiled-normal3-UE4.png"), std::string("UE4 Slate: Normal"));
	//std::future<Ref<graphics::Texture>> slateRoughness = std::async(std::launch::async, LoadTexture, window, std::string("res/img/slate2-tiled-ue4/slate2-tiled-rough.png"), std::string("UE4 Slate: Roughness"));
	//std::future<Ref<graphics::Texture>> slateAO = std::async(std::launch::async, LoadTexture, window, std::string("res/img/slate2-tiled-ue4/slate2-tiled-ao.png"), std::string("UE4 Slate: AO"));

	Material::CreateInfo matCI;
	/*matCI.debugName = "UE4 Rust Iron";
	matCI.device = window->GetDevice();
	matCI.pbrTextures = {
		{ Material::TextureType::NORMAL, rustIronNormal.get() },
		{ Material::TextureType::ALBEDO, rustIronAlbedo.get() },
		{ Material::TextureType::METALLIC, rustIronMetallic.get() },
		{ Material::TextureType::ROUGHNESS, rustIronRoughness.get() }
	};
	Ref<Material> rustIronMaterial = CreateRef<Material>(&matCI);*/

	/*matCI.debugName = "UE4 Slate";
	matCI.device = window->GetDevice();
	matCI.pbrTextures = {
		{ Material::TextureType::NORMAL, slateNormal.get() },
		{ Material::TextureType::ALBEDO, slateAlbedo.get() },
		{ Material::TextureType::METALLIC, slateMetallic.get() },
		{ Material::TextureType::ROUGHNESS, slateRoughness.get() },
		{ Material::TextureType::AMBIENT_OCCLUSION, slateAO.get()}
	};
	Ref<Material> slateMaterial = CreateRef<Material>(&matCI);*/

	Mesh::CreateInfo meshCI;
	/*meshCI.debugName = "quad.fbx";
	meshCI.device = window->GetDevice();
	meshCI.filepath = "res/obj/quad.fbx";
	Ref<Mesh> quadMesh = CreateRef<Mesh>(&meshCI);
	quadMesh->SetOverrideMaterial(0, slateMaterial);*/

	/*meshCI.debugName = "sphere.fbx";
	meshCI.device = window->GetDevice();
	meshCI.filepath = "res/obj/sphere.fbx";
	Ref<Mesh> sphereMesh = CreateRef<Mesh>(&meshCI);
	sphereMesh->SetOverrideMaterial(0, rustIronMaterial);*/

	/*meshCI.debugName = "KagemitsuG4.fbx";
	meshCI.device = window->GetDevice();
	meshCI.filepath = "res/obj/KagemitsuG4.fbx";
	Ref<Mesh> kagemitsuG4Mesh = CreateRef<Mesh>(&meshCI);*/

	/*modelCI.debugName = "Quad";
	modelCI.device = window->GetDevice();
	modelCI.pMesh = quadMesh;
	modelCI.materialTextureScaling = Vec2(100.0f, 100.0f);
	modelCI.transform.translation = Vec3(0, 0, 0);
	modelCI.transform.orientation = Quat(sqrt(2)/2, -sqrt(2)/2, 0, 0);
	modelCI.transform.scale = Vec3(100.0f, 100.0f, 100.0f);
	modelCI.renderPipelineName = "PBROpaque";
	Entity quad = activeScene->CreateEntity();
	quad.AddComponent<ModelComponent>(std::move(CreateRef<Model>(&modelCI)));*/

	Model::CreateInfo modelCI;
	/*modelCI.debugName = "KagemitsuG4";
	modelCI.device = window->GetDevice();
	modelCI.pMesh = kagemitsuG4Mesh;
	modelCI.transform.translation = Vec3(0.0, 0.825, -0.1);
	modelCI.transform.orientation = Quat(cos(DegToRad(45.0)), sin(DegToRad(45.0)), 0, 0);
	modelCI.transform.scale = Vec3(1.0f, 1.0f, 1.0f);
	modelCI.renderPipelineName = "PBROpaque";
	Entity KagemitsuG4 = activeScene->CreateEntity();
	KagemitsuG4.AddComponent<ModelComponent>(CreateRef<Model>(&modelCI));*/

#if 0
	uint32_t gridSize = 5;
	for (uint32_t i = 0; i < gridSize; i++)
	{
		for (uint32_t j = 0; j < gridSize; j++)
		{
			float pos_x = (2.0f / float(gridSize)) * float(int(i) - int(gridSize / 2));
			float pos_y = (2.0f / float(gridSize)) * float(int(j) - int(gridSize / 2)) + 1.0f;

			matCI.debugName = "Test Material_" + std::to_string(i) + "_" + std::to_string(j);
			matCI.device = window->GetDevice();
			matCI.pbrConstants.fresnel = Vec4(1, 1, 1, 1);
			matCI.pbrConstants.albedo = Vec4(1, 0, 0, 1);
			matCI.pbrConstants.metallic = (float(i) + 0.5f) / float(gridSize);
			matCI.pbrConstants.roughness = (float(j) + 0.5f) / float(gridSize);
			matCI.pbrConstants.ambientOcclusion = 0.0f;
			matCI.pbrConstants.emissive = Vec4(0, 0, 0, 1);
			Ref<Material> testMaterial = CreateRef<Material>(&matCI);

			meshCI.debugName = "sphere.fbx";
			meshCI.device = window->GetDevice();
			meshCI.filepath = "res/obj/sphere.fbx";
			Ref<Mesh> sphereMesh = CreateRef<Mesh>(&meshCI);
			sphereMesh->SetOverrideMaterial(0, testMaterial);

			modelCI.debugName = "Sphere";
			modelCI.device = window->GetDevice();
			modelCI.pMesh = sphereMesh;
			modelCI.materialTextureScaling = Vec2(1.0f, 1.0f);
			modelCI.transform.translation = Vec3(pos_x, pos_y, -2.0f);
			modelCI.transform.orientation = Quat(cos(DegToRad(45.0)), -sin(DegToRad(45.0)), 0, 0);
			modelCI.transform.scale = Vec3(1.0f / float(gridSize), 1.0f / float(gridSize), 1.0f / float(gridSize));
			modelCI.renderPipelineName = "PBROpaque";
			Entity sphere = activeScene->CreateEntity();
			sphere.AddComponent<ModelComponent>(std::move(CreateRef<Model>(&modelCI)));
		}
	}
#endif

	std::future<Ref<graphics::Texture>> droneAlbedo = std::async(std::launch::async, LoadTexture, window, std::string("res/img/drone/Totally_LP_defaultMat_BaseColor.png"), std::string("Drone: Albedo"), true);
	std::future<Ref<graphics::Texture>> droneMetallic = std::async(std::launch::async, LoadTexture, window, std::string("res/img/drone/Totally_LP_defaultMat_Metallic.png"), std::string("Drone: Metallic"), true);
	std::future<Ref<graphics::Texture>> droneNormal = std::async(std::launch::async, LoadTexture, window, std::string("res/img/drone/Totally_LP_defaultMat_Normal.png"), std::string("Drone: Normal"));
	std::future<Ref<graphics::Texture>> droneRoughness = std::async(std::launch::async, LoadTexture, window, std::string("res/img/drone/Totally_LP_defaultMat_Roughness.png"), std::string("Drone: Roughness"), true);
	std::future<Ref<graphics::Texture>> droneAO = std::async(std::launch::async, LoadTexture, window, std::string("res/img/drone/Totally_LP_defaultMat_Occlusion.png"), std::string("Drone: AO"), true);
	std::future<Ref<graphics::Texture>> droneEmissive = std::async(std::launch::async, LoadTexture, window, std::string("res/img/drone/Totally_LP_defaultMat_Emissive.png"), std::string("Drone: Emissive"), true);

	matCI.debugName = "Drone Material";
	matCI.device = window->GetDevice();
	matCI.pbrTextures = {
		{ Material::TextureType::NORMAL, droneNormal.get() },
		{ Material::TextureType::ALBEDO, droneAlbedo.get() },
		{ Material::TextureType::METALLIC, droneMetallic.get() },
		{ Material::TextureType::ROUGHNESS, droneRoughness.get() },
		{ Material::TextureType::AMBIENT_OCCLUSION, droneAO.get() },
		{ Material::TextureType::EMISSIVE, droneEmissive.get() },
	};
	Ref<Material> droneMaterial = CreateRef<Material>(&matCI);

	meshCI.debugName = "Drone Mesh";
	meshCI.device = window->GetDevice();
	meshCI.filepath = "res/obj/Drone_Animated_03.fbx";
	Ref<Mesh> droneMesh = CreateRef<Mesh>(&meshCI);
	for(size_t i = 0; i < droneMesh->GetMaterials().size(); i++)
		droneMesh->SetOverrideMaterial(i, droneMaterial);

	modelCI.debugName = "Drone Model";
	modelCI.device = window->GetDevice();
	modelCI.pMesh = droneMesh;
	modelCI.transform.translation = Vec3(0.0, 0.5, -1.0);
	modelCI.transform.orientation = Quat(sqrtf(2) / 2, -sqrtf(2) / 2, 0, 0);
	modelCI.transform.scale = Vec3(0.01f, 0.01f, 0.01f);
	modelCI.renderPipelineName = "PBROpaque";
	Entity drone = activeScene->CreateEntity();
	drone.AddComponent<ModelComponent>(CreateRef<Model>(&modelCI));

	Light::CreateInfo lightCI;
	lightCI.debugName = "Main";
	lightCI.device = window->GetDevice();
	lightCI.type = Light::LightType::POINT;
	lightCI.colour = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	lightCI.transform.translation = Vec3(0.0f, 2.0f, 0.0f);
	lightCI.transform.orientation = Quat(1, 0, 0, 0);
	lightCI.transform.scale = Vec3(1.0f, 1.0f, 1.0f);
	Entity lightEntity = activeScene->CreateEntity();
	lightEntity.AddComponent<LightComponent>(std::move(CreateRef<Light>(&lightCI)));

	Camera::CreateInfo cameraCI;
	cameraCI.debugName = "Main";
	cameraCI.device = window->GetDevice();
	cameraCI.transform.translation = Vec3(0, 1, 0);
	cameraCI.transform.orientation = Quat(1, 0, 0, 0);
	cameraCI.transform.scale = Vec3(1.0f, 1.0f, 1.0f);
	cameraCI.projectionType = Camera::ProjectionType::PERSPECTIVE;
	cameraCI.perspectiveParams = { DegToRad(90.0), window->GetRatio(), 0.01f, 3000.0f };
	cameraCI.flipX = false;
	cameraCI.flipY = false;
	Entity cameraEntity = activeScene->CreateEntity();
	cameraEntity.AddComponent<CameraComponent>(std::move(CreateRef<Camera>(&cameraCI)));
	cameraEntity.AddComponent<NativeScriptComponent>("TestScript");

	Animator::CreateInfo animatorCI;
	animatorCI.debugName = "Drone Animator";
	animatorCI.pMesh = droneMesh;
	Ref<Sequencer> sequencer = CreateRef<Animator>(&animatorCI);

	Renderer::CreateInfo rendererCI;
	rendererCI.window = window;
	rendererCI.shouldCopyToSwapchian = true;
	rendererCI.shouldDrawExternalUI = false;
	rendererCI.shouldPresent = true;
	Ref<Renderer> m_Renderer = CreateRef<Renderer>(&rendererCI);

	AllocatorManager::PrintMemoryBlockStatus();

	double yaw = 0;
	double pitch = 0;
	double roll = 0;

	double pos_x = 0, pos_y = 0;
	double last_pos_x = 0;
	double last_pos_y = 0;
	bool initMouse = true;
	core::Timer timer;

	bool windowResize = false;
	while (!window->Closed())
	{
		ref_cast<Animator>(sequencer)->Update();

		//Update Timer
		timer.Update();

		//Update Text
		auto& text = textEntity.GetComponent<TextComponent>().text;
		//text->UpdateLine("FPS: " + window->GetFPSString<uint32_t>(), 2, !(m_Renderer->GetFrameCount() % 60));

		//Update from Window
		if (window->Resized())
		{
			m_Renderer->ResizeRenderPipelineViewports(window->GetWidth(), window->GetHeight());
			text->m_CI.viewportWidth = (uint32_t)window->GetWidth();
			text->m_CI.viewportHeight = (uint32_t)window->GetHeight();
			text->Update();
			window->Resized() = false;
		}

		if (window->IsKeyPressed(GLFW_KEY_R))
		{
			m_Renderer->RecompileRenderPipelineShaders();
			ImageProcessing::RecompileRenderPipelineShaders();
			skyboxEntity.GetComponent<SkyboxComponent>().skybox->m_Generated = false;
		}

		if (window->IsKeyPressed(GLFW_KEY_T))
		{
			m_Renderer->ReloadTextures();
		}

		if (window->IsKeyPressed(GLFW_KEY_S) && window->IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
		{
			activeScene->SaveToFile();
		}

		if (window->IsKeyPressed(GLFW_KEY_L))
		{
			activeScene->UnloadNativeScriptLibrary();
			activeScene->LoadNativeScriptLibrary();
		}
		if (window->IsKeyPressed(GLFW_KEY_P))
		{
			activeScene->Play();
		}

		//Keyboard and Mouse input
		if (window->IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT))
		{
			window->GetMousePosition(pos_x, pos_y);
			pos_x /= window->GetWidth();
			pos_y /= window->GetHeight();
			pos_x = 2 * pos_x - 1.0;
			pos_y = 2 * pos_y - 1.0;

			double offset_pos_x = pos_x - last_pos_x;
			double offset_pos_y = -pos_y + last_pos_y;
			last_pos_x = pos_x;
			last_pos_y = pos_y;
			yaw += pi * offset_pos_x;
			pitch += pi / 2 * offset_pos_y;
			if (pitch > pi / 2)
				pitch = pi / 2;
			if (pitch < -pi / 2)
				pitch = -pi / 2;
		}

		//Camera Update
		auto& camera = cameraEntity.GetComponent<CameraComponent>().camera;
		if (window->IsKeyPressed(GLFW_KEY_D))
			camera->m_CI.transform.translation += Vec3::Normalise(camera->m_Right) * (float)timer;
		if (window->IsKeyPressed(GLFW_KEY_A))
			camera->m_CI.transform.translation -= Vec3::Normalise(camera->m_Right) * (float)timer;
		if (window->IsKeyPressed(GLFW_KEY_W))
			camera->m_CI.transform.translation += camera->m_Direction * (float)timer;
		if (window->IsKeyPressed(GLFW_KEY_S))
			camera->m_CI.transform.translation -= camera->m_Direction * (float)timer;

		double fov = 0.0;
		window->GetScrollPosition(fov);
		camera->m_CI.transform.orientation = Quat::FromEulerAngles(Vec3((float)pitch, (float)-yaw, (float)roll));
		camera->m_CI.perspectiveParams.horizonalFOV = DegToRad(90.0 - fov);
		camera->m_CI.perspectiveParams.aspectRatio = window->GetRatio();
		camera->m_CI.transform.translation.y = 1.0f;
		camera->Update();

		//Update Scene
		activeScene->OnUpdate(m_Renderer, timer);

		m_Renderer->SubmitRenderSurface(window->GetRenderSurface());
		m_Renderer->Execute();

		m_Renderer->Present(windowResize);
		window->Update();
		window->CalculateFPS();
	}
	window->GetContext()->DeviceWaitIdle();
}