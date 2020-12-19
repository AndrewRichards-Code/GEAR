#include "gear_core.h"

using namespace gear;
using namespace audio;
using namespace core;
using namespace graphics;
using namespace objects;
using namespace scene;

using namespace miru;
using namespace miru::crossplatform;

using namespace mars;

int main()
{
#if 0
	AudioListener::CreateInfo listenerCI;
	listenerCI.audioAPI = audio::AudioListenerInterface::API::OPENAL;
	auto listener = gear::CreateRef<AudioListener>(&listenerCI);

	AudioSource::CreateInfo rainbowRaodCI;
	rainbowRaodCI.filepath = "res/wav/Rainbow Road.wav";
	rainbowRaodCI.pAudioListener = listener;
	auto rainbowRoad = gear::CreateRef<AudioSource>(&rainbowRaodCI);
	rainbowRoad->SetPitch(0.0f);
	rainbowRoad->SetVolume(0.0f);
	rainbowRoad->Stream();
#endif
	Scene::CreateInfo sceneCI;
	sceneCI.debugName = "GEAR_TEST_Main_Scene";
	sceneCI.filepath = "res/scenes/current_scene.gsf.json";
	sceneCI.nativeScriptDir = "res/scripts/";
	gear::Ref<Scene> activeScene = gear::CreateRef<Scene>(&sceneCI);

	Window::CreateInfo windowCI;
	windowCI.api = GraphicsAPI::API::D3D12;
	//windowCI.api = GraphicsAPI::API::VULKAN;
	windowCI.title = "GEAR_TEST";
	windowCI.width = 1920;
	windowCI.height = 1080;
	windowCI.fullscreen = false;
	windowCI.vSync = true;
	windowCI.samples = Image::SampleCountBit::SAMPLE_COUNT_2_BIT;
	windowCI.graphicsDebugger = debug::GraphicsDebugger::DebuggerType::NONE;
	gear::Ref<Window> window = gear::CreateRef<Window>(&windowCI);

	AllocatorManager::CreateInfo mbmCI;
	mbmCI.pContext = window->GetContext();
	mbmCI.defaultBlockSize = Allocator::BlockSize::BLOCK_SIZE_128MB;
	AllocatorManager::Initialise(&mbmCI);
	
	Skybox::CreateInfo skyboxCI;
	skyboxCI.debugName = "Skybox-HDR";
	skyboxCI.device = window->GetDevice();
	skyboxCI.filepaths = { "res/img/snowy_park_01_2k.hdr" };
	skyboxCI.generatedCubemapSize = 1024;
	skyboxCI.transform.translation = Vec3(0, 0, 0);
	skyboxCI.transform.orientation = Quat(1, 0, 0, 0);
	skyboxCI.transform.scale = Vec3(500.0f, 500.0f, 500.0f);
	Entity skybox = activeScene->CreateEntity();
	skybox.AddComponent<SkyboxComponent>(std::move(gear::CreateRef<Skybox>(&skyboxCI)));

	auto LoadTexture = [](gear::Ref<Window> window, const std::string& filepath, const std::string& debugName) -> gear::Ref<graphics::Texture>
	{
		Texture::CreateInfo texCI;
		texCI.debugName = debugName.c_str();
		texCI.device = window->GetDevice();
		texCI.dataType = Texture::DataType::FILE;
		texCI.file.filepaths = &filepath;
		texCI.file.count = 1;
		texCI.mipLevels = GEAR_TEXTURE_MAX_MIP_LEVEL;
		texCI.arrayLayers = 1;
		texCI.type = miru::crossplatform::Image::Type::TYPE_2D;
		texCI.format = miru::crossplatform::Image::Format::R8G8B8A8_UNORM;
		texCI.samples = miru::crossplatform::Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
		texCI.usage = miru::crossplatform::Image::UsageBit(0);
		texCI.generateMipMaps = true;
		return std::move(gear::CreateRef<Texture>(&texCI));
	};

	//std::future<gear::Ref<graphics::Texture>> rustIronAlbedo = std::async(std::launch::async, LoadTexture, window, std::string("res/img/rustediron2-Unreal-Engine/rustediron2_basecolor.png"), std::string("UE4 Rust Iron: Albedo"));
	//std::future<gear::Ref<graphics::Texture>> rustIronMetallic = std::async(std::launch::async, LoadTexture, window, std::string("res/img/rustediron2-Unreal-Engine/rustediron2_metallic.png"), std::string("UE4 Rust Iron: Metallic"));
	//std::future<gear::Ref<graphics::Texture>> rustIronNormal = std::async(std::launch::async, LoadTexture, window, std::string("res/img/rustediron2-Unreal-Engine/rustediron2_normal.png"), std::string("UE4 Rust Iron: Normal"));
	//std::future<gear::Ref<graphics::Texture>> rustIronRoughness = std::async(std::launch::async, LoadTexture, window, std::string("res/img/rustediron2-Unreal-Engine/rustediron2_roughness.png"), std::string("UE4 Rust Iron: Roughness"));

	//std::future<gear::Ref<graphics::Texture>> slateAlbedo = std::async(std::launch::async, LoadTexture, window, std::string("res/img/slate2-tiled-ue4/slate2-tiled-albedo2.png"), std::string("UE4 Slate: Albedo"));
	//std::future<gear::Ref<graphics::Texture>> slateMetallic = std::async(std::launch::async, LoadTexture, window, std::string("res/img/slate2-tiled-ue4/slate2-tiled-metalness.png"), std::string("UE4 Slate: Metallic"));
	//std::future<gear::Ref<graphics::Texture>> slateNormal = std::async(std::launch::async, LoadTexture, window, std::string("res/img/slate2-tiled-ue4/slate2-tiled-normal3-UE4.png"), std::string("UE4 Slate: Normal"));
	//std::future<gear::Ref<graphics::Texture>> slateRoughness = std::async(std::launch::async, LoadTexture, window, std::string("res/img/slate2-tiled-ue4/slate2-tiled-rough.png"), std::string("UE4 Slate: Roughness"));
	//std::future<gear::Ref<graphics::Texture>> slateAO = std::async(std::launch::async, LoadTexture, window, std::string("res/img/slate2-tiled-ue4/slate2-tiled-ao.png"), std::string("UE4 Slate: AO"));
	
	Material::CreateInfo matCI;
	/*matCI.debugName = "UE4 Rust Iron";
	matCI.device = window->GetDevice();
	matCI.pbrTextures = {
		{ Material::TextureType::NORMAL, rustIronNormal.get() },
		{ Material::TextureType::ALBEDO, rustIronAlbedo.get() },
		{ Material::TextureType::METALLIC, rustIronMetallic.get() },
		{ Material::TextureType::ROUGHNESS, rustIronRoughness.get() }
	};
	gear::Ref<Material> rustIronMaterial = gear::CreateRef<Material>(&matCI);*/
	
	/*matCI.debugName = "UE4 Slate";
	matCI.device = window->GetDevice();
	matCI.pbrTextures = {
		{ Material::TextureType::NORMAL, slateNormal.get() },
		{ Material::TextureType::ALBEDO, slateAlbedo.get() },
		{ Material::TextureType::METALLIC, slateMetallic.get() },
		{ Material::TextureType::ROUGHNESS, slateRoughness.get() },
		{ Material::TextureType::AMBIENT_OCCLUSION, slateAO.get()}
	};
	gear::Ref<Material> slateMaterial = gear::CreateRef<Material>(&matCI);*/

	Mesh::CreateInfo meshCI;
	/*meshCI.debugName = "quad.fbx";
	meshCI.device = window->GetDevice();
	meshCI.filepath = "res/obj/quad.fbx";
	gear::Ref<Mesh> quadMesh = gear::CreateRef<Mesh>(&meshCI);
	quadMesh->SetOverrideMaterial(0, slateMaterial);*/

	/*meshCI.debugName = "sphere.fbx";
	meshCI.device = window->GetDevice();
	meshCI.filepath = "res/obj/sphere.fbx";
	gear::Ref<Mesh> sphereMesh = gear::CreateRef<Mesh>(&meshCI);
	sphereMesh->SetOverrideMaterial(0, rustIronMaterial);*/

	/*meshCI.debugName = "KagemitsuG4.fbx";
	meshCI.device = window->GetDevice();
	meshCI.filepath = "res/obj/KagemitsuG4.fbx";
	gear::Ref<Mesh> kagemitsuG4Mesh = gear::CreateRef<Mesh>(&meshCI);*/

	/*modelCI.debugName = "Quad";
	modelCI.device = window->GetDevice();
	modelCI.pMesh = quadMesh;
	modelCI.materialTextureScaling = Vec2(100.0f, 100.0f);
	modelCI.transform.translation = Vec3(0, 0, 0);
	modelCI.transform.orientation = Quat(sqrt(2)/2, -sqrt(2)/2, 0, 0);
	modelCI.transform.scale = Vec3(100.0f, 100.0f, 100.0f);
	modelCI.renderPipelineName = "PBROpaque";
	Entity quad = activeScene->CreateEntity();
	quad.AddComponent<ModelComponent>(std::move(gear::CreateRef<Model>(&modelCI)));*/

	Model::CreateInfo modelCI;
	/*modelCI.debugName = "KagemitsuG4";
	modelCI.device = window->GetDevice();
	modelCI.pMesh = kagemitsuG4Mesh;
	modelCI.transform.translation = Vec3(0.0, 0.825, -0.1);
	modelCI.transform.orientation = Quat(cos(DegToRad(45.0)), sin(DegToRad(45.0)), 0, 0);
	modelCI.transform.scale = Vec3(1.0f, 1.0f, 1.0f);
	modelCI.renderPipelineName = "PBROpaque";
	Entity KagemitsuG4 = activeScene->CreateEntity();
	KagemitsuG4.AddComponent<ModelComponent>(gear::CreateRef<Model>(&modelCI));*/

	uint32_t gridSize = 5;
	for (uint32_t i = 0; i < gridSize; i++)
	{
		for (uint32_t j = 0; j < gridSize; j++)
		{
			float pos_x =(2.0f / float(gridSize)) * float(int(i) - int(gridSize / 2));
			float pos_y =(2.0f / float(gridSize)) * float(int(j) - int(gridSize / 2)) + 1.0f;

			matCI.debugName = "Test Material_" + std::to_string(i) + "_" + std::to_string(j);
			matCI.device = window->GetDevice();
			matCI.pbrConstants.fresnel = Vec4(1, 1, 1, 1);
			matCI.pbrConstants.albedo = Vec4(1, 0, 0, 1);
			matCI.pbrConstants.metallic = (float(i) + 0.5f) / float(gridSize);
			matCI.pbrConstants.roughness = (float(j) + 0.5f) / float(gridSize);
			matCI.pbrConstants.ambientOcclusion = 0.0f;
			matCI.pbrConstants.emissive = Vec4(0, 0, 0, 1);
			gear::Ref<Material> testMaterial = gear::CreateRef<Material>(&matCI);

			meshCI.debugName = "sphere.fbx";
			meshCI.device = window->GetDevice();
			meshCI.filepath = "res/obj/sphere.fbx";
			gear::Ref<Mesh> sphereMesh = gear::CreateRef<Mesh>(&meshCI);
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
			sphere.AddComponent<ModelComponent>(std::move(gear::CreateRef<Model>(&modelCI)));
		}
	}

	Light::CreateInfo lightCI;
	lightCI.debugName = "Main";
	lightCI.device = window->GetDevice();
	lightCI.type = Light::LightType::POINT;
	lightCI.colour = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	lightCI.transform.translation = Vec3(0.0f, 2.0f, 0.0f);
	lightCI.transform.orientation = Quat(1, 0, 0, 0);
	lightCI.transform.scale = Vec3(1.0f, 1.0f, 1.0f);
	Entity light = activeScene->CreateEntity();
	light.AddComponent<LightComponent>(std::move(gear::CreateRef<Light>(&lightCI)));

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
	Entity cameraEnitity = activeScene->CreateEntity();
	cameraEnitity.AddComponent<CameraComponent>(std::move(gear::CreateRef<Camera>(&cameraCI)));
	cameraEnitity.AddComponent<NativeScriptComponent>("TestScript");

	gear::Ref<Renderer> m_Renderer = gear::CreateRef<Renderer>(window->GetContext());
	m_Renderer->InitialiseRenderPipelines({"res/pipelines/PBROpaque.grpf.json", "res/pipelines/Cube.grpf.json" }, (float)window->GetWidth(), (float)window->GetHeight(), window->GetCreateInfo().samples, window->GetRenderPass());

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
		//Update Timer
		timer.Update();

		//Update from Window
		if (window->Resized())
		{
			m_Renderer->ResizeRenderPipelineViewports((uint32_t)window->GetWidth(), (uint32_t)window->GetHeight());
			window->Resized() = false;
		}

		if (window->IsKeyPressed(GLFW_KEY_R))
		{
			m_Renderer->RecompileRenderPipelineShaders();
			ImageProcessing::RecompileRenderPipelineShaders();
			skybox.GetComponent<SkyboxComponent>().skybox->m_Generated = false;
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
		if(window->IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT))
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
			pitch += pi/2 * offset_pos_y;
			if (pitch > pi / 2)
				pitch = pi / 2;
			if (pitch < -pi / 2)
				pitch = -pi / 2;
		}

		//Camera Update
		auto& camera = cameraEnitity.GetComponent<CameraComponent>().camera;
		if (window->IsKeyPressed(GLFW_KEY_D))
			camera->m_CI.transform.translation += Vec3::Normalise(camera->m_Right) * (float)timer;
		if (window->IsKeyPressed(GLFW_KEY_A))
			camera->m_CI.transform.translation -= Vec3::Normalise(camera->m_Right) * (float)timer;
		if (window->IsKeyPressed(GLFW_KEY_W))
			camera->m_CI.transform.translation += camera->m_Direction * (float)timer;
		if (window->IsKeyPressed(GLFW_KEY_S))
			camera->m_CI.transform.translation -= camera->m_Direction * (float)timer;

		/*if (window->IsKeyPressed(GLFW_KEY_L))
			yaw += DegToRad(10.0);
		if (window->IsKeyPressed(GLFW_KEY_J))
			yaw -= DegToRad(10.0);
		if (window->IsKeyPressed(GLFW_KEY_I))
			pitch += DegToRad(10.0);
		if (window->IsKeyPressed(GLFW_KEY_K))
			pitch -= DegToRad(10.0);*/

		double fov = 0.0;
		window->GetScrollPosition(fov);
		camera->m_CI.transform.orientation = Quat(pitch, {1, 0, 0}) * Quat(yaw, { 0, 1, 0 });
		camera->m_CI.perspectiveParams.horizonalFOV = DegToRad(90.0 - fov);
		camera->m_CI.perspectiveParams.aspectRatio = window->GetRatio();
		camera->m_CI.transform.translation.y = 1.0f;
		camera->Update();
		
		//Update Scene
		activeScene->OnUpdate(m_Renderer, timer);

		m_Renderer->SubmitFramebuffer(window->GetFramebuffers());
		m_Renderer->Upload(true, false, true, false);
		m_Renderer->Flush();

		m_Renderer->Present(window->GetSwapchain(), windowResize);
		window->Update();
	}
	window->GetContext()->DeviceWaitIdle();
}