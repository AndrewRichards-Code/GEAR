#version 450 core

//From Application
layout(location = 0) in vec4 positions;

layout(binding = 0) uniform cameraUBO
{
	mat4 u_Proj;
	mat4 u_View;
	vec4 u_CameraPosition;
};

layout(std140, binding = 1) uniform modelUBO
{
	mat4 u_Modl;
};

void main()
{
	gl_Position = u_Proj * u_View * u_Modl * positions;
}
