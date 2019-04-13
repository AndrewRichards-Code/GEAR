#version 450 core

//To Post-Processing
layout(location = 0) out vec4 colour;

//From Vertex Shader
layout(location = 0) in vec4 v_Position;
layout(location = 1) in vec3 v_TextCoord;
layout(location = 2) in vec4 v_ReflectionDirection;
layout(location = 3) in vec4 v_RefractionDirection;

//From Application
layout(binding = 0) uniform samplerCube u_CubeMap;

void main()
{
	vec4 reflectionColour = texture(u_CubeMap, v_ReflectionDirection.xyz);
	vec4 refractionColour = texture(u_CubeMap, v_RefractionDirection.xyz);
	vec4 compositeColour = mix(reflectionColour, refractionColour, 0);

	colour = colour = mix(vec4(0.7, 0.7, 0.7, 1), compositeColour, 0.65);    
}