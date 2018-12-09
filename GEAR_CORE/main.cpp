#include "src/gear.h"
//#include "src/utils/fbxLoader.h"

#define GEAR_USE_FBO 0

using namespace GEAR;
using namespace GRAPHICS;
using namespace AUDIO;
using namespace INPUT;
using namespace ARM;

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

#if _DEBUG
	DebugOpenGL debug;
#endif

	Shader shader("res/shaders/basic.vert", "res/shaders/basic.frag");
	shader.SetLighting(GEAR_CALC_LIGHT_DIFFUSE + GEAR_CALC_LIGHT_SPECULAR + GEAR_CALC_LIGHT_AMBIENT);

	Shader shaderCube("res/shaders/cube.vert", "res/shaders/cube.frag");
	//Shader shaderPBR("res/shaders/pbr.vert", "res/shaders/pbr.frag");

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

	Object skybox("res/obj/cube.obj", shaderCube, textureSB, Mat4::Scale(Vec3(500, 500, 500)));
	Object cube("res/obj/cube.obj", shader, Vec4(1.0f, 1.0f, 1.0f, 1.0f), Mat4::Identity());
	Object stall("res/obj/stall.obj", shader, texture, Mat4::Identity());
	Object quad1("res/obj/quad.obj", shader, texture2, Mat4::Translation(Vec3(1.5f, 0.0f, -2.0f)));
	Object quad2("res/obj/quad.obj", shader, texture2, Mat4::Translation(Vec3(-1.5f, 0.0f, -2.0f)));
	Object floor("res/obj/quad.obj", shader, texture4, Mat4::Translation(Vec3(0.0f, -2.0f, -2.0f)) * Mat4::Rotation(pi / 2, Vec3(1, 0, 0)) * Mat4::Scale(Vec3(500, 500, 1)));
	//Object sword("res/obj/KagemitsuG4.obj", shader, texture3, Mat4::Translation(Vec3(0, 1, 0)));

	//For BatchRender2D
	Object quad3("res/obj/quad.obj", shader, texture, Vec3(0.0f, 0.0f, 2.0f), Vec2(0.5f, 0.5f));
	Object quad4("res/obj/quad.obj", shader, texture2, Vec3(1.5f, 0.0f, 2.0f), Vec2(0.5f, 0.5f));
	Object quad5("res/obj/quad.obj", shader, texture3, Vec3(-1.5f, 0.0f, 2.0f), Vec2(0.5f, 0.5f));

	Font testFont1(window.GetTitle(),																				"res/font/Source_Code_Pro/SourceCodePro-Regular.ttf", 75, Vec2(10.0f, 700.0f), Vec4(1.0f, 1.0f, 1.0f, 1.0f), window);
	Font testFont2(window.GetDeviceName(),																			"res/font/Source_Code_Pro/SourceCodePro-Regular.ttf", 75, Vec2(10.0f, 690.0f), Vec4(1.0f, 1.0f, 1.0f, 1.0f), window);
	Font testFont3("OpenGL: " + window.GetOpenGLVersion() + " | GLSL: " + window.GetGLSLVersion(),					"res/font/Source_Code_Pro/SourceCodePro-Regular.ttf", 75, Vec2(10.0f, 680.0f), Vec4(1.0f, 1.0f, 1.0f, 1.0f), window);
	Font testFont4("Resolution: " + std::to_string(window.GetWidth()) + " x " + std::to_string(window.GetHeight()), "res/font/Source_Code_Pro/SourceCodePro-Regular.ttf", 75, Vec2(10.0f, 670.0f), Vec4(1.0f, 1.0f, 1.0f, 1.0f), window);
	Font testFont5("MSAA: " + window.GetAntiAliasingValue() + "x",													"res/font/Source_Code_Pro/SourceCodePro-Regular.ttf", 75, Vec2(10.0f, 660.0f), Vec4(1.0f, 1.0f, 1.0f, 1.0f), window);
	Font testFont6("Anisotrophic: " + Texture::GetAnisotrophicValue() + "x",										"res/font/Source_Code_Pro/SourceCodePro-Regular.ttf", 75, Vec2(10.0f, 650.0f), Vec4(1.0f, 1.0f, 1.0f, 1.0f), window);
	Font testFont7("FPS: " + window.GetFPSString<int>(),															"res/font/Source_Code_Pro/SourceCodePro-Regular.ttf", 75, Vec2(10.0f, 640.0f), Vec4(1.0f, 1.0f, 1.0f, 1.0f), window);

	Camera cam_main(GEAR_CAMERA_PERSPECTIVE, shader, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, 1.0f, 0.0f));

	Light light_main(GEAR_LIGHT_SPOT, Vec3(0.0f, 10.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f), Vec4(1.0f, 1.0f, 1.0f, 1.0f), shader);
	light_main.Specular(64.0f, 10.0f);
	light_main.Ambient(0.5f);
	light_main.Attenuation(0.007f, 0.0002f);
	light_main.SpotCone(DegToRad(45));
	
	/*shaderPBR.Enable();
	shaderPBR.SetUniform<float>("u_LightColour", {1.0f, 1.0f, 1.0f, 1.0f});
	shaderPBR.SetUniform<float>("u_F0", { 0.99f, 0.98f, 0.96f }); //Base reflectivity
	shaderPBR.SetUniform<float>("u_Albedo", { 0.0f, 0.0f, 0.0f }); //Diffuse lighting
	shaderPBR.SetUniform<float>("u_Metallic", { 1.0f });
	shaderPBR.SetUniform<float>("u_Roughness", { 0.0f }); //Num of Micro-facets
	shaderPBR.SetUniform<float>("u_AO", { 0.00f }); //Micro-facets shadowing*/

	Listener listener_main(cam_main);

	BatchRenderer2D br2d;
	Renderer renderer;
	
	InputManager main_input(GEAR_INPUT_JOYSTICK_CONTROLLER);
	FrameBuffer fbo(window, shader);

	//FBX::read_fbx("res/obj/KagemitsuG4.fbx");

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
		Shader logo_shader("res/shaders/basic.vert", "res/shaders/basic.frag");
		logo_shader.SetLighting(GEAR_CALC_LIGHT_AMBIENT);

		Texture logo_texture("res/gear_core/GEAR_logo_square.png");
		Texture render_texture("res/gear_core/GEAR_OpenGL.png");
		Object logo("res/obj/quad.obj", logo_shader, logo_texture, Mat4::Translation(Vec3(0.0f, 0.0f, -1.0f)));
		Object render("res/obj/quad.obj", logo_shader, render_texture, Mat4::Translation(Vec3(0.0f, 0.0f, -1.0f)));

		Light logo_light(GEAR_LIGHT_POINT, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f), Vec4(1.0f, 1.0f, 1.0f, 1.0f), logo_shader);
		logo_light.Attenuation(0, 0);

		int time = 0;
		float increment = 0.017f;
		float ambient = 0.0f;
		while (time < 300)
		{
			window.Clear();
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			logo_shader.Enable();
			if (time < 60)
			{
				ambient += increment;
				logo_shader.SetUniform<float>("u_AmbientFactor", { ambient });
			}
			else if (time > 240)
			{
				ambient -= increment;
				logo_shader.SetUniform<float>("u_AmbientFactor", { ambient });
			}
			else
				logo_shader.SetUniform<float>("u_AmbientFactor", { 1.0f });
			
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
			logo_shader.Enable();
			if (time < 60 + 300)
			{
				ambient += increment;
				logo_shader.SetUniform<float>("u_AmbientFactor", { ambient });
			}
			else if (time > 240 + 300)
			{
				ambient -= increment;
				logo_shader.SetUniform<float>("u_AmbientFactor", { ambient });
			}
			else
				logo_shader.SetUniform<float>("u_AmbientFactor", { 1.0f });

			cam_main.DefineProjection(DegToRad(90), window.GetRatio(), 0.01f, 2.0f);
			cam_main.DefineView();

			renderer.Draw(&render);
			window.Update();
			if (window.IsKeyPressed(GLFW_KEY_ENTER)) break;
			time++;
		}
	}
		
	std::thread AudioThread(AudioThread);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	while (!window.Closed())
	{
#if GEAR_USE_FBO
		fbo.Bind();
		fbo.UpdateFrameBufferSize();
#endif
		window.Clear();
		window.CalculateFPS();
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		
		//Joystick Input
		main_input.Update();
		if(main_input.m_JoyStickPresent)
		{
			if (main_input.m_Axis[0] > 0.1)
				cam_main.m_Position = cam_main.m_Position - Vec3::Normalise(Vec3::Cross(cam_main.m_Up, cam_main.m_Forward)) * 2 * increment;
			if (main_input.m_Axis[0] < -0.1)
				cam_main.m_Position = cam_main.m_Position + Vec3::Normalise(Vec3::Cross(cam_main.m_Up, cam_main.m_Forward)) * 2 * increment;
			if (main_input.m_Axis[1] < -0.1)
				cam_main.m_Position = cam_main.m_Position - cam_main.m_Forward * 2 * increment;
			if (main_input.m_Axis[1] > 0.1)
				cam_main.m_Position = cam_main.m_Position + cam_main.m_Forward * 2 * increment;
			
			pos_x += (main_input.m_Axis[2] * increment);
			pos_y += (main_input.m_Axis[3] * increment);

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
			offset_pos_x *= increment * increment;
			offset_pos_y *= increment * increment;
			yaw += 2 * offset_pos_x;
			pitch += offset_pos_y;
			if (pitch > pi / 2)
				pitch = pi / 2;
			if (pitch < -pi / 2)
				pitch = -pi / 2;
		}


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
				light_main.m_Position.x = light_main.m_Position.x + increment;
			if (window.IsKeyPressed(GLFW_KEY_J))
				light_main.m_Position.x = light_main.m_Position.x - increment;
			if (window.IsKeyPressed(GLFW_KEY_H))
				light_main.m_Position.y = light_main.m_Position.y + increment;
			if (window.IsKeyPressed(GLFW_KEY_N))
				light_main.m_Position.y = light_main.m_Position.y - increment;
			if (window.IsKeyPressed(GLFW_KEY_K))
				light_main.m_Position.z = light_main.m_Position.z + increment;
			if (window.IsKeyPressed(GLFW_KEY_I))
				light_main.m_Position.z = light_main.m_Position.z - increment;
		}
		
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

			light_main.UpdateDirection(Light_yaw, Light_pitch, Light_roll, false);
		}

		light_main.UpdatePosition();
		/*shaderPBR.Enable();
		shaderPBR.SetUniform<float>("u_LightPosition", { light_main.m_Position.x , light_main.m_Position.y , light_main.m_Position.z });*/
		cube.SetUniformModlMatrix(Mat4::Translation(light_main.m_Position) * Mat4::Scale(Vec3(0.1f, 0.1f, 0.1f)));

		//Camera Update
		cam_main.DefineProjection(DegToRad(90), window.GetRatio(), 0.01f, 1500.0f);
		cam_main.UpdateCameraPosition();
		cam_main.CalcuateLookAround(yaw, pitch, roll, true);
		cam_main.m_Position.y = 1.0f;
		cam_main.DefineView();
		listener_main.UpdateListenerPosVelOri();


		//Main Render
		//renderer.AddLight(&light_main);
		stall.SetUniformModlMatrix(Mat4::Translation(Vec3(5.0f, -2.0f, -5.0f)) * Mat4::Rotation(pi, Vec3(0, 1, 0)));
		renderer.Draw(&stall);
		stall.SetUniformModlMatrix(Mat4::Translation(Vec3(-5.0f, -2.0f, -5.0f)) * Mat4::Rotation(pi, Vec3(0, 1, 0)));
		renderer.Draw(&stall);
		
		//renderer.Submit(&sword);
		renderer.Submit(&cube);
		renderer.Submit(&floor);
		renderer.Submit(&quad1);
		renderer.Submit(&quad2);
		renderer.Flush();
		renderer.Draw(&skybox);
		
		//br2d.CopyLights(renderer.GetLights());
		br2d.OpenMapBuffer();
		br2d.Submit(&quad3);
		br2d.Submit(&quad4);
		br2d.Submit(&quad5);
		br2d.CloseMapBuffer();
		br2d.Flush();
		
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
				testFont4.UpdateText("Resolution: " + std::to_string(window.GetWidth()) + " x " + std::to_string(window.GetHeight()));
				testFont7.UpdateText("FPS: " + window.GetFPSString<int>());
			}
			//Severe performance with texture, likely due to no font atlas. 
			testFont1.RenderText();
			testFont2.RenderText();
			testFont3.RenderText();
			testFont4.RenderText();
			testFont5.RenderText();
			testFont6.RenderText();
			testFont7.RenderText();
		}

#if GEAR_USE_FBO
		//FBO
		fbo.Unbind();
		window.Clear();
		renderer.Draw(&fbo.UseFrameBufferAsObject({ 0.75f, 0.75f, 0.0f }, { 0.25f, 0.25f, 1.0f }));
		renderer.Draw(&fbo.UseFrameBufferAsObject({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }));
#endif

		window.Update();
	}
	if (AudioThread.joinable() == true)
			AudioThread.join();
}