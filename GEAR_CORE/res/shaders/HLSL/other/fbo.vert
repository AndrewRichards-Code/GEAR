#version 450 core

//From Application
layout(location = 0) in vec4 positions;
layout(location = 1) in vec2 textCoords;

layout(std140, binding = 1) uniform modelUBO
{
	mat4 u_Modl;
};

//To Fragment Shader
layout(location = 0) out vec2 v_TextCoord;

void main()
{
	gl_Position = u_Modl * positions;
	v_TextCoord = textCoords;
}