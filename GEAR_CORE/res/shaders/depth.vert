#version 450 core

layout(location = 0) in vec4 positions;

uniform mat4 u_Proj;
uniform mat4 u_View;
uniform mat4 u_Modl;
uniform float u_CameraPosition;

void main()
{
	gl_Position = u_Proj * u_View * u_Modl * positions;
}
