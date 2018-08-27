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
	float m_Yaw;
	float m_Pitch;
	float m_Roll;

public:
	ARM::Vec3 m_Position;
	
	ARM::Vec3 m_LookForward;
	ARM::Vec3 m_LookUp;
	ARM::Vec3 m_LookRight;
	
	ARM::Mat4 m_ProjectionMatrix;
	ARM::Mat4 m_ViewMatrix;

private:
	void DefineView();

public:
	Camera(int projType, Shader& shader, ARM::Vec3 pos, float yaw = 0, float pitch = 0, float roll = 0);
	~Camera();

	void CameraPosition();

	void CalcuateForwardAndUp(float yaw, float pitch, float roll);
	void DefineProjection(float fov, float aspectRatio, float zNear, float zFar);
	void DefineProjection(float left, float right, float bottom, float top, float near, float far);
};
}
}

