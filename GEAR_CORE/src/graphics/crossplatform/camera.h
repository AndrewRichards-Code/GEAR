#pragma once

#include "gear_common.h"
#include "graphics/opengl/buffer/buffermanager.h"
#include "ARMLib.h"

#define GEAR_CAMERA_PERSPECTIVE 0
#define GEAR_CAMERA_ORTHOGRAPHIC 1

namespace GEAR {
namespace GRAPHICS {
namespace CROSSPLATFORM {

#ifdef GEAR_OPENGL

class Camera
{
private:
	int m_ProjectionType;
	double m_Yaw = 0;
	double m_Pitch = 0;
	double m_Roll = 0;

	const ARM::Vec3 m_xAxis = ARM::Vec3(1, 0, 0);
	const ARM::Vec3 m_yAxis = ARM::Vec3(0, 1, 0);
	const ARM::Vec3 m_zAxis = ARM::Vec3(0, 0, 1);

public:
	ARM::Vec3 m_Position;

	ARM::Vec3 m_Forward;
	ARM::Vec3 m_Up;
	ARM::Vec3 m_Right;

private:
	static bool s_InitialiseUBO;
	struct CameraUBO
	{
		ARM::Mat4 m_ProjectionMatrix;
		ARM::Mat4 m_ViewMatrix;
		ARM::Vec4 m_Position;
		
	} m_CameraUBO;

public:
	Camera(int projType, const ARM::Vec3& position, const ARM::Vec3& forward, const ARM::Vec3 up);
	~Camera();

	void UpdateCameraPosition();
	void CalcuateLookAround(double yaw, double pitch, double roll, bool invertYAxis = false);

	void DefineView();
	void DefineView(const ARM::Mat4& viewMatrix);
	void DefineProjection(double fov, float aspectRatio, float zNear, float zFar, bool flipX = false, bool flipY = false);
	void DefineProjection(float left, float right, float bottom, float top, float near, float far);

private:
	void CalculateRight();
	void CalculateUp();
	void InitialiseUBO();
};
}
}
}
#endif