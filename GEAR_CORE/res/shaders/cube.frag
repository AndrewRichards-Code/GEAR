#version 400 core

layout(location = 0) out vec4 colour;

in DATA
{
	vec4 v_Position;
	vec3 v_TextCoord;
} fs_in;

uniform samplerCube u_CubeMap;

void main()
{
	colour = texture(u_CubeMap, fs_in.v_TextCoord);
}