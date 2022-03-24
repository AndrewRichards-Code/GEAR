#pragma once
#include "gear_core_common.h"
#include "ObjectInterface.h"
#include "Graphics/Uniformbuffer.h"
#include "Transform.h"

#undef far
#undef near

namespace gear 
{
namespace objects 
{
	class GEAR_API Camera : public ObjectInterface
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

		struct CreateInfo : public ObjectInterface::CreateInfo
		{
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

		mars::float3 m_Direction;
		mars::float3 m_Up;
		mars::float3 m_Right;

	public:
		Camera(CreateInfo* pCreateInfo);
		~Camera();

		const Ref<graphics::Uniformbuffer<CameraUB>>& GetUB() const { return m_UB; };

		//Update the camera from the current state of Camera::CreateInfo m_CI.
		void Update(const Transform& transform) override;

		//The view matrix is already inversed and should be passed unmodified to a UniformBuffer's view matrix member.
		static mars::float4x4 GetCubemapFaceViewMatrix(uint32_t faceIndex, const mars::float3& translation = mars::float3(0.0f, 0.0f, 0.0f));
		
	protected:
		bool CreateInfoHasChanged(const ObjectInterface::CreateInfo* pCreateInfo) override;

	private:
		void DefineProjection();
		void DefineView(const Transform& transform);
		void SetPosition(const Transform& transform);
		void InitialiseUB();
	};
}
}