#include "GEAR_TEST.h"
#include "gear_core.h"

using namespace gear;
using namespace animation;
using namespace audio;
using namespace core;
using namespace graphics;
using namespace rendering;
using namespace objects;
using namespace scene;

using namespace miru;
using namespace base;

using namespace mars;

Ref<Application> CreateApplication(int argc, char** argv)
{
	ApplicationContext::CreateInfo applicationCI;
	applicationCI.applicationName = "GEAR_TEST";
	applicationCI.extensions = ApplicationContext::DefaultExtensions();
	applicationCI.commandLineOptions = CommandLineOptions::GetCommandLineOptions(argc, argv).SetWorkingDirectory();

	return CreateRef<GEAR_TEST>(ApplicationContext(&applicationCI));
}

GEAR_TEST::GEAR_TEST(const ApplicationContext& context)
	:Application(context) {}

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
	sceneCI.nativeScriptDir = "res/scripts/";
	Ref<Scene> activeScene = CreateRef<Scene>(&sceneCI);

	Window::CreateInfo windowCI;
	windowCI.applicationContext = m_ApplicationContext;
	windowCI.width = 1920;
	windowCI.height = 1080;
	windowCI.fullscreen = false;
	windowCI.fullscreenMonitorIndex = 1;
	windowCI.maximised = false;
	windowCI.vSync = true;
	windowCI.samples = Image::SampleCountBit::SAMPLE_COUNT_2_BIT;
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
	text->AddLine(font, window->GetDeviceName(), { 0, render_doc_offset + 0 * textRowHeight }, float4(1.0f, 1.0f, 1.0f, 1.0f));
	text->AddLine(font, window->GetGraphicsAPIVersion(), { 0, render_doc_offset + 1 * textRowHeight }, GraphicsAPI::IsD3D12() ? float4(0.0f, 1.0f, 0.0f, 1.0) : float4(1.0f, 0.0f, 0.0f, 1.0f));
	text->AddLine(font, "FPS: " + window->GetFPSString<uint32_t>(), { 0, render_doc_offset + 2 * textRowHeight }, float4(1.0f, 1.0f, 1.0f, 1.0f));
	text->AddLine(font, "MSAA: " + window->GetAntiAliasingValue() + "x", { 0, render_doc_offset + 3 * textRowHeight }, float4(1.0f, 1.0f, 1.0f, 1.0f));

	Skybox::CreateInfo skyboxCI;
	skyboxCI.debugName = "Skybox-HDR";
	skyboxCI.device = window->GetDevice();
	//skyboxCI.filepaths = { "res/img/snowy_park_01_2k.hdr" };
	skyboxCI.filepath = "res/img/kloppenheim_06_2k.hdr";
	skyboxCI.generatedCubemapSize = 1024;
	Entity skyboxEntity = activeScene->CreateEntity();
	skyboxEntity.AddComponent<SkyboxComponent>(CreateRef<Skybox>(&skyboxCI));
	skyboxEntity.GetComponent<TransformComponent>().transform.translation = float3(0, 0, 0);
	skyboxEntity.GetComponent<TransformComponent>().transform.orientation = Quaternion(1, 0, 0, 0);
	skyboxEntity.GetComponent<TransformComponent>().transform.scale = float3(500.0f, 500.0f, 500.0f);

	auto LoadTexture = [](Ref<Window> window, const std::string& filepath, const std::string& debugName, bool linear = false) -> Ref<graphics::Texture>
	{
		Texture::CreateInfo texCI;
		texCI.debugName = debugName.c_str();
		texCI.device = window->GetDevice();
		texCI.dataType = Texture::DataType::FILE;
		texCI.file.filepaths = { filepath };
		texCI.mipLevels = Texture::MaxMipLevel;
		texCI.arrayLayers = 1;
		texCI.type = Image::Type::TYPE_2D;
		texCI.format = linear ? Image::Format::R32G32B32A32_SFLOAT : Image::Format::R8G8B8A8_UNORM;
		texCI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
		texCI.usage = Image::UsageBit(0);
		texCI.generateMipMaps = true;
		texCI.gammaSpace = linear ? GammaSpace::LINEAR : GammaSpace::SRGB;
		return CreateRef<Texture>(&texCI);
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
	matCI.pbrConstants = UniformBufferStructures::DefaultPBRConstants();
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
	modelCI.renderPipelineName = "PBROpaque";
	Entity drone = activeScene->CreateEntity();
	drone.AddComponent<ModelComponent>(CreateRef<Model>(&modelCI));
	drone.GetComponent<TransformComponent>().transform.translation = float3(0.0f, 0.75f, 1.5f);
	drone.GetComponent<TransformComponent>().transform.orientation = Quaternion(sqrt(2) / 2, sqrt(2) / 2, 0, 0);
	drone.GetComponent<TransformComponent>().transform.scale = float3(0.01f, 0.01f, 0.01f);

	Light::CreateInfo lightCI;
	lightCI.debugName = "Main";
	lightCI.device = window->GetDevice();
	lightCI.type = Light::Type::POINT;
	lightCI.colour = float4(1.0f, 1.0f, 1.0f, 1.0f);
	Entity lightEntity = activeScene->CreateEntity();
	lightEntity.AddComponent<LightComponent>(CreateRef<Light>(&lightCI));
	lightEntity.GetComponent<TransformComponent>().transform.translation = float3(0.0f, 2.0f, 0.0f);
	lightEntity.GetComponent<TransformComponent>().transform.orientation = Quaternion(1, 0, 0, 0);
	lightEntity.GetComponent<TransformComponent>().transform.scale = float3(1.0f, 1.0f, 1.0f);

	Camera::CreateInfo cameraCI;
	cameraCI.debugName = "Main";
	cameraCI.device = window->GetDevice();
	cameraCI.projectionType = Camera::ProjectionType::PERSPECTIVE;
	cameraCI.perspectiveParams = { DegToRad(90.0), window->GetRatio(), 0.01f, 3000.0f };
	Entity cameraEntity = activeScene->CreateEntity();
	cameraEntity.AddComponent<CameraComponent>(CreateRef<Camera>(&cameraCI));
	cameraEntity.GetComponent<TransformComponent>().transform.translation = float3(0, 1, 0);
	cameraEntity.GetComponent<TransformComponent>().transform.orientation = Quaternion(1, 0, 0, 0);
	cameraEntity.GetComponent<TransformComponent>().transform.scale = float3(1.0f, 1.0f, 1.0f);
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
			text->m_CI.viewportWidth = (uint32_t)window->GetWidth();
			text->m_CI.viewportHeight = (uint32_t)window->GetHeight();
			text->Update(Transform());
			window->Resized() = false;
		}

		if (window->IsKeyPressed(GLFW_KEY_R))
		{
			m_Renderer->RecompileRenderPipelineShaders();
			skyboxEntity.GetComponent<SkyboxComponent>().skybox->m_Generated = false;
		}

		if (window->IsKeyPressed(GLFW_KEY_T))
		{
			m_Renderer->ReloadTextures();
		}

		if (window->IsKeyPressed(GLFW_KEY_S) && window->IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
		{
			activeScene->SaveToFile("res/scenes/current_scene.gsf");
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
		auto& cameraTransform = cameraEntity.GetComponent<TransformComponent>().transform;
		if (window->IsKeyPressed(GLFW_KEY_D))
			cameraTransform.translation += camera->m_Right.Normalise() * (float)timer;
		if (window->IsKeyPressed(GLFW_KEY_A))
			cameraTransform.translation -= camera->m_Right.Normalise() * (float)timer;
		if (window->IsKeyPressed(GLFW_KEY_W))
			cameraTransform.translation += camera->m_Direction * (float)timer;
		if (window->IsKeyPressed(GLFW_KEY_S))
			cameraTransform.translation -= camera->m_Direction * (float)timer;

		double fov = 0.0;
		window->GetScrollPosition(fov);
		cameraTransform.orientation = Quaternion::FromEulerAngles(float3((float)pitch, (float)-yaw, (float)roll));
		camera->m_CI.perspectiveParams.horizonalFOV = DegToRad(90.0 - fov);
		camera->m_CI.perspectiveParams.aspectRatio = window->GetRatio();
		cameraTransform.translation.y = 1.0f;

		//Update Scene
		activeScene->OnUpdate(m_Renderer, timer);

		m_Renderer->SubmitRenderSurface(window->GetRenderSurface());
		m_Renderer->Execute();

		window->Update();
		window->CalculateFPS();
	}
	window->GetContext()->DeviceWaitIdle();
	AllocatorManager::Uninitialise();
}