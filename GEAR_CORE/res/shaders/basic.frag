#version 450 core

layout(location = 0) out vec4 colour;

in DATA
{
	vec4 v_Position;
	vec2 v_TextCoord;
	float v_TextIds;
	vec4 v_Normal;
	vec4 v_WorldSpace;
	vec4 v_VertexToCamera;
	vec4 v_Colour;
} fs_in;

uniform sampler2D u_Texture;
uniform sampler2D u_Textures[32];

struct Light
{
	int   Type;
	vec4  Colour;
	vec3  Position;
	vec3  Direction;
	float ShineFactor;
	float Reflectivity;
	float AmbientFactor;
	float AttenuationConstant;
	float AttenuationLinear;
	float AttenuationQuadratic;
	float CutOff; 
};

const int MAX_LIGHTS = 8;
uniform Light[MAX_LIGHTS] u_Lights; 

uniform int u_SetLighting[4];

//Functions
vec4 diffuse = {0, 0, 0, 0};
vec4 specular = {0, 0, 0, 0};
vec4 ambient = {0, 0, 0, 0};

void CalcLighting(int i)
{
	//Unit Vector Calculations
	vec4 unitNormal = normalize(fs_in.v_Normal);
	vec4 unitVertexToLight = normalize(fs_in.v_WorldSpace - vec4(u_Lights[i].Position, 0.0));
	vec4 unitLightDirection = normalize(vec4(-u_Lights[i].Direction, 0.0));
	vec4 unitVertexToCamera = normalize(fs_in.v_VertexToCamera);
	
	//Attenuation Calculations
	float VertexToLightDistance = length(fs_in.v_WorldSpace - vec4(u_Lights[i].Position, 0.0));
	float attenuation = 1.0 / (u_Lights[i].AttenuationConstant + (u_Lights[i].AttenuationLinear * VertexToLightDistance) + (u_Lights[i].AttenuationQuadratic * VertexToLightDistance * VertexToLightDistance));
	
	//Spot Calculation
	float umbra = dot(vec4(u_Lights[i].Direction, 0.0), unitVertexToLight); 
		
	//Diffuse
	if(u_SetLighting[0] == 1)
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
		diffuse += diffuseLightFactor * u_Lights[i].Colour;
	}
	
	//Specular
	if(u_SetLighting[1] == 1)
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
		specular += dampedFactor * u_Lights[i].Reflectivity * u_Lights[i].Colour;
	}
	
	//Ambient
	if(u_SetLighting[2] == 1)
	{
		ambient += u_Lights[i].AmbientFactor * u_Lights[i].Colour;
	}

	//Emit
	vec4 emit = {0, 0, 0, 0};
	if(u_SetLighting[3] == 1)
	{}
}

void main()
{
	for(int i = 0; i < MAX_LIGHTS; i++)
	{
		CalcLighting(i);
	}


	if(fs_in.v_TextIds > 0)
	{
		for (int i = 0; i < 32; i++)
		{
			int tid = int(fs_in.v_TextIds - 0.5);
			if(tid == i)
			colour = (diffuse + specular + ambient) * texture(u_Textures[tid], fs_in.v_TextCoord);
		}	
	}
	else
	{
		colour = (diffuse + specular + ambient) * texture(u_Texture, fs_in.v_TextCoord);
	}

	if(fs_in.v_Colour != vec4(0, 0, 0, 0))
	{
		colour += fs_in.v_Colour;
	}
}