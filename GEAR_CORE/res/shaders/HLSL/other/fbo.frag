#version 450 core

//To Post-Processing
layout(location = 0) out vec4 colour;

//From Vertex Shader
layout(location = 0) in vec2 v_TextCoord;

layout(binding = 0) uniform sampler2D u_Texture;

void main()
{
	colour = texture(u_Texture, v_TextCoord);
}