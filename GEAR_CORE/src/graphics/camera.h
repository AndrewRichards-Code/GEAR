#pragma once

#include <iostream>
#include "shader.h"
#include "../maths/ARMLib.h"

#define GEAR_CAMERA_PERSPECTIVE 0
#define GEAR_CAMERA_ORTHOGRAPHIC 1

namespace GEAR {
namespace GRAPHICS {


class Camera
{
private:
	int m_ProjectionType;
	Shader& m_Shader;
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
	
	ARM::Mat4 m_ProjectionMatrix;
	ARM::Mat4 m_ViewMatrix;
	
public:
	Camera(int projType, Shader& shader, const ARM::Vec3& position, const ARM::Vec3& forward, const ARM::Vec3 up);
	~Camera();

private:
	void CalculateRight();
	void Move(const ARM::Vec3& direction, float value);
	void RotateX(double pitch);
	void RotateY(double yaw);
	ARM::Mat4 CameraMatrix(const ARM::Vec3& camPos, const ARM::Vec3& camTarget, const ARM::Vec3& up);

public:
	void UpdateCameraPosition();
	void CalcuateLookAround(double yaw, double pitch, double roll, bool invertYAxis = false);

	void DefineView();
	void DefineProjection(double fov, float aspectRatio, float zNear, float zFar);
	void DefineProjection(float left, float right, float bottom, float top, float near, float far);

};
}
}

