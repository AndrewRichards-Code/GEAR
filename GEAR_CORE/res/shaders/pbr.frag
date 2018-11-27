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

uniform vec4 u_LightColour;
uniform vec3 u_F0;
uniform vec3 u_Albedo;
uniform float u_Metallic;
uniform float u_Roughness;
uniform float u_AO;

const float pi = 3.14151926535;

vec3 Fresnel(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = pi * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 CalcPBRLighting()
{
	vec3 lightOut = {0, 0, 0};
	
	vec4 unitNormal = normalize(fs_in.v_Normal);
	vec4 unitVertexToCamera = normalize(fs_in.v_VertexToCamera);
	vec4 unitVertexToLight = -normalize(fs_in.v_VertexToLight);
	vec4 unitHalfVector = normalize(unitNormal + unitVertexToCamera);

	float distanceVertToCam = length(unitVertexToCamera);
	float attenuation =  1.0 / (distanceVertToCam * distanceVertToCam);
	vec3 radiance = u_LightColour.xyz * attenuation;

	vec3 F0 = mix(u_F0, u_Albedo, u_Metallic);
	vec3 F  = Fresnel(max(dot(unitHalfVector, unitVertexToCamera), 0.0), F0);

	float NDF = DistributionGGX(unitNormal.xyz, unitHalfVector.xyz, u_Roughness);
	float G = GeometrySmith(unitNormal.xyz, unitVertexToCamera.xyz, unitVertexToLight.xyz, u_Roughness);

	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * (max(dot(unitNormal, unitVertexToCamera), 0.0) * max(dot(unitNormal, unitVertexToLight), 0.0));
	vec3 specular = numerator / max(denominator, 0.001); 

	vec3 kSpecular = F;
	vec3 kDiffused = vec3(1.0) - kSpecular;
	kDiffused *= 1.0 - u_Metallic;

	lightOut = ((kDiffused  * u_Albedo / pi) + specular) * radiance * max(dot(unitNormal, unitVertexToLight), 0.0);

	vec3 ambient = vec3(0.03) * u_Albedo * u_AO;
	vec3 colourOut  = ambient + lightOut; 

	colourOut = colourOut / (colourOut + vec3(1.0));
	colourOut = pow(colourOut, vec3(1.0/2.2)); 

	return colourOut;
}

void main()
{
	vec3 lighting = CalcPBRLighting();

	if(fs_in.v_TextIds > 0)
	{
		for (int i = 0; i < 32; i++)
		{
			int tid = int(fs_in.v_TextIds - 0.5);
			if(tid == i)
				colour = vec4(lighting, 1.0) + texture(u_Textures[tid], fs_in.v_TextCoord);
		}	
	}
	else
	{
		colour = vec4(lighting, 1.0) + texture(u_Texture, fs_in.v_TextCoord);
	}

	if(fs_in.v_Colour != vec4(0, 0, 0, 0))
	{
		colour += fs_in.v_Colour;
	}
}