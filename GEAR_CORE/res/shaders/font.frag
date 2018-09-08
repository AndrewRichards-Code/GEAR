#version 400 core

layout(location = 0) out vec4 colour;

in DATA
{
	vec2 v_TextCoord;
	unsigned int v_TextIds;
} fs_in;

uniform sampler2D u_Textures[32];
uniform vec4 u_Colour;

void main()
{
	vec4 sampled = {1.0, 1.0, 1.0, 1.0};
	if( fs_in.v_TextIds > 0)
	{
		 sampled = vec4(1.0, 1.0, 1.0, texture(u_Textures[fs_in.v_TextIds], fs_in.v_TextCoord).r);
	}
	colour = u_Colour * sampled;
}
