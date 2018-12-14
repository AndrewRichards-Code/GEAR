#version 450 core

layout(location = 0) out vec4 colour;

in DATA
{
	vec4 v_Position;
	vec2 v_TextCoord;
} fs_in;

uniform sampler2D u_Texture;

void main()
{
	colour = texture(u_Texture, fs_in.v_TextCoord);
}