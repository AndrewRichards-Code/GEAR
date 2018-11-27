#version 400 core

layout(location = 0) in vec4 positions;

out DATA
{
	vec4 v_Position;
	vec3 v_TextCoord;
} vs_out;

layout(binding = 0) uniform ubo
{
	mat4 u_Proj;
	mat4 u_View;
};
uniform mat4 u_Modl;

void main()
{
	gl_Position = u_Proj * u_View * u_Modl * positions;
	vs_out.v_Position = positions;
	vs_out.v_TextCoord = positions.xyz;
}