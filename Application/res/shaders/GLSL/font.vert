#version 450 core

//From Application
layout(location = 0) in vec4 positions;
layout(location = 1) in vec2 textCoords;
layout(location = 2) in float textIds;
layout(location = 4) in vec4 colours;

layout(std140, binding = 0) uniform cameraUBO
{
	mat4 u_Proj;
	mat4 u_View;
	vec4 u_CameraPosition;
};

//To Fragment Shader
layout(location = 0) out vec2 v_TextCoord;
layout(location = 1) out float v_TextIds;
layout(location = 2) out vec4 v_Colour;

void main()
{
	gl_Position = u_Proj * positions;
	if(textCoords.y == 0.0)
	{
		v_TextCoord.x = textCoords.x;
		v_TextCoord.y = 1.0;
	}
	if(textCoords.y == 1.0)
	{
		v_TextCoord.x = textCoords.x;
		v_TextCoord.y = 0.0;
	}
	v_TextIds = textIds;
	v_Colour = colours;
}