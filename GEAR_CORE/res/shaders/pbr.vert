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
	vec4 v_VertexToLight;
	vec4 v_VertexToCamera;
	vec4 v_Colour;
} vs_out;

uniform mat4 u_Proj;
uniform mat4 u_View;
uniform mat4 u_Modl;
uniform vec3 u_LightPosition;
uniform vec3 u_CameraPosition;

void main()
{
	gl_Position = u_Proj * u_View * u_Modl * positions;
	vs_out.v_Position = positions;
	vs_out.v_TextCoord = textCoords;
	vs_out.v_TextIds = textIds;
	vs_out.v_Normal = normals;
	vs_out.v_VertexToLight = vec4(u_LightPosition, 0.0) - (u_Modl *  positions);
	vs_out.v_VertexToCamera = vec4(u_CameraPosition, 0.0) - (u_Modl * positions);
	vs_out.v_Colour = colours;
}
