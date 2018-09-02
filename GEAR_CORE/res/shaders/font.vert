#version 400 core

layout(location = 0) in vec4 positions;

out vec2 v_TextCoord;

uniform mat4 u_Proj;

void main()
{
	gl_Position = u_Proj * vec4(positions.xy, 0.0, 1.0);
	v_TextCoord = positions.zw;
}