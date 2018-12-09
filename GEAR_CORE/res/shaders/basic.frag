#version 450 core

layout(location = 0) out vec4 colour;

in DATA
{
	vec4 v_Position;
	vec2 v_TextCoord;
	float v_TextIds;
	vec4 v_Normal;
	vec4 v_VertexToLight;
	vec4 v_VertexToCamera;
	vec4 v_Colour;
} fs_in;

uniform sampler2D u_Texture;
uniform sampler2D u_Textures[32];

uniform int u_SetLighting[4];
uniform int u_LightType;
uniform vec4 u_LightColour;
uniform vec3 u_LightDirection;
uniform float u_ShineFactor;
uniform float u_Reflectivity;
uniform float u_AmbientFactor;
uniform float u_AttenuationConstant;
uniform float u_AttenuationLinear;
uniform float u_AttenuationQuadratic;
uniform float u_CutOff; 

//Functions
vec4 diffuse = {0, 0, 0, 0};
vec4 specular = {0, 0, 0, 0};
vec4 ambient = {0, 0, 0, 0};
float attenuation = 0.0;
float umbra = 0.0;

void CalcLighting()
{
	//Unit Vector Calculations
	vec4 unitNormal = normalize(fs_in.v_Normal);
	vec4 unitVertexToLight = normalize(-fs_in.v_VertexToLight);
	vec4 unitLightDirection = normalize(vec4(-u_LightDirection, 0.0));
	vec4 unitVertexToCamera = normalize(fs_in.v_VertexToCamera);
	
	//Attenuation Calculations
	float VertexToLightDistance = length(fs_in.v_VertexToLight);
	attenuation = 1.0 / (u_AttenuationConstant + (u_AttenuationLinear * VertexToLightDistance) + (u_AttenuationQuadratic * VertexToLightDistance * VertexToLightDistance));
	
	//Spot Calculation
	umbra = dot(vec4(u_LightDirection, 0.0), unitVertexToLight); 
		
	//Diffuse
	if(u_SetLighting[0] == 1)
	{
		float diffuseLightFactor;
		if(u_LightType == 2 && umbra < u_CutOff)
		{
			diffuse = vec4(0, 0, 0, 0);
		}
		else if(u_LightType == 1)
		{
			diffuseLightFactor = max(dot(unitNormal, unitLightDirection), 0.0); 
		}
		else
		{
			diffuseLightFactor = max(dot(unitNormal, unitVertexToLight), 0.0); 
		}
		diffuse = diffuseLightFactor * u_LightColour;
	}
	
	//Specular
	if(u_SetLighting[1] == 1)
	{
		vec4 lightDirection;
		if(u_LightType == 2 && umbra < u_CutOff)
		{
			specular = vec4(0, 0, 0, 0);
		}
		else if(u_LightType == 1)
		{
			lightDirection = unitLightDirection;
		}
		else
		{
			lightDirection = unitVertexToLight;
		}
		vec4 reflecedLightDirection = reflect(lightDirection, unitNormal);
		float specularLightFactor = max(dot(unitVertexToCamera, reflecedLightDirection), 0.0); 
		float dampedFactor = pow(specularLightFactor, u_ShineFactor);
		specular =  dampedFactor * u_Reflectivity * u_LightColour;
	}
	
	//Ambient
	if(u_SetLighting[2] == 1)
	{
		ambient = u_AmbientFactor * u_LightColour;
	}

	//Emit
	vec4 emit = {0, 0, 0, 0};
	if(u_SetLighting[3] == 1)
	{}
}

void main()
{
	CalcLighting();

	if(fs_in.v_TextIds > 0)
	{
		for (int i = 0; i < 32; i++)
		{
			int tid = int(fs_in.v_TextIds - 0.5);
			if(tid == i)
			colour = (diffuse + specular + ambient) * attenuation * texture(u_Textures[tid], fs_in.v_TextCoord);
		}	
	}
	else
	{
		colour = (diffuse + specular + ambient) *  attenuation * texture(u_Texture, fs_in.v_TextCoord);
	}

	if(fs_in.v_Colour != vec4(0, 0, 0, 0))
	{
		colour += fs_in.v_Colour;
	}
}