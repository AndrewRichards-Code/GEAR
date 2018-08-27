#version 400 core

layout(location = 0) in vec4 positions;
layout(location = 1) in vec2 textCoords;
layout(location = 2) in vec4 normals;
layout(location = 3) in vec4 colours;

out vec4 v_Position;
out vec2 v_TextCoord;
out vec4 v_Normal;
out vec4 v_VertexToLight;
out vec4 v_VertexToCamera;
out vec4 v_Colour;

uniform mat4 u_Proj;
uniform mat4 u_View;
uniform mat4 u_Modl;
uniform vec3 u_LightPosition;
uniform vec3 u_CameraPosition;

void main()
{
	gl_Position = u_Proj * u_View * u_Modl * positions;
	v_Position = positions;
	v_TextCoord = textCoords;
	v_Normal = u_Modl * normals;
	v_VertexToLight = vec4(u_LightPosition, 0.0) - positions;
	v_VertexToCamera = vec4(u_CameraPosition, 0.0) - positions;
	v_Colour = colours;
}
