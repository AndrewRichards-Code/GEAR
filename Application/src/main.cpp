#include "gear_core.h"

using namespace gear;
using namespace graphics;
using namespace objects;

using namespace miru;
using namespace miru::crossplatform;

using namespace mars;

void InitialisePipelines(const Window& window, std::map<std::string, gear::Ref<graphics::Pipeline>>& pipelines)
{
	std::string mscDirectory = "../../MIRU/MIRU_SHADER_COMPILER/exe/x64/";
	#if _DEBUG
	mscDirectory += "Debug";
	#else
	mscDirectory += "Release";
	#endif

	std::string binaryFilepath, hlslFilePath, debugName, binaryDir;
	auto get_shader_filepath_strings = [&](const std::string& _binaryFilepath) -> void
	{
		binaryFilepath = _binaryFilepath;
		hlslFilePath = binaryFilepath;
		hlslFilePath.replace(hlslFilePath.find("bin"), 3, "HLSL");
		hlslFilePath.replace(hlslFilePath.find("spv"), 3, "hlsl");
		debugName = binaryFilepath.substr(binaryFilepath.find_last_of('/') + 1);
		binaryDir = binaryFilepath.substr(0, binaryFilepath.find_last_of('/'));
	};

	//TODO: Fix memory leak from _strdup()

	//Basic
	graphics::Pipeline::CreateInfo basicPipelineCI;
	basicPipelineCI.shaderCreateInfo.resize(2);

	get_shader_filepath_strings("res/shaders/bin/basic.vert.spv");
	basicPipelineCI.shaderCreateInfo[0].debugName = _strdup(debugName.c_str());	
	basicPipelineCI.shaderCreateInfo[0].device = window.GetDevice();
	basicPipelineCI.shaderCreateInfo[0].stage = Shader::StageBit::VERTEX_BIT;
	basicPipelineCI.shaderCreateInfo[0].entryPoint = "main";
	basicPipelineCI.shaderCreateInfo[0].binaryFilepath = _strdup(binaryFilepath.c_str());
	basicPipelineCI.shaderCreateInfo[0].binaryCode = {};
	basicPipelineCI.shaderCreateInfo[0].recompileArguments = {
		_strdup(mscDirectory.c_str()), _strdup(hlslFilePath.c_str()), _strdup(binaryDir.c_str()), { "../../MIRU/MIRU_SHADER_COMPILER/shaders/includes" },
		nullptr, "6_4", {}, true, true, nullptr, nullptr, nullptr, false, false };

	get_shader_filepath_strings("res/shaders/bin/basic.frag.spv");
	basicPipelineCI.shaderCreateInfo[1].debugName = _strdup(debugName.c_str());
	basicPipelineCI.shaderCreateInfo[1].device = window.GetDevice();
	basicPipelineCI.shaderCreateInfo[1].stage = Shader::StageBit::FRAGMENT_BIT;
	basicPipelineCI.shaderCreateInfo[1].entryPoint = "main";
	basicPipelineCI.shaderCreateInfo[1].binaryFilepath = _strdup(binaryFilepath.c_str());
	basicPipelineCI.shaderCreateInfo[1].binaryCode = {};
	basicPipelineCI.shaderCreateInfo[1].recompileArguments = {
		_strdup(mscDirectory.c_str()), _strdup(hlslFilePath.c_str()), _strdup(binaryDir.c_str()), { "../../MIRU/MIRU_SHADER_COMPILER/shaders/includes" },
		nullptr, "6_4", {}, true, true, nullptr, nullptr, nullptr, false, false };

	basicPipelineCI.viewportState.viewports = { { 0.0f, 0.0f, (float)window.GetWidth(), (float)window.GetHeight(), 0.0f, 1.0f } };
	basicPipelineCI.viewportState.scissors = { { { 0, 0 },{ (uint32_t)window.GetWidth(), (uint32_t)window.GetHeight() } } };
	basicPipelineCI.rasterisationState = { false, false, PolygonMode::FILL, CullModeBit::BACK_BIT, FrontFace::COUNTER_CLOCKWISE, false, 0.0f, 0.0, 0.0f, 1.0f };
	basicPipelineCI.multisampleState = { Image::SampleCountBit::SAMPLE_COUNT_1_BIT, false, 1.0f, false, false };
	basicPipelineCI.depthStencilState = { true, true, CompareOp::LESS, false, false, {}, {}, 0.0f, 1.0f };
	basicPipelineCI.colourBlendState = { false, LogicOp::COPY, { {true, BlendFactor::SRC_ALPHA, BlendFactor::ONE_MINUS_SRC_ALPHA, BlendOp::ADD,
		BlendFactor::ONE, BlendFactor::ZERO, BlendOp::ADD, (ColourComponentBit)15 } }, { 0.0f, 0.0f, 0.0f, 0.0f } };
	basicPipelineCI.renderPass = window.GetRenderPass();
	basicPipelineCI.subpassIndex = 0;
	pipelines["basic"] = gear::CreateRef<graphics::Pipeline>(&basicPipelineCI);

	//Cube
	graphics::Pipeline::CreateInfo cubePipelineCI;
	cubePipelineCI.shaderCreateInfo.resize(2);
	
	get_shader_filepath_strings("res/shaders/bin/cube.vert.spv");
	cubePipelineCI.shaderCreateInfo[0].debugName = _strdup(debugName.c_str());
	cubePipelineCI.shaderCreateInfo[0].device = window.GetDevice();
	cubePipelineCI.shaderCreateInfo[0].stage = Shader::StageBit::VERTEX_BIT;
	cubePipelineCI.shaderCreateInfo[0].entryPoint = "main";
	cubePipelineCI.shaderCreateInfo[0].binaryFilepath = _strdup(binaryFilepath.c_str());
	cubePipelineCI.shaderCreateInfo[0].binaryCode = {};
	cubePipelineCI.shaderCreateInfo[0].recompileArguments = {
		_strdup(mscDirectory.c_str()), _strdup(hlslFilePath.c_str()), _strdup(binaryDir.c_str()), { "../../MIRU/MIRU_SHADER_COMPILER/shaders/includes" },
		nullptr, "6_4", {}, true, true, nullptr, nullptr, nullptr, false, false };

	get_shader_filepath_strings("res/shaders/bin/cube.frag.spv");
	cubePipelineCI.shaderCreateInfo[1].debugName = _strdup(debugName.c_str());
	cubePipelineCI.shaderCreateInfo[1].device = window.GetDevice();
	cubePipelineCI.shaderCreateInfo[1].stage = Shader::StageBit::FRAGMENT_BIT;
	cubePipelineCI.shaderCreateInfo[1].entryPoint = "main";
	cubePipelineCI.shaderCreateInfo[1].binaryFilepath = _strdup(binaryFilepath.c_str());
	cubePipelineCI.shaderCreateInfo[1].binaryCode = {};
	cubePipelineCI.shaderCreateInfo[1].recompileArguments = {
		_strdup(mscDirectory.c_str()), _strdup(hlslFilePath.c_str()), _strdup(binaryDir.c_str()), { "../../MIRU/MIRU_SHADER_COMPILER/shaders/includes" },
		nullptr, "6_4", {}, true, true, nullptr, nullptr, nullptr, false, false };

	cubePipelineCI.viewportState.viewports = { { 0.0f, 0.0f, (float)window.GetWidth(), (float)window.GetHeight(), 0.0f, 1.0f } };
	cubePipelineCI.viewportState.scissors = { { { 0, 0 },{ (uint32_t)window.GetWidth(), (uint32_t)window.GetHeight() } } };
	cubePipelineCI.rasterisationState = { false, false, PolygonMode::FILL, CullModeBit::NONE_BIT, FrontFace::COUNTER_CLOCKWISE, false, 0.0f, 0.0, 0.0f, 1.0f };
	cubePipelineCI.multisampleState = { Image::SampleCountBit::SAMPLE_COUNT_1_BIT, false, 1.0f, false, false };
	cubePipelineCI.depthStencilState = { true, true, CompareOp::LESS, false, false, {}, {}, 0.0f, 1.0f };
	cubePipelineCI.colourBlendState = { false, LogicOp::COPY, { {true, BlendFactor::SRC_ALPHA, BlendFactor::ONE_MINUS_SRC_ALPHA, BlendOp::ADD,
		BlendFactor::ONE, BlendFactor::ZERO, BlendOp::ADD, (ColourComponentBit)15 } }, { 0.0f, 0.0f, 0.0f, 0.0f } };
	cubePipelineCI.renderPass = window.GetRenderPass();
	cubePipelineCI.subpassIndex = 0;
	pipelines["cube"] = gear::CreateRef<graphics::Pipeline>(&cubePipelineCI);
}

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
	windowCI.vSync = false;
	windowCI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	windowCI.iconFilepath;
	Window window(&windowCI);

	//Font font(window.GetDevice(), "res/font/Source_Code_Pro/SourceCodePro-Regular.ttf", 75, window.GetWidth(), window.GetHeight(), window.GetRatio());

	std::map<std::string, gear::Ref<graphics::Pipeline>> pipelines;
	InitialisePipelines(window, pipelines);

	Texture::SetContext(window.GetContext());
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
	
	textureCI.filepaths = { "res/gear_core/GEAR_logo_square.png" };
	gear::Ref<Texture> gear_logo = gear::CreateRef<Texture>(&textureCI);
	
	textureCI.filepaths = { "C:/Users/Andrew/source/repos/MIRU/logo.png" };
	gear::Ref<Texture> miru_logo = gear::CreateRef<Texture>(&textureCI);
	
	textureCI.filepaths = { "res/img/stallTexture.png" };
	gear::Ref<Texture> stall_tex = gear::CreateRef<Texture>(&textureCI);
	
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
			"res/img/galaxy/GalaxyTex_NegativeX.tga", 
			"res/img/galaxy/GalaxyTex_PositiveX.tga",
			"res/img/galaxy/GalaxyTex_NegativeY.tga",
			"res/img/galaxy/GalaxyTex_PositiveY.tga",
			"res/img/galaxy/GalaxyTex_NegativeZ.tga",
			"res/img/galaxy/GalaxyTex_PositiveZ.tga",
	};
	textureCI.type = Image::Type::TYPE_CUBE;
	gear::Ref<Texture> skybox_cubemap = gear::CreateRef<Texture>(&textureCI);

	Mesh::SetContext(window.GetContext());
	Mesh::CreateInfo meshCI;
	meshCI.device = window.GetDevice();
	meshCI.filepath = "res/obj/quad.obj";
	gear::Ref<Mesh> quadMesh = gear::CreateRef<Mesh>(&meshCI);
	meshCI.filepath = "res/obj/cube.obj";
	gear::Ref<Mesh> cubeMesh = gear::CreateRef<Mesh>(&meshCI);
	meshCI.filepath = "res/obj/stall.obj";
	gear::Ref<Mesh> stallMesh = gear::CreateRef<Mesh>(&meshCI);

	Model::SetContext(window.GetContext());
	Model::CreateInfo modelCI;
	modelCI.device = window.GetDevice();
	modelCI.pMesh = quadMesh;
	modelCI.transform.translation = Vec3(-2.0f, 0.0f, -1.0f);
	modelCI.transform.orientation = Quat(1.0f, 0.0f, 0.0f, 0.0f);
	modelCI.transform.scale = Vec3(1.0f, 1.0f, 1.0f);
	modelCI.pTexture = gear_logo;
	modelCI.colour = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	modelCI.pPipeline.reset(pipelines["basic"].get());
	gear::Ref<Model> quad1 = gear::CreateRef<Model>(&modelCI);
	
	modelCI.transform.translation = Vec3(+2.0f, 0.0f, -1.0f);
	modelCI.pTexture = miru_logo;
	gear::Ref<Model> quad2 = gear::CreateRef<Model>(&modelCI);

	modelCI.transform.translation = Vec3(0.0f, -2.0f, -2.0f);
	modelCI.transform.orientation = Quat((float)-pi/2, Vec3(1, 0, 0));
	modelCI.transform.scale = Vec3(500.0f, 500.0f, 1.0f);
	modelCI.pTexture = woodFloor;
	gear::Ref<Model> floor = gear::CreateRef<Model>(&modelCI);

	modelCI.pMesh = stallMesh;
	modelCI.transform.translation = Vec3(0.0f, -2.0f, -5.0f);
	modelCI.transform.orientation = Quat((float)pi, Vec3(0, 1, 0));
	modelCI.transform.scale = Vec3(1.0f, 1.0f, 1.0f);
	modelCI.pTexture = stall_tex;
	gear::Ref<Model> stall = gear::CreateRef<Model>(&modelCI);
	
	modelCI.pMesh = cubeMesh;
	modelCI.transform.translation = Vec3(0.0f, 0.0f, 0.0f);
	modelCI.transform.orientation = Quat(1.0f, 0.0f, 0.0f, 0.0f);
	modelCI.transform.scale = Vec3(500.0f, 500.0f, 500.0f);
	modelCI.pTexture = skybox_cubemap;
	modelCI.pPipeline.reset(pipelines["cube"].get());
	gear::Ref<Model> skybox = gear::CreateRef<Model>(&modelCI);

	Light::SetContext(window.GetContext());
	Light light(window.GetDevice(), Light::LightType::GEAR_LIGHT_POINT, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f), Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	light.Specular(64.0f, 10.0f);
	light.Ambient(5.0f);
	light.Attenuation(0.007f, 0.0002f);
	light.SpotCone(DegToRad(45.0));

	Camera::SetContext(window.GetContext());
	Camera cam(window.GetDevice(), GEAR_CAMERA_PERSPECTIVE, Vec3(0, 0, 0), Vec3(0, 0, 1), Vec3(0, 1, 0));
	cam.DefineProjection(90.0, window.GetRatio(), 0.01f, 3000.0f, false, false);
	cam.DefineView();

	Renderer renderer(window.GetContext());

	double yaw = 0;
	double pitch = 0;
	double roll = 0;

	double pos_x = 0, pos_y = 0;
	double last_pos_x = window.GetWidth() / 2.0;
	double last_pos_y = window.GetHeight() / 2.0;
	bool initMouse = true;
	float increment = 0.5f;
	gear::core::Timer timer;

	bool windowResize = false;
	while (!window.Closed())
	{
		if (window.GetSwapchain()->m_Resized)
		{
			for (auto& pipeline : pipelines)
			{
				pipeline.second->m_CI.viewportState.viewports = { { 0.0f, 0.0f, (float)window.GetWidth(), (float)window.GetHeight(), 0.0f, 1.0f } };
				pipeline.second->m_CI.viewportState.scissors = { { { 0, 0 },{ (uint32_t)window.GetWidth(), (uint32_t)window.GetHeight() } } };
				pipeline.second->Rebuild();
			}
			window.GetSwapchain()->m_Resized = false;
		}

		if (window.IsKeyPressed(GLFW_KEY_R))
		{
			window.GetContext()->DeviceWaitIdle();
			for (auto& pipeline : pipelines)
				pipeline.second->RecompileShaders();
		}
		renderer.SubmitFramebuffer(window.GetFramebuffers());
		renderer.SubmitCamera(&cam);
		renderer.SubmitLights({ &light });
		//renderer.Submit(quad1);
		//renderer.Submit(quad2);
		//renderer.Submit(stall);
		//renderer.Submit(floor);
		renderer.Submit(skybox);
		renderer.Flush();

		//Keyboard and Mouse input
		double fov = 0.0;
		window.GetScrollPosition(fov);
		{
			if (window.IsKeyPressed(GLFW_KEY_D))
				cam.m_Position -= Vec3::Normalise(Vec3::Cross(cam.m_Up, cam.m_Forward)) * increment * timer;
			if (window.IsKeyPressed(GLFW_KEY_A))
				cam.m_Position += Vec3::Normalise(Vec3::Cross(cam.m_Up, cam.m_Forward)) * increment * timer;
			if (window.IsKeyPressed(GLFW_KEY_S))
				cam.m_Position -= cam.m_Forward * increment * timer;
			if (window.IsKeyPressed(GLFW_KEY_W))
				cam.m_Position += cam.m_Forward * increment * timer;


			window.GetMousePosition(pos_x, pos_y);

			if (initMouse || window.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT))
			{
				last_pos_x = pos_x;
				last_pos_y = pos_y;
				initMouse = false;
			}

			double offset_pos_x = pos_x - last_pos_x;
			double offset_pos_y = pos_y - last_pos_y;
			last_pos_x = pos_x;
			last_pos_y = pos_y;
			offset_pos_x *= static_cast<double>(increment) * static_cast<double>(increment) * (1.0 / (abs(fov) + 1.0)) * (double)timer;
			offset_pos_y *= static_cast<double>(increment) * static_cast<double>(increment) * (1.0 / (abs(fov) + 1.0)) * (double)timer;
			yaw += 2 * offset_pos_x;
			pitch += offset_pos_y;
			if (pitch > pi / 2)
				pitch = pi / 2;
			if (pitch < -pi / 2)
				pitch = -pi / 2;
		}

		//Camera Update
		cam.DefineProjection(DegToRad(90.0 - fov), window.GetRatio(), 0.01f, 1500.0f, false, false);
		cam.UpdateCameraPosition();
		cam.CalcuateLookAround(yaw, pitch, roll, true);
		cam.m_Position.y = 1.0f;
		cam.DefineView();
		renderer.UpdateCamera();

		renderer.Present(window.GetSwapchain(), windowResize);
		window.Update();
	}
	window.GetContext()->DeviceWaitIdle();
}