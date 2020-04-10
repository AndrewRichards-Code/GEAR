#pragma once

#include "gear_common.h"
#include "graphics/opengl/buffer/buffermanager.h"
#include "mars.h"

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

	const mars::Vec3 m_xAxis = mars::Vec3(1, 0, 0);
	const mars::Vec3 m_yAxis = mars::Vec3(0, 1, 0);
	const mars::Vec3 m_zAxis = mars::Vec3(0, 0, 1);

public:
	mars::Vec3 m_Position;

	mars::Vec3 m_Forward;
	mars::Vec3 m_Up;
	mars::Vec3 m_Right;

private:
	static bool s_InitialiseUBO;
	struct CameraUBO
	{
		mars::Mat4 m_ProjectionMatrix;
		mars::Mat4 m_ViewMatrix;
		mars::Vec4 m_Position;
		
	} m_CameraUBO;

public:
	Camera(int projType, const mars::Vec3& position, const mars::Vec3& forward, const mars::Vec3 up);
	~Camera();

	void UpdateCameraPosition();
	void CalcuateLookAround(double yaw, double pitch, double roll, bool invertYAxis = false);

	void DefineView();
	void DefineView(const mars::Mat4& viewMatrix);
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