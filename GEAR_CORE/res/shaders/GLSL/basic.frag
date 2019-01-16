#version 450 core

//To Post-Processing
layout(location = 0) out vec4 colour;

//From Vertex Shader
layout(location = 0) in vec4 v_Position;
layout(location = 1) in vec2 v_TextCoord;
layout(location = 2) in float v_TextIds;
layout(location = 3) in vec4 v_Normal;
layout(location = 4) in vec4 v_WorldSpace;
layout(location = 5) in vec4 v_VertexToCamera;
layout(location = 6) in vec4 v_Colour;

//From Application
layout(binding = 0) uniform sampler2D u_Texture;
layout(binding = 1) uniform sampler2D u_Textures[32];

struct Light
{
	vec4  Colour;
	vec4  Position;
	vec4  Direction;
	float Type;
	float ShineFactor;
	float Reflectivity;
	float AmbientFactor;
	float AttenuationConstant;
	float AttenuationLinear;
	float AttenuationQuadratic;
	float CutOff; 
};
const int MAX_LIGHTS = 8;

layout(std140, binding = 2) uniform lightUBO
{
	Light[MAX_LIGHTS] u_Lights;
}; 

layout(std140, binding = 3) uniform setLightingUBO
{
	float u_Diffuse;
	float u_Specular;
	float u_Ambient;
	float u_Emit;
};

//Functions
vec4 diffuse =  {0, 0, 0, 0};
vec4 specular = {0, 0, 0, 0};
vec4 ambient =  {0, 0, 0, 0};
float attenuation = 1.0;

void CalcLighting(int i)
{
	if(u_Lights[i].Colour == vec4(0, 0, 0, 0))
	{
		return;
	}
	//Unit Vector Calculations
	vec4 unitNormal = normalize(v_Normal);
	vec4 unitVertexToLight = normalize(-u_Lights[i].Position + v_WorldSpace);
	vec4 unitLightDirection = normalize(-u_Lights[i].Direction);
	vec4 unitVertexToCamera = normalize(v_VertexToCamera);
	
	//Attenuation Calculations
	float VertexToLightDistance = length(v_WorldSpace - u_Lights[i].Position);
	attenuation = 1.0 / (u_Lights[i].AttenuationConstant + (u_Lights[i].AttenuationLinear * VertexToLightDistance) + (u_Lights[i].AttenuationQuadratic * VertexToLightDistance * VertexToLightDistance));
	
	//Spot Calculation
	float umbra = dot(-unitLightDirection, unitVertexToLight); 
		
	//Diffuse
	if(u_Diffuse == 1.0)
	{
		float diffuseLightFactor;
		if(u_Lights[i].Type == 2 && umbra < u_Lights[i].CutOff)
		{
			diffuse += vec4(0, 0, 0, 0);
		}
		else if(u_Lights[i].Type == 1)
		{
			diffuseLightFactor = max(dot(unitNormal, unitLightDirection), 0.0); 
		}
		else
		{
			diffuseLightFactor = max(dot(unitNormal, unitVertexToLight), 0.0); 
		}
		diffuse += diffuseLightFactor * attenuation * u_Lights[i].Colour;
	}
	
	//Specular
	if(u_Specular == 1.0)
	{
		vec4 lightDirection;
		if(u_Lights[i].Type == 2 && umbra < u_Lights[i].CutOff)
		{
			specular += vec4(0, 0, 0, 0);
		}
		else if(u_Lights[i].Type == 1)
		{
			lightDirection = unitLightDirection;
		}
		else
		{
			lightDirection = unitVertexToLight;
		}
		vec4 reflecedLightDirection = reflect(lightDirection, unitNormal);
		float specularLightFactor = max(dot(unitVertexToCamera, reflecedLightDirection), 0.0); 
		float dampedFactor = pow(specularLightFactor, u_Lights[i].ShineFactor);
		specular += dampedFactor * u_Lights[i].Reflectivity * attenuation * u_Lights[i].Colour;
	}
	
	//Ambient
	if(u_Ambient == 1.0)
	{
		ambient += u_Lights[i].AmbientFactor * attenuation * u_Lights[i].Colour;
	}

	//Emit
	vec4 emit = {0, 0, 0, 0};
	if(u_Emit == 1.0)
	{}
}

void main()
{
	for(int i = 0; i < MAX_LIGHTS; i++)
	{
		CalcLighting(i);
	}


	if(v_TextIds > 0)
	{
		for (int i = 0; i < 32; i++)
		{
			int tid = int(v_TextIds - 0.5);
			if(tid == i)
			colour = ((diffuse + specular + ambient)/MAX_LIGHTS) * texture(u_Textures[tid], v_TextCoord);
		}	
	}
	else
	{
		colour = ((diffuse + specular + ambient)/MAX_LIGHTS) * texture(u_Texture, v_TextCoord);
	}

	if(v_Colour != vec4(0, 0, 0, 0))
	{
		colour += v_Colour;
	}
}