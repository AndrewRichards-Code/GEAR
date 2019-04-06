#version 450 core

//To Post-Processing
layout(location = 0) out vec4 colour;

//From Vertex Shader
layout(location = 0) in vec2 v_TextCoord;
layout(location = 1) in float v_TextIds;
layout(location = 2) in vec4 v_Colour;

//From Application
layout(binding = 1) uniform sampler2D u_Textures[32];


void main()
{
	vec4 sampled = {0, 0, 0, 0};
	if( v_TextIds > 0)
	{
		for (int i = 0; i < 32; i++)
		{
			int tid = int(v_TextIds - 0.5);
			if(tid == i)
			sampled = vec4(1.0, 1.0, 1.0, texture(u_Textures[tid], v_TextCoord).r);
		}
	}
	colour = v_Colour * sampled;
}
