#version 450 core

//To Post-Processing
layout(location = 0) out vec4 fragmentdepth;

layout(std140, binding = 5) uniform depthUBO
{
	float u_Near; 
	float u_Far;
	float u_Linear;
	float u_Reverse;
};

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * u_Near * u_Far) / (u_Far + u_Near - z * (u_Far - u_Near));	
}

void main()
{             
    float depth = LinearizeDepth(gl_FragCoord.z) / u_Far; // divide by far for demonstration
    fragmentdepth = vec4(vec3(depth), 1.0);
}