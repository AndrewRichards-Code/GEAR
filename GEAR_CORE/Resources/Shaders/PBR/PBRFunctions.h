#pragma once

static const float PI = 3.1415926535897932384626433832795;
static const float TAU = 2.0 * PI;

////////////////////////////////
//------Fresnel FUNCTIONS-----//
////////////////////////////////
float3 FresnelSchlick(float3 F0, float3 Wo, float3 N)
{
	return F0 + (1.0 - F0) * pow(max(1.0 - dot(Wo, N), 0.0), 5.0);
}

////////////////////////////////
//---DISTRUBUTION FUNCTIONS---//
////////////////////////////////

//GGX/Towbridge-Reitz normal distribution function. Uses Disney's reparametrization of alpha = roughness^2.
float GGXTR(float cosWhN, float roughness)
{
	float alpha = roughness * roughness;
	float alphaSq = alpha * alpha;
	float denom = (cosWhN * cosWhN) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

//Importance sample GGX normal distribution function for a fixed roughness value.
//This returns normalized half-vector between Wi & Wo.
//For derivation see: http://blog.tobias-franke.eu/2014/03/30/notes_on_importance_sampling.html
float3 SampleGGX(float2 hammersleySample, float roughness)
{
	float alpha = roughness * roughness;

	float cosTheta = sqrt((1.0 - hammersleySample.y) / (1.0 + (alpha * alpha - 1.0) * hammersleySample.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	float phi = TAU * hammersleySample.x;

	//Convert to Cartesian.
	return float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
}

////////////////////////////////
//-----GEOMETRIC FUNCTIONS----//
////////////////////////////////

//Single term for separate Schlick-GGX
float SchlickGGXSub(float cosWoN, float k)
{
	return cosWoN / ((cosWoN * (1 - k)) + k);
}

//Schlick-GGX approximation of geometric attenuation function using Smith's method.
float SchlickGGX(float cosWiN, float cosWoN, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0; //Epic suggests using this roughness remapping for analytic lights.
	return SchlickGGXSub(cosWiN, k) * SchlickGGXSub(cosWoN, k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float SchlickGGX_IBL(float cosWiN, float cosWoN, float roughness)
{
	float r = roughness;
	float k = (r * r) / 2.0; //Epic suggests using this roughness remapping IBL lights.
	return SchlickGGXSub(cosWiN, k) * SchlickGGXSub(cosWoN, k);
}

////////////////////////////////
//--- HAMMERSLEY FUNCTIONS ---//
////////////////////////////////

//Compute Van der Corput radical inverse See: http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
float RadicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

//Sample i-th point from Hammersley point set of NumSamples points total.
float2 SampleHammersley(uint i, float NumSamples)
{
	return float2((float(i) / NumSamples), RadicalInverse_VdC(i));
}

////////////////////////////////
//--- HEMISPHERE FUNCTIONS ---//
////////////////////////////////

//Uniformly sample point on a hemisphere.
//See: "Physically Based Rendering" 3rd ed., section 13.6.1.
float3 SampleHemisphere(float2 hammersleySample)
{
	const float cosTheta = sqrt(max(1.0 - hammersleySample.x * hammersleySample.x, 0.0));
	return float3(cos(TAU * hammersleySample.y) * cosTheta, sin(TAU * hammersleySample.y) * cosTheta, hammersleySample.x);
}