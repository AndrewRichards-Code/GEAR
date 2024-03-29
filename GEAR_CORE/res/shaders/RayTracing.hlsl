//Ray Generation
RWTexture2D<float4> output : register(u0, space0);

struct SceneConstants
{
	struct Camera
	{
		float4x4 viewMatrix;
		float3 position;
		float aspectRatio;
		float4 direction;
	} camera;
	float TMin;
	float TMax;
};
ConstantBuffer<SceneConstants> scene : register(b0, space0);

struct RTShaderConstants
{
	uint RayFlags;
	uint HitGroupIndex;
	uint NumHitGroups;
	uint MissShaderIndex;
};
ConstantBuffer<RTShaderConstants> rtShaders : register(b1, space0);

RaytracingAccelerationStructure accelStruct : register(t0, space0);

struct Payload
{
	float4 colour;
};

float3 GetCameraToPixelRayDirection(SceneConstants::Camera camera)
{
	uint2 pixelID = DispatchRaysIndex().xy;
	uint2 dimension = DispatchRaysDimensions().xy;
	uint2 normalisedCentrePixelID = 2.0 * (float2(pixelID - 1)/float2(dimension - 1)) - float2(1.0, 1.0);
	float4 scaledViewPixelID_F = normalize(float4(normalisedCentrePixelID.x * camera.aspectRatio, normalisedCentrePixelID.y, -1.0, 0.0)); 
	float3 viewDirection = normalize(mul(camera.viewMatrix, scaledViewPixelID_F)).xyz;
	return viewDirection;
}

[shader("raygeneration")]
void ray_generation_main()
{
	RayDesc ray;
	ray.Origin = scene.camera.position;
	ray.TMin = scene.TMin;
	ray.Direction = GetCameraToPixelRayDirection(scene.camera);
	ray.TMax = scene.TMax;
	
	Payload payload = { float4(0,0,0,0) };
	
	TraceRay( scene, rtShaders.RayFlags, 0xFF, 
				rtShaders.HitGroupIndex, rtShaders.NumHitGroups, rtShaders.MissShaderIndex,
				ray, payload );
	
	output[DispatchRaysIndex().xy] = payload.colour;

}

//Any-Hit
[shader("anyhit")]
void any_hit_main(inout Payload payload, in BuiltInTriangleIntersectionAttributes  attr)
{
	payload.colour = float4(1.0, 0.0, 0.0, 1.0);
}

//Closest-Hit
[shader("closesthit")]
void closest_hit_main(inout Payload payload, in BuiltInTriangleIntersectionAttributes  attr)
{
	payload.colour = float4(0.0, 1.0, 0.0, 1.0);
}

//Miss
[shader("miss")]
void miss_main(inout Payload payload)
{
	payload.colour = float4(0.0, 0.0, 0.0, 1.0);
}