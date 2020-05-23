#include "msc_common.h"

//To Post-Processing
struct PS_OUT
{
	MIRU_LOCATION(0, float4, colour, SV_TARGET0);
};

//From Vertex Shader
struct PS_IN
	{
	MIRU_LOCATION(0, float4, v_Position, SV_POSITION);
	MIRU_LOCATION(1, float2, v_TextCoord, TEXCOORD1);
	MIRU_LOCATION(2, float, v_TextIds, PSIZE2);
	MIRU_LOCATION(3, float4, v_Normal, NORMAL3);
	MIRU_LOCATION(4, float4, v_WorldSpace, POSITION5);
	MIRU_LOCATION(5, float4, v_VertexToCamera, POSITION6);
	MIRU_LOCATION(6, float4, v_Colour, COLOR7);
};

//From Application
MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_2D, 1, 1, float4, uTexture);
//MIRU_COMBINED_IMAGE_SAMPLER_ARRAY(MIRU_IMAGE_2D, 1, 2, float4, u_Textures, 32);

struct Light
{
	float4 Colour;
	float4 Position;
	float4 Direction;
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

struct Lights
{
	Light u_Lights[8];
}; 
MIRU_UNIFORM_BUFFER(2, 0, Lights, lights);

struct Lighting
{
	float u_Diffuse;
	float u_Specular;
	float u_Ambient;
	float u_Emit;
};
MIRU_UNIFORM_BUFFER(3, 0, Lighting, lighting);

//Functions
static float4 diffuse =  {0, 0, 0, 0};
static float4 specular = {0, 0, 0, 0};
static float4 ambient =  {0, 0, 0, 0};
static float attenuation = 1.0;

void CalcLighting(int i, PS_IN IN)
{
	if( lights.u_Lights[i].Colour.r == 0.0 &&
		lights.u_Lights[i].Colour.g == 0.0 &&
		lights.u_Lights[i].Colour.b == 0.0 &&
		lights.u_Lights[i].Colour.a == 0.0 )
	{
		return;
	}
	//Unit Vector Calculations
	float4 unitNormal = normalize(IN.v_Normal);
	float4 unitVertexToLight = normalize(-lights.u_Lights[i].Position + IN.v_WorldSpace);
	float4 unitLightDirection = normalize(-lights.u_Lights[i].Direction);
	float4 unitVertexToCamera = normalize(IN.v_VertexToCamera);
	
	//Attenuation Calculations
	float VertexToLightDistance = length(IN.v_WorldSpace - lights.u_Lights[i].Position);
	float _const = lights.u_Lights[i].AttenuationConstant;
	float _linear = lights.u_Lights[i].AttenuationLinear;
	float _quad = lights.u_Lights[i].AttenuationQuadratic;
	attenuation = 1.0 / _const + (_linear * VertexToLightDistance) + (_quad * VertexToLightDistance * VertexToLightDistance);
	
	//Spot Calculation
	float umbra = dot(-unitLightDirection, unitVertexToLight); 
		
	//Diffuse
	if(lighting.u_Diffuse == 1.0)
	{
		float diffuseLightFactor;
		if(lights.u_Lights[i].Type == 2 && umbra < lights.u_Lights[i].CutOff)
		{
			diffuse += float4(0, 0, 0, 0);
		}
		else if(lights.u_Lights[i].Type == 1)
		{
			diffuseLightFactor = max(dot(unitNormal, unitLightDirection), 0.0); 
		}
		else
		{
			diffuseLightFactor = max(dot(unitNormal, unitVertexToLight), 0.0); 
		}
		diffuse += diffuseLightFactor * attenuation * lights.u_Lights[i].Colour;
	}
	
	//Specular
	if(lighting.u_Specular == 1.0)
	{
		float4 lightDirection;
		if(lights.u_Lights[i].Type == 2 && umbra < lights.u_Lights[i].CutOff)
		{
			specular += float4(0, 0, 0, 0);
		}
		else if(lights.u_Lights[i].Type == 1)
		{
			lightDirection = unitLightDirection;
		}
		else
		{
			lightDirection = unitVertexToLight;
		}
		float4 reflecedLightDirection = reflect(lightDirection, unitNormal);
		float specularLightFactor = max(dot(unitVertexToCamera, reflecedLightDirection), 0.0); 
		float dampedFactor = pow(specularLightFactor, lights.u_Lights[i].ShineFactor);
		specular += dampedFactor * lights.u_Lights[i].Reflectivity * attenuation * lights.u_Lights[i].Colour;
	}
	
	//Ambient
	if(lighting.u_Ambient == 1.0)
	{
		ambient += lights.u_Lights[i].AmbientFactor * attenuation * lights.u_Lights[i].Colour;
	}

	//Emit
	float4 emit = {0, 0, 0, 0};
	if(lighting.u_Emit == 1.0)
	{}
}

PS_OUT main(PS_IN IN)
{
	/*r(int i = 0; i < MAX_LIGHTS; i++)
	{
		CalcLighting(i, IN);
	}*/
	
	PS_OUT OUT;

	/*if(IN.v_TextIds > 0)
	{
		for (int i = 0; i < 32; i++)
		{
			int tid = int(IN.v_TextIds - 0.5);
			if(tid == i)
			OUT.colour = ((diffuse + specular + ambient)/MAX_LIGHTS) * u_Textures_image_cis[tid].Sample(u_Textures_sampler_cis[tid], IN.v_TextCoord);
		}	
	}
	else
	{*/
	OUT.colour = uTexture_image_cis.Sample(uTexture_sampler_cis, IN.v_TextCoord);
	/*((diffuse + specular + ambient)/MAX_LIGHTS) **/ 
	/*

	if( IN.v_Colour.r != 0.0 &&
		IN.v_Colour.g != 0.0 &&
		IN.v_Colour.b != 0.0 &&
		IN.v_Colour.a != 0.0 )
	{
		OUT.colour += IN.v_Colour;
	}*/
	
	return OUT;
}