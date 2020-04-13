#if 0
#include "gear_core.h"
//#if !(_DEBUG)
//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
//#endif

#define GEAR_USE_FBO 0

using namespace GEAR;
using namespace GRAPHICS;
using namespace CROSSPLATFORM;
using namespace OPENGL;
using namespace AUDIO;
using namespace INPUT;
using namespace mars;

void AudioThread()
{
#if _DEBUG
	std::cout << "GEAR: AudioThread started. Thread ID: " << std::this_thread::get_id() << std::endl;
#endif

	AudioSource music("res/wav/Air_BWV1068.wav", Vec3(0, 0, 0), Vec3(0, 0, 1));
	music.DefineConeParameters(0.0f, 10.0, 45.0);
	music.Loop();
	while (true)
	{
		music.Stream();
	}
}

int main()
{
	Window window("GEAR", 1280, 720, 16);
	//window.Fullscreen();
	window.UpdateVSync(true);
	FrameBuffer fbo(window.GetWidth(), window.GetHeight(), 1);

	//std::vector<AssimpLoader::Mesh> meshes = AssimpLoader::LoadModel("res/obj/KagemitsuG4.fbx");

#if _DEBUG
	DebugOpenGL debug;
#endif

	Shader shader("res/shaders/GLSL/basic.vert", "res/shaders/GLSL/basic.frag");
	shader.SetLighting(Shader::GEAR_CALC_LIGHT_DIFFUSE | Shader::GEAR_CALC_LIGHT_SPECULAR | Shader::GEAR_CALC_LIGHT_AMBIENT);

	Shader shaderCube("res/shaders/GLSL/cube.vert", "res/shaders/GLSL/cube.frag");
	Shader shaderReflection("res/shaders/GLSL/reflection.vert", "res/shaders/GLSL/reflection.frag");
	Shader shaderDepth("res/shaders/GLSL/depth.vert", "res/shaders/GLSL/depth.frag");
	ComputeShader computeTest("res/shaders/GLSL/test.comp");
	Shader shaderPBR("res/shaders/GLSL/pbr.vert", "res/shaders/GLSL/pbr.frag");
	
	Texture::EnableDisableAniostrophicFilting(16.0f);
	Texture::EnableDisableMipMapping();
	Texture texture("res/img/stallTexture.png");
	Texture texture2("res/gear_core/GEAR_logo_square.png");
	Texture texture3("res/img/andrew_manga_3_square.png");
	Texture texture4("res/img/tileable_wood_texture_01_by_goodtextures-d31qde8.jpg");
	texture4.Tile(50.0f);

	Texture textureSB({
		"res/img/mp_arctic/arctic-ice_ft.tga",
		"res/img/mp_arctic/arctic-ice_bk.tga",
		"res/img/mp_arctic/arctic-ice_up.tga",
		"res/img/mp_arctic/arctic-ice_dn.tga",
		"res/img/mp_arctic/arctic-ice_rt.tga",
		"res/img/mp_arctic/arctic-ice_lf.tga" });

	Texture textureSB2({
		"res/img/mp_midnight/midnight-silence_ft.tga",
		"res/img/mp_midnight/midnight-silence_bk.tga",
		"res/img/mp_midnight/midnight-silence_up.tga",
		"res/img/mp_midnight/midnight-silence_dn.tga",
		"res/img/mp_midnight/midnight-silence_rt.tga",
		"res/img/mp_midnight/midnight-silence_lf.tga"
		});


	Texture albedo("res/img/rustediron2-Unreal-Engine/rustediron2_basecolor.png");
	Texture metallic("res/img/rustediron2-Unreal-Engine/rustediron2_metallic.png");
	Texture normal("res/img/rustediron2-Unreal-Engine/rustediron2_normal.png");
	Texture roughness("res/img/rustediron2-Unreal-Engine/rustediron2_roughness.png");
	Texture ao("res/img/rustediron2-Unreal-Engine/rustediron2_roughness.png");
	Material rustedIron(shaderPBR);
	rustedIron.AddTexture(albedo, Material::TextureType::GEAR_TEXTURE_ALBEDO);
	rustedIron.AddTexture(metallic, Material::TextureType::GEAR_TEXTURE_METALLIC);
	rustedIron.AddTexture(normal, Material::TextureType::GEAR_TEXTURE_NORMAL);
	rustedIron.AddTexture(roughness, Material::TextureType::GEAR_TEXTURE_ROUGHNESS);
	rustedIron.AddTexture(ao, Material::TextureType::GEAR_TEXTURE_AMBIENT_OCCLUSION);
	rustedIron.SetPBRParameters(Vec4(0.24f, 0.24f, 0.24f, 1.0f));

	Object skybox("res/obj/cube.obj", shaderCube, textureSB, Mat4::Scale(Vec3(500, 500, 500)));
	Object cube1("res/obj/cube.obj", shader, Vec4(1.0f, 1.0f, 1.0f, 1.0f), Mat4::Identity());
	Object cube2("res/obj/cube.obj", shader, Vec4(1.0f, 0.0f, 0.0f, 1.0f), Mat4::Identity());
	Object spherePBR("res/obj/stall.obj", shaderPBR, textureSB, Mat4::Translation(Vec3(0.0f, 0.0f, 5.0f)) * Mat4::Scale(Vec3(0.5f, 0.5f, 0.5f)));
	Object sphere("res/obj/sphere.obj", shaderReflection, textureSB, Mat4::Translation(Vec3(5.0f, 1.0f, 5.0f)) * Mat4::Scale(Vec3(0.5f, 0.5f, 0.5f)));
	Object stall("res/obj/stall.obj", shader, texture, Mat4::Translation(Vec3(5.0f, -2.0f, -5.0f)) * Mat4::Rotation(pi, Vec3(0, 1, 0)));
	Object stall2("res/obj/stall.obj", shader, texture, Mat4::Translation(Vec3(-5.0f, -2.0f, -5.0f)) * Mat4::Rotation(pi, Vec3(0, 1, 0)));
	Object quad1("res/obj/quad.obj", shader, texture2, Mat4::Translation(Vec3(1.5f, 0.0f, -2.0f)));
	Object quad2("res/obj/quad.obj", shader, texture2, Mat4::Translation(Vec3(-1.5f, 0.0f, -2.0f)));
	Object floor("res/obj/quad.obj", shader, texture4, Mat4::Translation(Vec3(0.0f, -2.0f, -2.0f)) * Mat4::Rotation(-pi / 2, Vec3(1, 0, 0)) * Mat4::Scale(Vec3(500, 500, 1)));

	//For BatchRender2D
	Object quad3("res/obj/quad.obj", shader, texture,  Vec4(0, 0, 0, 1), Vec3(0.0f, 0.0f, 2.0f), Vec2(0.5f, 0.5f));
	Object quad4("res/obj/quad.obj", shader, texture2, Vec4(0, 0, 0, 1), Vec3(1.5f, 0.0f, 2.0f), Vec2(0.5f, 0.5f));
	Object quad5("res/obj/quad.obj", shader, texture3, Vec4(0, 0, 0, 1), Vec3(-1.5f, 0.0f, 2.0f), Vec2(0.5f, 0.5f));
	
	//For BatchRender3D
	Object quad6("res/obj/quad.obj", shader, texture,  Mat4::Translation(Vec3(0.0f, 0.0f, 2.0f)));
	Object quad7("res/obj/quad.obj", shader, texture2, Mat4::Translation(Vec3(3.0f, 0.0f, 2.0f)));
	Object quad8("res/obj/quad.obj", shader, texture3, Mat4::Translation(Vec3(-3.0f, 0.0f, 2.0f)));

	Font font("res/font/Source_Code_Pro/SourceCodePro-Regular.ttf", 75, window.GetWidth(), window.GetHeight(), window.GetRatio());
	font.AddLine(window.GetTitle(),																					Vec2(10.0f, 700.0f), Vec4(1.00f, 1.00f, 1.00f, 1.00f));
	font.AddLine(window.GetDeviceName(),																			Vec2(10.0f, 690.0f), Vec4(1.00f, 0.00f, 0.00f, 1.00f));
	font.AddLine("OpenGL: " + window.GetOpenGLVersion() + " | GLSL:" + window.GetGLSLVersion(),						Vec2(10.0f, 680.0f), Vec4(0.33f, 0.53f, 0.64f, 1.00f));
	font.AddLine("Resolution: " + std::to_string(window.GetWidth()) + " x " + std::to_string(window.GetHeight()),   Vec2(10.0f, 670.0f), Vec4(1.00f, 1.00f, 1.00f, 1.00f));
	font.AddLine("MSAA: " + std::to_string(fbo.GetMultisampleValue()) + "x",										Vec2(10.0f, 660.0f), Vec4(1.00f, 1.00f, 1.00f, 1.00f));
	font.AddLine("Anisotrophic: " + Texture::GetAnisotrophicValue() + "x",											Vec2(10.0f, 650.0f), Vec4(1.00f, 1.00f, 1.00f, 1.00f));
	font.AddLine("FPS: " + window.GetFPSString<int>(),																Vec2(10.0f, 640.0f), Vec4(1.00f, 1.00f, 1.00f, 1.00f));

	Camera cam_main(GEAR_CAMERA_PERSPECTIVE, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, 1.0f, 0.0f));
	OmniProbe environmentProbe(Vec3(-5.0f, 1.0f, 5.0f), 128, 8, Texture::ImageFormat::GEAR_RGBA8);
	
	Light light_main1(Light::LightType::GEAR_LIGHT_POINT, Vec3(0.0f, 10.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f), Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	light_main1.Specular(64.0f, 10.0f);
	light_main1.Ambient(5.0f);
	light_main1.Attenuation(0.007f, 0.0002f);
	light_main1.SpotCone(DegToRad(45));

	Light light_main2(Light::LightType::GEAR_LIGHT_SPOT, Vec3(0.0f, 10.0f, 30.0f), Vec3(0.0f, -1.0f, 0.0f), Vec4(1.0f, 0.0f, 0.0f, 1.0f));
	light_main2.Specular(64.0f, 10.0f);
	light_main2.Ambient(0.05f);
	light_main2.Attenuation(0.007f, 0.0002f);
	light_main2.SpotCone(DegToRad(45));

	Listener listener_main(cam_main);

	BatchRenderer2D br2d;
	BatchRenderer3D br3d;
	Renderer renderer;
	Compute compute(computeTest);
	
	InputManager main_input(GEAR_INPUT_JOYSTICK_CONTROLLER);

	double yaw = 0;
	double pitch = 0;
	double roll = 0;

	double Light_yaw = 0;
	double Light_pitch = 0;
	double Light_roll = 0;
	bool rotateLight = false;

	double pos_x = 0, pos_y = 0;
	double last_pos_x = window.GetWidth() / 2.0;
	double last_pos_y = window.GetHeight() / 2.0;
	bool initMouse = true;
	float increment = 0.05f;

	bool showDebug = false;
	int fpsTime = 0;

	//Logo Splashscreen
	{
		light_main1.m_Colour = Vec4(0, 0, 0, 0);
		//light_main2.m_Colour = Vec4(0, 0, 0, 0);
		light_main1.UpdateColour();
		//light_main2.UpdateColour();	
		shader.SetLighting(Shader::GEAR_CALC_LIGHT_AMBIENT);

		Texture logo_texture("res/gear_core/GEAR_logo_square.png");
		Texture render_texture("res/gear_core/GEAR_OpenGL.png");
		Object logo("res/obj/quad.obj", shader, logo_texture, Mat4::Translation(Vec3(0.0f, 0.0f, -1.0f)));
		Object render("res/obj/quad.obj", shader, render_texture, Mat4::Translation(Vec3(0.0f, 0.0f, -1.0f)));

		Light logo_light(Light::LightType::GEAR_LIGHT_POINT, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f), GEAR_MAX_LIGHTS*Vec4(1.0f, 1.0f, 1.0f, 1.0f));
		logo_light.Attenuation(0, 0);

		int time = 0;
		float increment = 0.017f;
		float ambient = 0.0f;
		while (time < 300)
		{
			window.Clear();
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			shader.Enable();
			if (time < 60)
			{
				ambient += increment;
				logo_light.Ambient(ambient);
			}
			else if (time > 240)
			{
				ambient -= increment;
				logo_light.Ambient(ambient);
			}
			else
				logo_light.Ambient(ambient);
			
			cam_main.DefineProjection(DegToRad(90), window.GetRatio(), 0.01f, 2.0f);
			cam_main.DefineView();

			renderer.Draw(&logo);
			window.Update();
			if (window.IsKeyPressed(GLFW_KEY_ENTER)) break;
			time++;
		}
		while (time < 300 + 300)
		{
			window.Clear();
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			shader.Enable();
			if (time < 60 + 300)
			{
				ambient += increment;
				logo_light.Ambient(ambient);
			}
			else if (time > 240 + 300)
			{
				ambient -= increment;
				logo_light.Ambient(ambient);
			}
			else
				logo_light.Ambient(ambient);

			cam_main.DefineProjection(DegToRad(90), window.GetRatio(), 0.01f, 2.0f);
			cam_main.DefineView();

			renderer.Draw(&render);
			window.Update();
			if (window.IsKeyPressed(GLFW_KEY_ENTER)) break;
			time++;
		}
		light_main1.m_Colour = Vec4(1, 1, 1, 1);
		//light_main2.m_Colour = Vec4(1, 0, 0, 1);
		light_main1.UpdateColour();
		//light_main2.UpdateColour();
	}
	shader.SetLighting(Shader::GEAR_CALC_LIGHT_DIFFUSE | Shader::GEAR_CALC_LIGHT_SPECULAR | Shader::GEAR_CALC_LIGHT_AMBIENT);
	
	std::thread AudioThread(AudioThread);
	AudioThread.detach();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	//Compute Task Setup
	struct Pos
	{
		Vec4 pos[1024];
	}pos;
	struct Vel
	{
		Vec4 vel[1024];
	}vel;

	//Fill with Rand Nums
	for (int i = 0; i < 1024; i++)
	{
		float randNum = static_cast<float>(rand() % 10);
		pos.pos[i] = Vec4(randNum, randNum, randNum, 1);
		vel.vel[i] = Vec4(randNum, randNum, randNum, 1);
	}

	compute.AddImage(Texture::TextureType::GEAR_TEXTURE_2D, Texture::ImageFormat::GEAR_RGBA8, 1, 32, 32, 1, 0);
	//compute.m_Images[0].Bind(Image::ImageAccess::GEAR_WRITE_ONLY);
	compute.AddSSBO(sizeof(Pos), 0);
	compute.AddSSBO(sizeof(Vel), 1);
	
	while (!window.Closed())
	{
		//Compute Task
		{
			//compute.m_Images[0].Bind(Image::ImageAccess::GEAR_WRITE_ONLY);
			compute.AccessSSBO(0, (float*)&pos, sizeof(Pos), 0, ShaderStorageBuffer::GEAR_MAP_WRITE_BIT);
			compute.AccessSSBO(1, (float*)&vel, sizeof(Vel), 0, ShaderStorageBuffer::GEAR_MAP_WRITE_BIT);
			compute.Dispatch(1, 1, 1);
			compute.AccessSSBO(0, (float*)&pos, sizeof(Pos), 0, ShaderStorageBuffer::GEAR_MAP_READ_BIT);
			compute.AccessSSBO(1, (float*)&vel, sizeof(Vel), 0, ShaderStorageBuffer::GEAR_MAP_READ_BIT);
			//compute.m_Images[0].Unbind();
		}

		window.Clear();
		window.CalculateFPS();
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		
		//Joystick Input
		main_input.Update();
		double deadZone = 0.2;
		bool useJoystick =false;
		if(useJoystick && main_input.m_JoyStickPresent)
		{
			//main_input.PrintJoystickDetails();
			if (main_input.m_Axis[0] > deadZone)
				cam_main.m_Position = cam_main.m_Position - Vec3::Normalise(Vec3::Cross(cam_main.m_Up, cam_main.m_Forward)) * 4 * increment;
			if (main_input.m_Axis[0] < -deadZone)																			  
				cam_main.m_Position = cam_main.m_Position + Vec3::Normalise(Vec3::Cross(cam_main.m_Up, cam_main.m_Forward)) * 4 * increment;
			if (main_input.m_Axis[1] < -deadZone)
				cam_main.m_Position = cam_main.m_Position + cam_main.m_Forward * 4 * increment;
			if (main_input.m_Axis[1] > deadZone)								 
				cam_main.m_Position = cam_main.m_Position - cam_main.m_Forward * 4 * increment;
			
			if (main_input.m_Axis[2] > deadZone)
				pos_y += static_cast<double>(main_input.m_Axis[2]) * static_cast<double>(increment);
			if (main_input.m_Axis[2] < -deadZone)
				pos_y += static_cast<double>(main_input.m_Axis[2]) * static_cast<double>(increment);
			if (main_input.m_Axis[5] > deadZone)
				pos_x += static_cast<double>(main_input.m_Axis[5]) * static_cast<double>(increment);
			if (main_input.m_Axis[5] < -deadZone)
				pos_x += static_cast<double>(main_input.m_Axis[5]) * static_cast<double>(increment);

			if (initMouse || main_input.m_Button[0])
			{
				last_pos_x = pos_x;
				last_pos_y = pos_y;
				initMouse = false;
			}

			double offset_pos_x = pos_x - last_pos_x;
			double offset_pos_y = -pos_y + last_pos_y;
			last_pos_x = pos_x;
			last_pos_y = pos_y;
			yaw += 2 * offset_pos_x;
			pitch += offset_pos_y;
			if (pitch > pi / 2)
				pitch = pi / 2;
			if (pitch < -pi / 2)
				pitch = -pi / 2;
		}
		
		//Mouse and Keyboard Input
		else {
			if (window.IsKeyPressed(GLFW_KEY_D))
				cam_main.m_Position = cam_main.m_Position - Vec3::Normalise(Vec3::Cross(cam_main.m_Up, cam_main.m_Forward)) * 2 * increment;
			if (window.IsKeyPressed(GLFW_KEY_A))
				cam_main.m_Position = cam_main.m_Position + Vec3::Normalise(Vec3::Cross(cam_main.m_Up, cam_main.m_Forward)) * 2 * increment;
			if (window.IsKeyPressed(GLFW_KEY_S))
				cam_main.m_Position = cam_main.m_Position - cam_main.m_Forward * 2 * increment;
			if (window.IsKeyPressed(GLFW_KEY_W))
				cam_main.m_Position = cam_main.m_Position + cam_main.m_Forward * 2 * increment;


			window.GetMousePosition(pos_x, pos_y);

			if (initMouse || window.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE))
			{
				last_pos_x = pos_x;
				last_pos_y = pos_y;
				initMouse = false;
			}

			double offset_pos_x = pos_x - last_pos_x;
			double offset_pos_y = -pos_y + last_pos_y;
			last_pos_x = pos_x;
			last_pos_y = pos_y;
			offset_pos_x *= static_cast<double>(increment) * static_cast<double>(increment);
			offset_pos_y *= static_cast<double>(increment) * static_cast<double>(increment);
			yaw += 2 * offset_pos_x;
			pitch += offset_pos_y;
			if (pitch > pi / 2)
				pitch = pi / 2;
			if (pitch < -pi / 2)
				pitch = -pi / 2;
		}

		//Lights
		if (window.IsKeyPressed(GLFW_KEY_O))
		{
			switch (rotateLight)
			{
			case true:
				rotateLight = false;
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				break;

			case false:
				rotateLight = true;
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				break;
			}
		}
		if (!rotateLight)
		{
			if (window.IsKeyPressed(GLFW_KEY_L))
				light_main1.m_Position.x = light_main1.m_Position.x + increment;
			if (window.IsKeyPressed(GLFW_KEY_J))
				light_main1.m_Position.x = light_main1.m_Position.x - increment;
			if (window.IsKeyPressed(GLFW_KEY_H))
				light_main1.m_Position.y = light_main1.m_Position.y + increment;
			if (window.IsKeyPressed(GLFW_KEY_N))
				light_main1.m_Position.y = light_main1.m_Position.y - increment;
			if (window.IsKeyPressed(GLFW_KEY_K))
				light_main1.m_Position.z = light_main1.m_Position.z + increment;
			if (window.IsKeyPressed(GLFW_KEY_I))
				light_main1.m_Position.z = light_main1.m_Position.z - increment;
			
		}
		light_main1.UpdatePosition();
		
		if(rotateLight)
		{
			if (window.IsKeyPressed(GLFW_KEY_L))
				Light_yaw += increment * 0.5;
			if (window.IsKeyPressed(GLFW_KEY_J))
				Light_yaw -= increment * 0.5;
			if (window.IsKeyPressed(GLFW_KEY_H))
				Light_roll += increment * 0.5;
			if (window.IsKeyPressed(GLFW_KEY_N))
				Light_roll += increment * 0.5;
			if (window.IsKeyPressed(GLFW_KEY_K))
				Light_pitch += increment * 0.5;
			if (window.IsKeyPressed(GLFW_KEY_I))
				Light_pitch -= increment * 0.5;

			light_main1.UpdateDirection(Light_yaw, Light_pitch, Light_roll, false);
		}
	
		cube1.SetUniformModlMatrix(Mat4::Translation(light_main1.m_Position) * Mat4::Scale(Vec3(0.1f, 0.1f, 0.1f)));
		cube2.SetUniformModlMatrix(Mat4::Translation(light_main2.m_Position) * Mat4::Scale(Vec3(0.1f, 0.1f, 0.1f)));

		//Main Render
		//Object Submission
		//renderer.Submit(&sword);
		renderer.Submit(&skybox);
		renderer.Submit(&stall);
		renderer.Submit(&stall2);
		//renderer.Submit(&cube1);
		renderer.Submit(&cube2);
		renderer.Submit(&sphere);
		renderer.Submit(&floor);
		renderer.Submit(&quad1);
		renderer.Submit(&quad2);

		//Render to Probe
		environmentProbe.Render(renderer.GetRenderQueue(), window.GetWidth(), window.GetHeight());

		//Render to Light depth textures
		light_main1.SetDepthParameters(0.1f, 500.0f);
		light_main1.RenderDepthTexture(renderer.GetRenderQueue(), window.GetWidth(), window.GetHeight());

		//Camera Update
		cam_main.DefineProjection(DegToRad(90), window.GetRatio(), 0.01f, 1500.0f);
		cam_main.UpdateCameraPosition();
		cam_main.CalcuateLookAround(yaw, pitch, roll, true);
		cam_main.m_Position.y = 1.0f;
		cam_main.DefineView();
		listener_main.UpdateListenerPosVelOri();

		//Render to Screen
#if GEAR_USE_FBO
		fbo.Bind();
		fbo.UpdateFrameBufferSize(window.GetWidth(), window.GetHeight());
		fbo.Clear();
#endif
		renderer.Flush();

		std::shared_ptr<Texture> shadowMap;
		shadowMap = light_main1.GetDepthTexture();

		std::shared_ptr<Texture> enviromentMap;
		enviromentMap = environmentProbe.GetCubemap();
		Object cube2("res/obj/cube.obj", shaderCube, *shadowMap, Mat4::Translation(Vec3(-5.0f, 1.0f, 5.0f)) * Mat4::Scale(Vec3(0.5f, 0.5f, 0.5f)));
		renderer.Draw(&cube2);

		rustedIron.AddTexture(*enviromentMap, Material::TextureType::GEAR_TEXTURE_AMBIENT);
		rustedIron.BindPBRTextures();
		renderer.Draw(&spherePBR);
		rustedIron.UnbindPBRTextures();
		
		//br2d.OpenMapBuffer();
		//br2d.Submit(&quad3);
		//br2d.Submit(&quad4);
		//br2d.Submit(&quad5);
		//br2d.CloseMapBuffer();
		//br2d.Flush();

		br3d.OpenMapBuffer();
		br3d.Submit(&quad6);
		br3d.Submit(&cube1);
		br3d.Submit(&quad7);
		br3d.Submit(&quad8);
		br3d.CloseMapBuffer();
		br3d.Flush();
		
		fpsTime++;
		if (window.IsKeyPressed(GLFW_KEY_F3))
		{
			switch (showDebug)
			{
			case true:
				showDebug = false;
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				break;

			case false:
				showDebug = true;
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				break;
			}
		}

		if (showDebug == true)
		{
			if (fpsTime % 60 == 0)
			{
				font.UpdateLine("Resolution: " + std::to_string(window.GetWidth()) + " x " + std::to_string(window.GetHeight()), 3);
				font.UpdateLine("FPS: " + window.GetFPSString<int>(), 6);
			}
			//Severe performance with texture, likely due to no font atlas. 
			font.Render();
		}

#if GEAR_USE_FBO
		//FBO
		fbo.Unbind();
		fbo.Resolve();
		Shader fboShader("res/shaders/GLSL/fbo.vert", "res/shaders/GLSL/fbo.frag");
		Object fboColour("res/obj/quad.obj", fboShader, *(fbo.GetColourTexture()), Mat4::Scale(Vec3(1.0f, 1.0f, 1.0f)));
		Object fboDepth("res/obj/quad.obj", fboShader, *(fbo.GetDepthTexture()), Mat4::Translation(Vec3(0.75f, 0.75f, 0.0f)) * Mat4::Scale(Vec3(0.25f, 0.25f, 1.0f)));
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderer.Draw(&fboDepth);
		renderer.Draw(&fboColour);
#endif
		window.Update();
	}
	
	if (AudioThread.joinable() == true)
		AudioThread.join();
			
}
#endif

#include "gear_core.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace OBJECTS;

using namespace miru;
using namespace miru::crossplatform;

using namespace mars;

int main()
{
	Window window("GEAR_MIRU_TEST", 400, 300, 1, true, false);

	std::shared_ptr<GRAPHICS::Pipeline> basic = std::make_shared<GRAPHICS::Pipeline>(window.GetDevice(), "res/shaders/bin/basic.vert.spv", "res/shaders/bin/basic.frag.spv");
	basic->SetViewport(0.0f, 0.0f, (float)window.GetWidth(), (float)window.GetHeight(), 0.0f, 1.0f);
	basic->SetRasterisationState(false, false, PolygonMode::FILL, CullModeBit::BACK_BIT, FrontFace::COUNTER_CLOCKWISE, false, 0, 0, 0, 0);
	basic->SetMultisampleState(Image::SampleCountBit::SAMPLE_COUNT_1_BIT, false, 1.0f, false, false);
	basic->SetDepthStencilState(true, true, CompareOp::GREATER, false, false, {}, {}, 0.0f, 1.0f);
	float blendConsts[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	basic->SetColourBlendState(false, LogicOp::COPY, { {true, BlendFactor::SRC_ALPHA, BlendFactor::ONE_MINUS_SRC_ALPHA, BlendOp::ADD,
											BlendFactor::ONE, BlendFactor::ZERO, BlendOp::ADD, (ColourComponentBit)15 } }, blendConsts);
	basic->SetRenderPass(window.GetRenderPass(), 0);
	basic->FinalisePipline();

	Texture::SetContext(window.GetContext());
	std::shared_ptr<Texture> logo =  std::make_shared<Texture>(window.GetDevice(), "res/img/andrew_manga_3_square.png");

	Object::SetContext(window.GetContext());
	Object quad(window.GetDevice(), "res/obj/quad.obj", basic, logo, mars::Mat4::Translation({ 0, 0, -2 }));

	Light::SetContext(window.GetContext());
	Light light(window.GetDevice(), Light::LightType::GEAR_LIGHT_POINT, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f), GEAR_MAX_LIGHTS * Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	light.Ambient(1.0f);
	light.Attenuation(0.0f, 0.0f);

	Camera::SetContext(window.GetContext());
	Camera cam(window.GetDevice(), GEAR_CAMERA_PERSPECTIVE, Vec3(0, 0, 0), Vec3(0, 0, -1), Vec3(0, 1, 0));
	cam.DefineProjection(DegToRad(90), window.GetRatio(), 0.01f, 5.0f, false, true);
	cam.DefineView();

	Renderer renderer(window.GetContext());
	renderer.SubmitFramebuffer(window.GetFramebuffers());
	renderer.SubmitCamera(&cam);
	renderer.SubmitLights({&light});
	renderer.Submit(&quad);
	renderer.Flush();

	Fence::CreateInfo fenceCI;
	fenceCI.debugName = "DrawFence";
	fenceCI.device = window.GetDevice();
	fenceCI.signaled = true;
	fenceCI.timeout = UINT64_MAX;
	std::vector<Ref<Fence>>draws = { Fence::Create(&fenceCI), Fence::Create(&fenceCI) };
	Semaphore::CreateInfo semaphoreCI = { "Seamphore", window.GetDevice() };
	std::vector<Ref<Semaphore>>acquire = { Semaphore::Create(&semaphoreCI), Semaphore::Create(&semaphoreCI) };
	std::vector<Ref<Semaphore>>submit = { Semaphore::Create(&semaphoreCI), Semaphore::Create(&semaphoreCI) };
	bool windowResize;
	while (!window.Closed())
	{
		/*if (window.GetSwapchain()->m_Resized)
		{
			window.GetSwapchain()->m_Resized = false;
		}*/

		renderer.GetCmdBuffer()->Present({ 0, 1 }, window.GetSwapchain(), draws, acquire, submit, windowResize);
		window.Update();
	}
}