#pragma once

#include "gear_core_common.h"
#include "Graphics/Uniformbuffer.h"
#include "Transform.h"

#undef far
#undef near

namespace gear 
{
namespace objects 
{
	class GEAR_API Camera
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
			std::string		debugName;
			void*			device;
			Transform		transform;
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
		typedef graphics::UniformBufferStructures::Camera CameraUB;
		Ref<graphics::Uniformbuffer<CameraUB>> m_UB;

	public:
		CreateInfo m_CI;

		mars::Vec3 m_Direction;
		mars::Vec3 m_Up;
		mars::Vec3 m_Right;

	public:
		Camera(CreateInfo* pCreateInfo);
		~Camera();

		//Update the camera from the current state of Camera::CreateInfo m_CI.
		void Update();

		const Ref<graphics::Uniformbuffer<CameraUB>>& GetUB() const { return m_UB; };

	private:
		void DefineProjection();
		void DefineView();
		void SetPosition();
		void InitialiseUB();
	};
}
}