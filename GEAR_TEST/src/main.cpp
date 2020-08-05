#include "gear_core.h"
#include <future>

using namespace gear;
using namespace graphics;
using namespace objects;

using namespace miru;
using namespace miru::crossplatform;

using namespace mars;

gear::Ref<graphics::Texture> LoadTexture(gear::Ref<Window> window, std::string filepath, std::string debugName)
{
	Texture::CreateInfo texCI;
	texCI.debugName = debugName.c_str();
	texCI.device = window->GetDevice();
	texCI.filepaths = { filepath };
	texCI.format = miru::crossplatform::Image::Format::R8G8B8A8_UNORM;
	texCI.type = miru::crossplatform::Image::Type::TYPE_2D;
	texCI.samples = miru::crossplatform::Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	return std::move(gear::CreateRef<Texture>(&texCI));
};

int main()
{
	system("BuildShaders.bat");
	system("CLS");

	Window::CreateInfo windowCI;
	windowCI.api = GraphicsAPI::API::VULKAN;
	windowCI.title = "GEAR_MIRU_TEST";
	windowCI.width = 1920;
	windowCI.height = 1080;
	windowCI.fullscreen = false;
	windowCI.vSync = true;
	windowCI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	windowCI.iconFilepath;
	gear::Ref<Window> window = gear::CreateRef<Window>(&windowCI);
	
	MemoryBlockManager::CreateInfo mbmCI;
	mbmCI.pContext = window->GetContext();
	mbmCI.defaultBlockSize = MemoryBlock::BlockSize::BLOCK_SIZE_128MB;
	MemoryBlockManager::Initialise(&mbmCI);
	
	/*Mesh::CreateInfo meshCI;
	meshCI.device = window.GetDevice();
	meshCI.debugName = "KagemitsuG4.fbx";
	meshCI.filepath = "res/obj/KagemitsuG4.fbx";
	gear::Ref<Mesh> kagemitsuG4Mesh = gear::CreateRef<Mesh>(&meshCI);

	Model::CreateInfo modelCI;
	modelCI.debugName = "KagemitsuG4";
	modelCI.device = window.GetDevice();
	modelCI.pMesh = kagemitsuG4Mesh;
	modelCI.transform.translation = Vec3(0, 0.9, -0.25);
	modelCI.transform.orientation = Quat(1, 0, 0, 0);
	modelCI.transform.scale = Vec3(0.01f, 0.01f, 0.01f);
	modelCI.renderPipelineName = "basic";
	gear::Ref<Model> kagemitsuG4 = gear::CreateRef<Model>(&modelCI);*/

	std::future<gear::Ref<graphics::Texture>> rustIronAlbedo = std::async(std::launch::async, LoadTexture, window, std::string("res/img/rustediron2-Unreal-Engine/rustediron2_basecolor.png"), std::string("UE4 Rust Iron: Albedo"));
	std::future<gear::Ref<graphics::Texture>> rustIronMetallic = std::async(std::launch::async, LoadTexture, window, std::string("res/img/rustediron2-Unreal-Engine/rustediron2_metallic.png"), std::string("UE4 Rust Iron: Metallic"));
	std::future<gear::Ref<graphics::Texture>> rustIronNormal = std::async(std::launch::async, LoadTexture, window, std::string("res/img/rustediron2-Unreal-Engine/rustediron2_normal.png"), std::string("UE4 Rust Iron: Roughness"));
	std::future<gear::Ref<graphics::Texture>> rustIronRoughness = std::async(std::launch::async, LoadTexture, window, std::string("res/img/rustediron2-Unreal-Engine/rustediron2_roughness.png"), std::string("UE4 Rust Iron: Albedo"));
	
	Material::CreateInfo matCI;
	matCI.debugName = "UE4 Rust Iron";
	matCI.device = window->GetDevice();
	matCI.pbrTextures = {
		{ Material::TextureType::NORMAL, rustIronNormal.get() },
		{ Material::TextureType::ALBEDO, rustIronAlbedo.get() },
		{ Material::TextureType::METALLIC, rustIronMetallic.get() },
		{ Material::TextureType::ROUGHNESS, rustIronRoughness.get() },
	};
	gear::Ref<Material> rustIronMaterial = gear::CreateRef<Material>(&matCI);

	Mesh::CreateInfo meshCI;
	meshCI.device = window->GetDevice();
	meshCI.debugName = "quad.obj";
	meshCI.filepath = "res/obj/quad.obj";
	gear::Ref<Mesh> quadMesh = gear::CreateRef<Mesh>(&meshCI);
	quadMesh->SetOverrideMaterial(0, rustIronMaterial);

	Model::CreateInfo modelCI;
	modelCI.debugName = "Quad";
	modelCI.device = window->GetDevice();
	modelCI.pMesh = quadMesh;
	modelCI.transform.translation = Vec3(0, 1.0f, -10.0f);
	modelCI.transform.orientation = Quat(1, 0, 0, 0);
	modelCI.transform.scale = Vec3(1.0f, 1.0f, 1.0f);
	modelCI.renderPipelineName = "basic";
	gear::Ref<Model> quad = gear::CreateRef<Model>(&modelCI);

	Light::CreateInfo lightCI;
	lightCI.debugName = "Main";
	lightCI.device = window->GetDevice();
	lightCI.type = Light::LightType::GEAR_LIGHT_POINT;
	lightCI.colour = Vec4(1.0f, 1.0f, 1.0f, 1.0f) * 25.0f;
	lightCI.position = Vec3(0.0f, 1.0f, 0.0f);
	lightCI.direction = Vec3(0.0f, 0.0f, -1.0f);
	gear::Ref<Light> light = gear::CreateRef<Light>(&lightCI);

	Camera::CreateInfo cameraCI;
	cameraCI.debugName = "Main";
	cameraCI.device = window->GetDevice();
	cameraCI.position = Vec3(0, 0, 0);
	cameraCI.orientation = Quat(1, 0, 0, 0);
	cameraCI.projectionType = Camera::ProjectionType::PERSPECTIVE;
	cameraCI.perspectiveParams = { DegToRad(90.0), window->GetRatio(), 0.01f, 3000.0f };
	cameraCI.flipX = false;
	cameraCI.flipY = false;
	gear::Ref<Camera> cam = gear::CreateRef<Camera>(&cameraCI);

	gear::Ref<Renderer> renderer = gear::CreateRef<Renderer>(window->GetContext());
	renderer->InitialiseRenderPipelines({"res/pipelines/basic.grpf.json", "res/pipelines/cube.grpf.json" }, (float)window->GetWidth(), (float)window->GetHeight(), window->GetRenderPass());

	MemoryBlockManager::PrintMemoryBlockStatus();
	
	double yaw = 0;
	double pitch = 0;
	double roll = 0;

	double pos_x = 0, pos_y = 0;
	double last_pos_x = 0;
	double last_pos_y = 0;
	bool initMouse = true;
	gear::core::Timer timer;

	bool windowResize = false;
	while (!window->Closed())
	{
		if (window->GetSwapchain()->m_Resized)
		{
			for (auto& renderPipeline : renderer->GetRenderPipelines())
			{
				renderPipeline.second->m_CI.viewportState.viewports = { { 0.0f, 0.0f, (float)window->GetWidth(), (float)window->GetHeight(), 0.0f, 1.0f } };
				renderPipeline.second->m_CI.viewportState.scissors = { { { 0, 0 },{ (uint32_t)window->GetWidth(), (uint32_t)window->GetHeight() } } };
				renderPipeline.second->Rebuild();
			}
			window->GetSwapchain()->m_Resized = false;
		}

		if (window->IsKeyPressed(GLFW_KEY_R))
		{
			window->GetContext()->DeviceWaitIdle();
			for (auto& renderPipeline : renderer->GetRenderPipelines())
				renderPipeline.second->RecompileShaders();
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
		if (window->IsKeyPressed(GLFW_KEY_D))
			cam->m_CI.position += Vec3::Normalise(cam->m_Right) * 5 * timer;
		if (window->IsKeyPressed(GLFW_KEY_A))
			cam->m_CI.position -= Vec3::Normalise(cam->m_Right) * 5 * timer;
		if (window->IsKeyPressed(GLFW_KEY_S))
			cam->m_CI.position += cam->m_Direction * 5 * timer;
		if (window->IsKeyPressed(GLFW_KEY_W))
			cam->m_CI.position -= cam->m_Direction * 5 * timer;
		
		double fov = 0.0;
		window->GetScrollPosition(fov);
		cam->m_CI.orientation = (Quat(pitch, {1, 0, 0}) * Quat(yaw, { 0, 1, 0 })).Normalise();
		cam->m_CI.perspectiveParams.horizonalFOV = DegToRad(90.0 - fov);
		cam->m_CI.perspectiveParams.aspectRatio = window->GetRatio();
		cam->m_CI.position.y = 1.0f;
		cam->Update();
		
		renderer->SubmitFramebuffer(window->GetFramebuffers());
		renderer->SubmitCamera(cam);
		renderer->SubmitLights({ light });
		renderer->SubmitModel(quad);
		renderer->Flush();

		renderer->Present(window->GetSwapchain(), windowResize);
		window->Update();

	}
	window->GetContext()->DeviceWaitIdle();
}