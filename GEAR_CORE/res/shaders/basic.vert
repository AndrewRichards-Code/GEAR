#version 450 core

layout(location = 0) in vec4 positions;
layout(location = 1) in vec2 textCoords;
layout(location = 2) in float textIds;
layout(location = 3) in vec4 normals;
layout(location = 4) in vec4 colours;

out DATA
{
	vec4 v_Position;
	vec2 v_TextCoord;
	float v_TextIds;
	vec4 v_Normal;
	vec4 v_WorldSpace;
	vec4 v_VertexToCamera;
	vec4 v_Colour;
} vs_out;

layout(binding = 0) uniform ubo
{
	mat4 u_Proj;
	mat4 u_View;
};

uniform mat4 u_Modl;
uniform vec3 u_CameraPosition;

void main()
{
	gl_Position = u_Proj * u_View * u_Modl * positions;
	vs_out.v_Position = positions;
	vs_out.v_TextCoord = textCoords;
	vs_out.v_TextIds = textIds;
	vs_out.v_Normal = mat4(transpose(inverse(u_Modl))) * normals;
	vs_out.v_WorldSpace = (u_Modl *  positions);
	vs_out.v_VertexToCamera = vec4(u_CameraPosition, 0.0) - vs_out.v_WorldSpace;
	vs_out.v_Colour = colours;
}
