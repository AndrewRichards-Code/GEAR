#include "gear_core_common.h"
#include "Graphics/Picker.h"
#include "Objects/Camera.h"
#include "Objects/Light.h"
#include "Objects/Model.h"
#include "Objects/Mesh.h"

#include <numeric>

using namespace gear;
using namespace graphics;
using namespace objects;

using namespace miru::base;

using namespace mars;

Picker::Ray Picker::GetPickingRay(const Ref<Camera>& camera, const float2& pixelCoords, const float2& viewportSize)
{
	float2 ndc = float2(pixelCoords.x / viewportSize.x, pixelCoords.y / viewportSize.y) * 2.0f - float2(1.0f, 1.0f);
	float4 clipCoords = float4(ndc, float2(1.0f, 1.0f));

	const Ref<Uniformbuffer<UniformBufferStructures::Camera>>& cameraUB = camera->GetCameraUB();
	float4x4 projInverse = float4x4::Inverse(cameraUB->proj);
	float4 viewSpaceCoords = projInverse * clipCoords;
	viewSpaceCoords.z = 1.0f;
	viewSpaceCoords.w = 0.0f;

	float4x4 viewInverse = float4x4::Inverse(cameraUB->view);
	float3 worldCoords = viewInverse * viewSpaceCoords;
	worldCoords.Normalise();

	return { cameraUB->position, worldCoords, 1.0f };
}

AabbData Picker::GetAABB(const Ref<Model>& model)
{
	const Ref<Mesh>& mesh = model->GetMesh();
	const float4x4& modl = model->GetModlMatrix();

	AabbData aabb = {
		+std::numeric_limits<float>::max(),
		+std::numeric_limits<float>::max(),
		+std::numeric_limits<float>::max(),
		-std::numeric_limits<float>::max(),
		-std::numeric_limits<float>::max(),
		-std::numeric_limits<float>::max()
	};

	for (const auto& modelMesh : mesh->m_CI.modelData.meshes)
	{
		for (const auto& vertex : modelMesh.vertices)
		{
			const float3 worldPosition = modl * vertex.position;
			aabb.minX = std::min(aabb.minX, worldPosition.x);
			aabb.minY = std::min(aabb.minY, worldPosition.y);
			aabb.minZ = std::min(aabb.minZ, worldPosition.z);
			aabb.maxX = std::max(aabb.maxX, worldPosition.x);
			aabb.maxY = std::max(aabb.maxY, worldPosition.y);
			aabb.maxZ = std::max(aabb.maxZ, worldPosition.z);
		}
	}
	return aabb;
}

float Picker::RayIntersectsAABB(const miru::base::AabbData& aabb, const Ray& ray)
{
	//https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection.html

	const float3& min = { aabb.minX, aabb.minY, aabb.minZ };
	const float3& max = { aabb.maxX, aabb.maxY, aabb.maxZ };

	float tmin = (min.x - ray.origin.x) / ray.direction.x;
	float tmax = (max.x - ray.origin.x) / ray.direction.x;

	if (tmin > tmax) 
		std::swap(tmin, tmax);

	float tymin = (min.y - ray.origin.y) / ray.direction.y;
	float tymax = (max.y - ray.origin.y) / ray.direction.y;

	if (tymin > tymax) 
		std::swap(tymin, tymax);

	if ((tmin > tymax) || (tymin > tmax))
		return std::numeric_limits<float>::max();

	if (tymin > tmin)
		tmin = tymin;

	if (tymax < tmax)
		tmax = tymax;

	float tzmin = (min.z - ray.origin.z) / ray.direction.z;
	float tzmax = (max.z - ray.origin.z) / ray.direction.z;

	if (tzmin > tzmax) 
		std::swap(tzmin, tzmax);

	if ((tmin > tzmax) || (tzmin > tmax))
		return std::numeric_limits<float>::max();

	if (tzmin > tmin)
		tmin = tzmin;

	if (tzmax < tmax)
		tmax = tzmax;

	return tmin;
}

float Picker::RayIntersectsModelGeometry(const Ref<Model>& model, const Ray& ray)
{
	const Ref<Mesh>& mesh = model->GetMesh();
	const float4x4& modl = model->GetModlMatrix();

	for (const auto& modelMesh : mesh->m_CI.modelData.meshes)
	{
		for (size_t i = 0; i < modelMesh.indices.size(); i = i + 3)
		{
			//https://www.braynzarsoft.net/viewtutorial/q16390-24-picking

			const uint32_t& index0 = modelMesh.indices[i + 0];
			const uint32_t& index1 = modelMesh.indices[i + 1];
			const uint32_t& index2 = modelMesh.indices[i + 2];

			const float3 vertex0 = modl * modelMesh.vertices[index0].position;
			const float3 vertex1 = modl * modelMesh.vertices[index1].position;
			const float3 vertex2 = modl * modelMesh.vertices[index2].position;

			const float3 normal = float3::Cross((vertex1 - vertex0), (vertex2 - vertex0)).Normalise();

			//Plane equation: Ax + By + Cz + D = 0
			float D = -float3::Dot<float>(normal, vertex0);

			float p1 = float3::Dot<float>(ray.origin, normal);
			float p2 = float3::Dot<float>(ray.direction, normal);

			float t = 0.0f;
			if (p2 != 0.0f)
			{
				 t = -(p1 + D) / p2;
				if (t > 0.0f)
				{
					return t;
				}
			}
		}
	}

	return std::numeric_limits<float>::max();
}

Ref<Model> Picker::GetNearestModel(const std::vector<Ref<Model>>& models, const Ref<Camera>& camera, const float2& pixelCoords, const float2& viewportSize)
{
	Ray pickingRay = GetPickingRay(camera, pixelCoords, viewportSize);

	Ref<Model> nearestModel = nullptr;
	float shortestLength = std::numeric_limits<float>::max();

	for (const auto& model : models)
	{
		AabbData aabb = GetAABB(model);
		float length = RayIntersectsAABB(aabb, pickingRay);
		bool hit = length < std::numeric_limits<float>::max();
		bool closer = length <= shortestLength;

		if (hit && closer)
		{
			length = RayIntersectsModelGeometry(model, pickingRay);
			hit = length < std::numeric_limits<float>::max();
			closer = length <= shortestLength;
			if (hit && closer)
			{
				shortestLength = length;
				nearestModel = model;
			}
		}
	}

	return nearestModel;
}

Ref<Light> Picker::GetNearestLight(const std::vector<Ref<Light>>& lights, const Ref<Camera>& camera, const float2& pixelCoords, const float2& viewportSize)
{
	Ray pickingRay = GetPickingRay(camera, pixelCoords, viewportSize);

	Ref<Light> nearestLight = nullptr;
	float shortestLength = std::numeric_limits<float>::max();

	for (const auto& light : lights)
	{
		const float3& position = light->GetUB()->lights[light->GetLightID()].position;
		const float3& direction = position - pickingRay.origin;
		const float& length = direction.Length<float>();
		float dot = float3::Dot<float>(float3::Normalise(direction), pickingRay.direction);

		bool hit = dot > 0.9995;
		bool closer = length <= shortestLength;

		if (hit && closer)
		{
			shortestLength = length;
			nearestLight = light;
		}
	}

	return nearestLight;
}
