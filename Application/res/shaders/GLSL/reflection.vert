#version 450 core

//From Application
layout(location = 0) in vec4 positions;
layout(location = 3) in vec4 normals;

layout(std140, binding = 0) uniform cameraUBO
{
	mat4 u_Proj;
	mat4 u_View;
	vec4 u_CameraPosition;
};

layout(std140, binding = 1) uniform modelUBO
{
	mat4 u_Modl;
};

//To Fragment Shader
layout(location = 0) out vec4 v_Position;
layout(location = 1) out vec3 v_TextCoord;
layout(location = 2) out vec4 v_ReflectionDirection;
layout(location = 3) out vec4 v_RefractionDirection;


void main()
{
	gl_Position = u_Proj * u_View * u_Modl * positions;
	v_Position = positions;
	v_TextCoord = positions.xyz;
	v_ReflectionDirection = reflect(normalize((u_Modl * positions) - u_CameraPosition), normalize(normals));
	v_RefractionDirection = refract(normalize((u_Modl * positions) - u_CameraPosition), normalize(normals), 1.0/1.33);
}