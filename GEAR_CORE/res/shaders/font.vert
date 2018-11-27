#version 400 core

layout(location = 0) in vec4 positions;
layout(location = 1) in vec2 textCoords;
layout(location = 2) in float textIds;

out DATA
{
	vec2 v_TextCoord;
	float v_TextIds;
} vs_out;

uniform mat4 u_Proj;


void main()
{
	gl_Position = u_Proj * positions;
	if(textCoords.y == 0.0)
	{
		vs_out.v_TextCoord.x = textCoords.x;
		vs_out.v_TextCoord.y = 1.0;
	}
	if(textCoords.y == 1.0)
	{
		vs_out.v_TextCoord.x = textCoords.x;
		vs_out.v_TextCoord.y = 0.0;
	}
	vs_out.v_TextIds = textIds;
}