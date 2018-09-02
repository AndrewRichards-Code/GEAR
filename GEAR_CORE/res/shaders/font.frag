#version 400 core

layout(location = 0) out vec4 colour;
in vec2 v_TextCoord;

uniform sampler2D u_Texture;
uniform vec4 u_Colour;

void main()
{
	vec4 sampled =  vec4(1.0, 1.0, 1.0, texture(u_Texture, v_TextCoord).r);
	colour = u_Colour * sampled;
}
