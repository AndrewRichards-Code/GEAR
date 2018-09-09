#version 400 core

layout(location = 0) in vec4 positions;
layout(location = 2) in float textIds;

out DATA
{
	vec2 v_TextCoord;
	float v_TextIds;
} vs_out;

uniform mat4 u_Proj;

void main()
{
	gl_Position = u_Proj * vec4(positions.xy, 0.0, 1.0);
	vs_out.v_TextCoord = positions.zw;
	vs_out.v_TextIds = textIds;
}