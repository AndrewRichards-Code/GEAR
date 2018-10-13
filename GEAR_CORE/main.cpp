#include "src/graphics/window.h"
#include "src/graphics/renderer/renderer.h"
#include "src/graphics/renderer/batchrenderer2d.h"
#include "src/graphics/texture.h"
#include "src/maths/ARMLib.h"
#include "src/graphics/camera.h"
#include "src/input/inputmanager.h"
#include "src/graphics/light.h"
#include "src/graphics/font.h"
#include "src/audio/listener.h"
#include "src/audio/audiosource.h"


#if _DEBUG
#include "src/graphics/debugopengl.h"
#endif

	using namespace GEAR;
	using namespace GRAPHICS;
	using namespace AUDIO;
	using namespace INPUT;
	using namespace ARM;

	int main()
	{
		Window window("GEAR", 1280, 720, 16);
		//window.Fullscreen();
		//window.UpdateVSync(false);

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

	//For BatchRender2D
	Object quad3("res/obj/quad.obj", shader, texture, Vec3(0.0f, 0.0f, -2.0f), Vec2(0.5f, 0.5f));
	Object quad4("res/obj/quad.obj", shader, texture2, Vec3(-1.5f, 0.0f, -2.0f), Vec2(0.5f, 0.5f));
	Object quad5("res/obj/quad.obj", shader, texture3, Vec3(1.5f, 0.0f, -2.0f), Vec2(0.5f, 0.5f));

	Font testFont1(window.GetTitle(),										 "res/font/consola/consola.ttf", 75, Vec2(10.0f, 700.0f), Vec4(1.0f, 1.0f, 1.0f, 1.0f), window);
	Font testFont2(window.GetOpenGLVersion(),								 "res/font/consola/consola.ttf", 75, Vec2(10.0f, 690.0f), Vec4(1.0f, 1.0f, 1.0f, 1.0f), window);
	Font testFont3("MSAA: " + window.GetAntiAliasingValue() + "x",			 "res/font/consola/consola.ttf", 75, Vec2(10.0f, 680.0f), Vec4(1.0f, 1.0f, 1.0f, 1.0f), window);
	Font testFont4("Anisotrophic: " + Texture::GetAnisotrophicValue() + "x", "res/font/consola/consola.ttf", 75, Vec2(10.0f, 670.0f), Vec4(1.0f, 1.0f, 1.0f, 1.0f), window);

	Camera cam_main(GEAR_CAMERA_PERSPECTIVE, shader, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, 1.0f, 0.0f));
	cam_main.DefineProjection((float)DegToRad(90), window.GetRatio(), 0.5f, 50.0f);

	Light light_main(GEAR_LIGHT_POINT, Vec3(0.0f, 0.0f, 0.0f), Vec4(1.0f, 1.0f, 1.0f, 1.0f), shader);
	light_main.Specular(64.0f, 10.0f);
	light_main.Ambient(0.6f);

	Listener listener_main(cam_main);
	AudioSource music("res/wav/Rainbow Road.wav", Vec3(0, 0, 0), Vec3(0, 0, 1));
	music.DefineConeParameters(0.0f, 10.0, 45.0);
	music.Loop();

	BatchRenderer2D br2d;
	Renderer renderer;
	
	InputManager main_input(GEAR_INPUT_JOYSTICK_CONTROLLER);

	double yaw = 0;
	double pitch = 0;
	double roll = 0;

	double pos_x = 0, pos_y = 0;
	double last_pos_x = window.GetWidth() / 2.0;
	double last_pos_y = window.GetHeight() / 2.0;
	bool initMouse = true;

	float increment = 0.05f;

	//Logo Splashscreen
	{
		Shader logo_shader("res/shaders/basic.vert", "res/shaders/basic.frag");
		logo_shader.SetLighting(GEAR_CALC_LIGHT_AMBIENT);

		Texture logo_texture("res/gear_core/GEAR_logo_square.png");
		Object logo("res/obj/quad.obj", logo_shader, logo_texture, Mat4::Translation(Vec3(0.0f, 0.0f, 0.0f)));

		Camera logo_cam(GEAR_CAMERA_ORTHOGRAPHIC, logo_shader, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, 1.0f, 0.0f));
		Light logo_light(GEAR_LIGHT_POINT, Vec3(0.0f, 0.0f, 0.0f), Vec4(1.0f, 1.0f, 1.0f, 1.0f), logo_shader);
		

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
			
			logo_cam.DefineProjection(-window.GetRatio(), window.GetRatio(), -1.0f, 1.0f, -1.0f, 1.0f);
			logo_cam.DefineView();

			renderer.Draw(&logo);
			window.Update();
			if (window.IsKeyPressed(GLFW_KEY_ENTER)) break;
			time++;
		}
	}

	while (!window.Closed())
	{
		window.Clear();
		window.CalculateFPS();
		shader.Enable();
		glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
		music.Stream();

		//Joystick Input
		/*if(main_input.m_JoyStickPresent)
		{
			main_input.Update();
			if (main_input.m_Axis[0] > 0.1)
				cam_main.m_Position = cam_main.m_Position - Vec3::Normalise(Vec3::Cross(cam_main.m_Up, cam_main.m_Forward)) * 2 * increment;
			if (main_input.m_Axis[0] < -0.1)
				cam_main.m_Position = cam_main.m_Position + Vec3::Normalise(Vec3::Cross(cam_main.m_Up, cam_main.m_Forward)) * 2 * increment;
			if (main_input.m_Axis[1] < -0.1)
				cam_main.m_Position = cam_main.m_Position - cam_main.m_Forward * 2 * increment;
			if (main_input.m_Axis[1] > 0.1)
				cam_main.m_Position = cam_main.m_Position + cam_main.m_Forward * 2 * increment;
			
			pos_x = main_input.m_Axis[2];
			pos_y = main_input.m_Axis[3];

			if (initMouse || main_input.m_Button[0])
			{
				last_pos_x = pos_x;
				last_pos_y = pos_y;
				initMouse = false;
			}
		}*/
		
		//Mouse and Keyboard Input
		
		if (window.IsKeyPressed(GLFW_KEY_D))
			cam_main.m_Position = cam_main.m_Position - Vec3::Normalise(Vec3::Cross(cam_main.m_Up, cam_main.m_Forward)) * 2 * increment;
		if (window.IsKeyPressed(GLFW_KEY_A))
			cam_main.m_Position = cam_main.m_Position + Vec3::Normalise(Vec3::Cross(cam_main.m_Up, cam_main.m_Forward)) * 2 * increment;
		if (window.IsKeyPressed(GLFW_KEY_S))
			cam_main.m_Position = cam_main.m_Position - cam_main.m_Forward * 2 * increment;
		if (window.IsKeyPressed(GLFW_KEY_W))
			cam_main.m_Position = cam_main.m_Position + cam_main.m_Forward * 2 * increment;

		if (window.IsKeyPressed(GLFW_KEY_L))
			light_main.m_PosDir.x = light_main.m_PosDir.x + increment;
		if (window.IsKeyPressed(GLFW_KEY_J))
			light_main.m_PosDir.x = light_main.m_PosDir.x - increment;
		if (window.IsKeyPressed(GLFW_KEY_H))
			light_main.m_PosDir.y = light_main.m_PosDir.y + increment;
		if (window.IsKeyPressed(GLFW_KEY_N))
			light_main.m_PosDir.y = light_main.m_PosDir.y - increment;
		if (window.IsKeyPressed(GLFW_KEY_K))
			light_main.m_PosDir.z = light_main.m_PosDir.z + increment;
		if (window.IsKeyPressed(GLFW_KEY_I))
			light_main.m_PosDir.z = light_main.m_PosDir.z - increment;

		light_main.Point();
		cube.SetUniformModlMatrix(Mat4::Translation(light_main.m_PosDir) * Mat4::Scale(Vec3(0.1f, 0.1f, 0.1f)));

		window.GetMousePosition(pos_x, pos_y);

		//Camera Update
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

		cam_main.UpdateCameraPosition();
		cam_main.CalcuateLookAround(yaw, pitch, roll, true);
		cam_main.m_Position.y = 1.0f;
		cam_main.DefineView();
		cam_main.DefineProjection(DegToRad(90), window.GetRatio(), 0.5f, 1000.0f);
		cam_main.UpdateProjectionAndViewInOtherShader(shaderCube);
		listener_main.UpdateListenerPosVelOri();


		//Main Render
		stall.SetUniformModlMatrix(Mat4::Translation(Vec3(5.0f, -2.0f, -5.0f)) * Mat4::Rotation(pi, Vec3(0, 1, 0)));
		renderer.Draw(&stall);
		stall.SetUniformModlMatrix(Mat4::Translation(Vec3(-5.0f, -2.0f, -5.0f)) * Mat4::Rotation(pi, Vec3(0, 1, 0)));
		renderer.Draw(&stall);
		
		renderer.Submit(&cube);
		renderer.Submit(&floor);
		renderer.Submit(&quad1);
		renderer.Submit(&quad2);
		renderer.Flush();
		renderer.Draw(&skybox);
		
		br2d.OpenMapBuffer();
		br2d.Submit(&quad3);
		br2d.Submit(&quad4);
		br2d.Submit(&quad5);
		br2d.CloseMapBuffer();
		br2d.Flush();
		
		//Severe performance with texture, likely due to no font atlas. 
		testFont1.RenderText();
		testFont2.RenderText();
		testFont3.RenderText();
		testFont4.RenderText();
		
		window.Update();
	}
}