#include "msc_common.h"
#include "UniformBufferStructures.h"
#include "ColourFunctions.h"

MIRU_RW_IMAGE_2D(0, 0, float4, inputImage);
MIRU_RW_IMAGE_2D(0, 1, float4, outputImage);
MIRU_UNIFORM_BUFFER(0, 2, BloomInfo, bloomInfo);

MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_2D, 0, 0, float4, inputImageRO);

MIRU_COMPUTE_LAYOUT(8, 8, 1)
void prefilter(uint3 pos : SV_DispatchThreadID)//MIRU_DISPATCH_THREAD_ID)
{
	float4 inputColour = inputImage[pos.xy];
	float threshold = bloomInfo.threshold;
	
	float4 finalColour;
	finalColour.r = inputColour.r > threshold ? inputColour.r : 0.0;
	finalColour.g = inputColour.g > threshold ? inputColour.g : 0.0;
	finalColour.b = inputColour.b > threshold ? inputColour.b : 0.0;
	
	float weight = 1.0 / (1.0 + GetLuminance(0, finalColour.rgb));
	finalColour.rgb *= weight;
	finalColour.a = 1.0;
	
	
	outputImage[pos.xy] = finalColour;
}

// Better, temporally stable box filtering
// [Jimenez14] http://goo.gl/eomGso
// . . . . . . .
// . A . B . C .
// . . D . E . .
// . F . G . H .
// . . I . J . .
// . K . L . M .
// . . . . . . .
float4 ColateColourBoxSample13(Texture2D image, SamplerState imageSampler, float2 texCoords, float2 imageSize)
{
	float2 scale = 1.0 / imageSize; //Scale pixel coordinates to texture coordinates.
	
	float4 A = image.SampleLevel(imageSampler, texCoords + float2(-1.0, -1.0) * scale, 0.0);
	float4 B = image.SampleLevel(imageSampler, texCoords + float2( 0.0, -1.0) * scale, 0.0);
	float4 C = image.SampleLevel(imageSampler, texCoords + float2( 1.0, -1.0) * scale, 0.0);
	float4 D = image.SampleLevel(imageSampler, texCoords + float2(-0.5, -0.5) * scale, 0.0);
	float4 E = image.SampleLevel(imageSampler, texCoords + float2( 0.5, -0.5) * scale, 0.0);
	float4 F = image.SampleLevel(imageSampler, texCoords + float2(-1.0,  0.0) * scale, 0.0);
	float4 G = image.SampleLevel(imageSampler, texCoords + float2( 0.0,  0.0) * scale, 0.0);
	float4 H = image.SampleLevel(imageSampler, texCoords + float2( 1.0,  0.0) * scale, 0.0);
	float4 I = image.SampleLevel(imageSampler, texCoords + float2(-0.5,  0.5) * scale, 0.0);
	float4 J = image.SampleLevel(imageSampler, texCoords + float2( 0.5,  0.5) * scale, 0.0);
	float4 K = image.SampleLevel(imageSampler, texCoords + float2(-1.0,  1.0) * scale, 0.0);
	float4 L = image.SampleLevel(imageSampler, texCoords + float2( 0.0,  1.0) * scale, 0.0);
	float4 M = image.SampleLevel(imageSampler, texCoords + float2( 1.0,  1.0) * scale, 0.0);
	
	float4 result = float4(0, 0, 0, 0);
	result += ((D + E + I + J) / 4.0) * 0.5;
	result += ((A + B + G + F) / 4.0) * 0.125;
	result += ((B + C + H + G) / 4.0) * 0.125;
	result += ((F + G + L + K) / 4.0) * 0.125;
	result += ((G + H + M + L) / 4.0) * 0.125;

	return result;
}

MIRU_COMPUTE_LAYOUT(8, 8, 1)
void downsample(uint3 pos : MIRU_DISPATCH_THREAD_ID)
{
	float2 imageSize;
	inputImageRO_ImageCIS.GetDimensions(imageSize.x, imageSize.y);
	float2 texCoords = 2.0 * float2(pos.xy) / imageSize;
	outputImage[pos.xy] = ColateColourBoxSample13(inputImageRO_ImageCIS, inputImageRO_SamplerCIS, texCoords, imageSize);
}

// Tent filtering
// [Jimenez14] http://goo.gl/eomGso
// . . . . . . .
// . A . B . C .
// . . . . . . .
// . D . E . F .
// . . . . . . .
// . G . H . I .
// . . . . . . .
float4 ColateColourTentSample9(Texture2D image, SamplerState imageSampler, float2 texCoords, float2 imageSize, float sampleScale)
{
	float2 scale = sampleScale / imageSize; //Scale pixel coordinates to texture coordinates with a genereal sample scale to the tent radius.
	
	float4 A = image.SampleLevel(imageSampler, texCoords + float2(-1.0, -1.0) * scale, 0.0) * 1.0;
	float4 B = image.SampleLevel(imageSampler, texCoords + float2( 0.0, -1.0) * scale, 0.0) * 2.0;
	float4 C = image.SampleLevel(imageSampler, texCoords + float2( 1.0, -1.0) * scale, 0.0) * 1.0;
	float4 D = image.SampleLevel(imageSampler, texCoords + float2(-1.0,  0.0) * scale, 0.0) * 2.0;
	float4 E = image.SampleLevel(imageSampler, texCoords + float2( 0.0,  0.0) * scale, 0.0) * 4.0;
	float4 F = image.SampleLevel(imageSampler, texCoords + float2( 1.0,  0.0) * scale, 0.0) * 2.0;
	float4 G = image.SampleLevel(imageSampler, texCoords + float2(-1.0,  1.0) * scale, 0.0) * 1.0;
	float4 H = image.SampleLevel(imageSampler, texCoords + float2( 0.0,  1.0) * scale, 0.0) * 2.0;
	float4 I = image.SampleLevel(imageSampler, texCoords + float2( 1.0,  1.0) * scale, 0.0) * 1.0;
	
	float4 result = A + B + C + D + E + F + G + H + I;
	result /= 16.0;
	
	return result;
}

MIRU_COMPUTE_LAYOUT(8, 8, 1)
void upsample(uint3 pos : MIRU_DISPATCH_THREAD_ID)
{
	float2 imageSize;
	inputImageRO_ImageCIS.GetDimensions(imageSize.x, imageSize.y);
	float2 texCoords = 0.5 * float2(pos.xy) / imageSize;
	outputImage[pos.xy] += ColateColourTentSample9(inputImageRO_ImageCIS, inputImageRO_SamplerCIS, texCoords, imageSize, bloomInfo.upsampleScale);
}