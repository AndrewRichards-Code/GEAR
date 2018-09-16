#version 400 core

layout(location = 0) out vec4 colour;

in DATA
{
	vec2 v_TextCoord;
	float v_TextIds;
} fs_in;

uniform sampler2D u_Textures[32];
uniform vec4 u_Colour;

void main()
{
	vec4 sampled = {1.0, 1.0, 1.0, 1.0};
	if( fs_in.v_TextIds > 0)
	//{
	//	int tid = int(fs_in.v_TextIds - 0.5);
	//	 sampled = vec4(1.0, 1.0, 1.0, texture(u_Textures[tid], fs_in.v_TextCoord).r);
	//}
	for (int i = 0; i < 32; i++)
	{
		int tid = int(fs_in.v_TextIds - 0.5);
		if(tid == i)
		sampled = vec4(1.0, 1.0, 1.0, texture(u_Textures[tid], fs_in.v_TextCoord).r);
	}

	colour = u_Colour * sampled;
}
