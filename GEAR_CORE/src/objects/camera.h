#pragma once

#include "gear_core_common.h"
#include "graphics/uniformbuffer.h"
#include "mars.h"


#undef far
#undef near

namespace gear {
namespace objects {

class Camera
{
public:
	enum class ProjectionType : uint32_t
	{
		PERSPECTIVE,
		ORTHOGRAPHIC 
	};

	struct OrthographicParameters
	{
		float	left;
		float	right;
		float	bottom;
		float	top;
		float	near;
		float	far;
	};
	struct PerspectiveParameters
	{
		double	horizonalFOV;
		float	aspectRatio;
		float	zNear;
		float	zFar;
	};

	struct CreateInfo
	{
		const char*		debugName;
		void*			device;
		mars::Vec3		position;
		mars::Quat		orientation;
		ProjectionType	projectionType;
		union
		{
			OrthographicParameters	orthographicsParams;
			PerspectiveParameters	perspectiveParams;
		};
		bool			flipX;
		bool			flipY;
	};

private:
	struct CameraUB
	{
		mars::Mat4 projectionMatrix;
		mars::Mat4 viewMatrix;
		mars::Vec4 position;
		
	};
	gear::Ref<graphics::UniformBuffer<CameraUB>> m_UB;

	std::string m_DebugName;

public:
	CreateInfo m_CI;

	mars::Vec3 m_Direction;
	mars::Vec3 m_Up;
	mars::Vec3 m_Right;

public:
	Camera(CreateInfo* pCreateInfo);
	~Camera();

	//Update the camera the current static of Camera::CreateInfo m_CI.
	void Update();
	gear::Ref<graphics::UniformBuffer<CameraUB>> GetUB() { return m_UB; };

private:
	void DefineProjection();
	void DefineView();
	void SetPosition();
	void InitialiseUB();
};
}
}