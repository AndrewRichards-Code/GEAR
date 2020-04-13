#version 450 core

//To Post-Processing
layout(location = 0) out vec4 colour;

//From Vertex Shader
layout(location = 0) in vec4 v_Position;
layout(location = 1) in vec3 v_TextCoord;

//From Application
layout(binding = 0) uniform samplerCube u_CubeMap;

void main()
{
	colour = texture(u_CubeMap, v_TextCoord);
}