#include "gear_core.h"

#ifndef _DEBUG
	#if _WIN64
	#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
	#endif
#endif

using namespace gear;
using namespace graphics;
using namespace objects;

using namespace miru;
using namespace miru::crossplatform;

using namespace mars;

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
	Window window(&windowCI);

	MemoryBlockManager::CreateInfo mbmCI;
	mbmCI.pContext = window.GetContext();
	mbmCI.defaultBlockSize = MemoryBlock::BlockSize::BLOCK_SIZE_128MB;
	MemoryBlockManager::Initialise(&mbmCI);

	Texture::CreateInfo textureCI;
	textureCI.device =  window.GetDevice();
	textureCI.data = nullptr;
	textureCI.size = 0;
	textureCI.width = 0;
	textureCI.height = 0;
	textureCI.depth = 0;
	textureCI.format = Image::Format::R8G8B8A8_UNORM;
	textureCI.type = Image::Type::TYPE_2D;
	textureCI.samples  = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	
	textureCI.debugName = "GEAR_logo_dark";
	textureCI.filepaths = { "../Branding/GEAR_logo_dark.png" };
	gear::Ref<Texture> gear_logo = gear::CreateRef<Texture>(&textureCI);
	
	textureCI.debugName = "MIRU_logo";
	textureCI.filepaths = { "../GEAR_CORE/dep/MIRU/logo.png" };
	gear::Ref<Texture> miru_logo = gear::CreateRef<Texture>(&textureCI);
	
	textureCI.debugName = "StallTexture";
	textureCI.filepaths = { "res/img/stallTexture.png" };
	gear::Ref<Texture> stall_tex = gear::CreateRef<Texture>(&textureCI);
	
	textureCI.debugName = "WoodFloor";
	textureCI.filepaths = { "res/img/tileable_wood_texture_01_by_goodtextures-d31qde8.jpg" };
	gear::Ref<Texture> woodFloor = gear::CreateRef<Texture>(&textureCI);
	woodFloor->SetTileFactor(50.0f);
	woodFloor->SetAnisotrophicValue(16.0f);

	/*textureCI.filepaths = {
			"res/img/mp_arctic/arctic-ice_ft.tga",
			"res/img/mp_arctic/arctic-ice_bk.tga",
			"res/img/mp_arctic/arctic-ice_up.tga",
			"res/img/mp_arctic/arctic-ice_dn.tga",
			"res/img/mp_arctic/arctic-ice_rt.tga",
			"res/img/mp_arctic/arctic-ice_lf.tga",
	};*/
	textureCI.filepaths = {
			"res/img/galaxy/GalaxyTex_NegativeX_2048.tga", 
			"res/img/galaxy/GalaxyTex_PositiveX_2048.tga",
			"res/img/galaxy/GalaxyTex_NegativeY_2048.tga",
			"res/img/galaxy/GalaxyTex_PositiveY_2048.tga",
			"res/img/galaxy/GalaxyTex_NegativeZ_2048.tga",
			"res/img/galaxy/GalaxyTex_PositiveZ_2048.tga",
	};
	textureCI.debugName = "Skybox";
	textureCI.type = Image::Type::TYPE_CUBE;
	gear::Ref<Texture> skybox_cubemap = gear::CreateRef<Texture>(&textureCI);

	Mesh::CreateInfo meshCI;
	meshCI.device = window.GetDevice();
	meshCI.debugName = "quad";
	meshCI.filepath = "res/obj/quad.obj";
	gear::Ref<Mesh> quadMesh = gear::CreateRef<Mesh>(&meshCI);
	meshCI.debugName = "cube";
	meshCI.filepath = "res/obj/cube.obj";
	gear::Ref<Mesh> cubeMesh = gear::CreateRef<Mesh>(&meshCI);
	meshCI.debugName = "stall";
	meshCI.filepath = "res/obj/stall.obj";
	gear::Ref<Mesh> stallMesh = gear::CreateRef<Mesh>(&meshCI);

	Model::CreateInfo modelCI;
	modelCI.debugName = "GEAR_Quad";
	modelCI.device = window.GetDevice();
	modelCI.pMesh = quadMesh;
	modelCI.transform.translation = Vec3(-2.0f, 0.0f, -1.0f);
	modelCI.transform.orientation = Quat(1.0, 0.0, 0.0, 0.0);
	modelCI.transform.scale = Vec3(1.0f, 1.0f, 1.0f);
	modelCI.pTexture = gear_logo;
	modelCI.colour = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	modelCI.renderPipelineName = "basic";
	gear::Ref<Model> quad1 = gear::CreateRef<Model>(&modelCI);
	
	modelCI.debugName = "MIRU_Quad";
	modelCI.transform.translation = Vec3(+2.0f, 0.0f, -1.0f);
	modelCI.pTexture = miru_logo;
	gear::Ref<Model> quad2 = gear::CreateRef<Model>(&modelCI);

	modelCI.debugName = "Wood_Floor";
	modelCI.transform.translation = Vec3(0.0f, -2.0f, -2.0f);
	modelCI.transform.orientation = Quat(-pi/2, Vec3(1, 0, 0));
	modelCI.transform.scale = Vec3(500.0f, 500.0f, 1.0f);
	modelCI.pTexture = woodFloor;
	gear::Ref<Model> floor = gear::CreateRef<Model>(&modelCI);

	modelCI.debugName = "Stall";
	modelCI.pMesh = stallMesh;
	modelCI.transform.translation = Vec3(0.0f, -2.0f, -5.0f);
	modelCI.transform.orientation = Quat(pi, Vec3(0, 1, 0));
	modelCI.transform.scale = Vec3(1.0f, 1.0f, 1.0f);
	modelCI.pTexture = stall_tex;
	gear::Ref<Model> stall = gear::CreateRef<Model>(&modelCI);
	
	modelCI.debugName = "Skybox";
	modelCI.pMesh = cubeMesh;
	modelCI.transform.translation = Vec3(0.0f, 0.0f, 0.0f);
	modelCI.transform.orientation = Quat(1.0, 0.0, 0.0, 0.0);
	modelCI.transform.scale = Vec3(500.0f, 500.0f, 500.0f);
	modelCI.pTexture = skybox_cubemap;
	modelCI.renderPipelineName = "cube";
	gear::Ref<Model> skybox = gear::CreateRef<Model>(&modelCI);

	Light::CreateInfo lightCI;
	lightCI.debugName = "Main";
	lightCI.device = window.GetDevice();
	lightCI.type = Light::LightType::GEAR_LIGHT_POINT;
	lightCI.colour = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	lightCI.position = Vec3(0.0f, 0.0f, 0.0f);
	lightCI.direction = Vec3(0.0f, 0.0f, -1.0f);
	Light light(&lightCI);
	light.Specular(64.0f, 10.0f);
	light.Ambient(5.0f);
	light.Attenuation(0.007f, 0.0002f);
	light.SpotCone(DegToRad(45.0));

	Camera::CreateInfo cameraCI;
	cameraCI.debugName = "Main";
	cameraCI.device = window.GetDevice();
	cameraCI.position = Vec3(0, 0, 0);
	cameraCI.orientation = Quat(1, 0, 0, 0);
	cameraCI.projectionType = Camera::ProjectionType::PERSPECTIVE;
	cameraCI.perspectiveParams = { DegToRad(90.0), window.GetRatio(), 0.01f, 3000.0f };
	cameraCI.flipX = false;
	cameraCI.flipY = false;
	Camera cam(&cameraCI);

	Renderer renderer(window.GetContext());
	renderer.InitialiseRenderPipelines({"res/pipelines/basic.grpf.json", "res/pipelines/cube.grpf.json" }, (float)window.GetWidth(), (float)window.GetHeight(), window.GetRenderPass());

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
	while (!window.Closed())
	{
		if (window.GetSwapchain()->m_Resized)
		{
			for (auto& renderPipeline : renderer.GetRenderPipelines())
			{
				renderPipeline.second->m_CI.viewportState.viewports = { { 0.0f, 0.0f, (float)window.GetWidth(), (float)window.GetHeight(), 0.0f, 1.0f } };
				renderPipeline.second->m_CI.viewportState.scissors = { { { 0, 0 },{ (uint32_t)window.GetWidth(), (uint32_t)window.GetHeight() } } };
				renderPipeline.second->Rebuild();
			}
			window.GetSwapchain()->m_Resized = false;
		}

		if (window.IsKeyPressed(GLFW_KEY_R))
		{
			window.GetContext()->DeviceWaitIdle();
			for (auto& renderPipeline : renderer.GetRenderPipelines())
				renderPipeline.second->RecompileShaders();
		}

		//Keyboard and Mouse input
		if(window.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT))
		{
			window.GetMousePosition(pos_x, pos_y);
			pos_x /= window.GetWidth();
			pos_y /= window.GetHeight();
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
		if (window.IsKeyPressed(GLFW_KEY_D))
			cam.m_CI.position += Vec3::Normalise(cam.m_Right) * 5 * timer;
		if (window.IsKeyPressed(GLFW_KEY_A))	 
			cam.m_CI.position -= Vec3::Normalise(cam.m_Right) * 5 * timer;
		if (window.IsKeyPressed(GLFW_KEY_S))
			cam.m_CI.position += cam.m_Direction * 5 * timer;
		if (window.IsKeyPressed(GLFW_KEY_W))
			cam.m_CI.position -= cam.m_Direction * 5 * timer;
		
		double fov = 0.0;
		window.GetScrollPosition(fov);
		cam.m_CI.orientation = (Quat(pitch, {1, 0, 0}) * Quat(yaw, { 0, 1, 0 })).Normalise();
		cam.m_CI.perspectiveParams.horizonalFOV = DegToRad(90.0 - fov);
		cam.m_CI.perspectiveParams.aspectRatio = window.GetRatio();
		cam.m_CI.position.y = 1.0f;
		cam.Update();
		
		renderer.SubmitFramebuffer(window.GetFramebuffers());
		renderer.SubmitCamera(&cam);
		renderer.SubmitLights({ &light });
		renderer.Submit(quad1);
		renderer.Submit(quad2);
		renderer.Submit(stall);
		renderer.Submit(floor);
		renderer.Submit(skybox);
		renderer.Flush();

		renderer.Present(window.GetSwapchain(), windowResize);
		window.Update();

	}
	window.GetContext()->DeviceWaitIdle();
}