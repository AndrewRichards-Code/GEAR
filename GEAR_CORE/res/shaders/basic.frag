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

layout(binding = 1) uniform LightParams
{
	vec4 u_LightColour;
	float u_ShineFactor;
	float u_Reflectivity;
	float u_AmbientFactor;
};

//Functions
vec4 diffuse = {0, 0, 0, 0};
vec4 specular = {0, 0, 0, 0};
vec4 ambient = {0, 0, 0, 0};

void CalcLighting()
{
	vec4 unitNormal = normalize(fs_in.v_Normal);
	vec4 unitVertexToLight = normalize(fs_in.v_VertexToLight);
	vec4 unitVertexToCamera = normalize(fs_in.v_VertexToCamera);
	
	//Diffuse
	if(u_SetLighting[0] == 1)
	{
		float diffuseLightFactor = max(dot(unitNormal, unitVertexToLight), 0.0); 
		diffuse = diffuseLightFactor * u_LightColour;
	}
	
	//Specular
	if(u_SetLighting[1] == 1)
	{
		vec4 lightDirection = unitVertexToLight;
		vec4 reflecedLightDirection = reflect(-lightDirection, unitNormal);
		float specularLightFactor = max(dot(unitVertexToCamera, reflecedLightDirection), 0.0); 
		float dampedFactor = pow(specularLightFactor, u_ShineFactor);
		specular = dampedFactor * u_Reflectivity * u_LightColour;
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