#version 450 core

//From Application
layout(location = 0) in vec4 positions;
layout(location = 1) in vec2 textCoords;
layout(location = 2) in float textIds;
layout(location = 3) in vec4 normals;
layout(location = 4) in vec4 colours;

layout(std140, binding = 0) uniform cameraUBO
{
	mat4 u_Proj;
	mat4 u_View;
	vec4 u_CameraPosition;
};

layout(std140, binding = 1) uniform modelUBO
{
	mat4 u_Modl;
};

//To Fragment Shader
layout(location = 0) out vec4 v_Position;
layout(location = 1) out vec2 v_TextCoord;
layout(location = 2) out float v_TextIds;
layout(location = 3) out vec4 v_Normal;
layout(location = 4) out vec4 v_WorldSpace;
layout(location = 5) out vec4 v_VertexToCamera;
layout(location = 6) out vec4 v_Colour;

void main()
{
	gl_Position = u_Proj * u_View * u_Modl * positions;
	v_Position = positions;
	v_TextCoord = textCoords;
	v_TextIds = textIds;
	v_Normal = mat4(transpose(inverse(u_Modl))) * normals;
	v_WorldSpace = (u_Modl * positions);
	v_VertexToCamera = u_CameraPosition - v_WorldSpace;
	v_Colour = colours;
}
