#version 450 core

layout(location = 0) in vec4 positions;
layout(location = 1) in vec2 textCoords;

uniform mat4 u_Modl;

out DATA
{
	vec4 v_Position;
	vec2 v_TextCoord;
} vs_out;

void main()
{
	gl_Position = u_Modl * positions;
	vs_out.v_TextCoord = textCoords;
}