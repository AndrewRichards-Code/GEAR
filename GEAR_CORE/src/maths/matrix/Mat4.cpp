#include "Mat4.h"

namespace ARM
{
	//Constructs a Mat4 of 0.
	Mat4::Mat4()
		:a(0), b(0), c(0), d(0), e(0), f(0), g(0), h(0),
		i(0), j(0), k(0), l(0), m(0), n(0), o(0), p(0) {}

	//Constructs a Mat4 taking a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p.
	Mat4::Mat4(float a, float b, float c, float d, float e, float f, float g, float h,
		float i, float j, float k, float l, float m, float n, float o, float p)
		: a(a), b(b), c(c), d(d), e(e), f(f), g(g), h(h),
		i(i), j(j), k(k), l(l), m(m), n(n), o(o), p(p) {}

	//Constructs a Mat4 from four Vec4s.
	Mat4::Mat4(const Vec4& a, const Vec4& b, const Vec4& c, const Vec4& d)
		: a(a.x), b(a.y), c(a.z), d(a.w), e(b.x), f(b.y), g(b.z), h(b.w),
		i(c.x), j(c.y), k(c.z), l(c.w), m(d.x), n(d.y), o(d.z), p(d.w) {}

	//Constructs a Mat4 where the diagonal is the input.
	Mat4::Mat4(float diagonal)
		: a(diagonal), b(0), c(0), d(0), e(0), f(diagonal), g(0), h(0),
		i(0), j(0), k(diagonal), l(0), m(0), n(0), o(0), p(diagonal) {}

	//Destructs the Mat4.
	Mat4::~Mat4() {}

	//Calcuates determinant, and returns the sum of the vector components.
	float Mat4::Det()
	{
		float temp_i = a * Mat3(f, g, h, j, k, l, n, o, p).Det();
		float temp_j = b * Mat3(e, g, h, i, k, l, m, o, p).Det();
		float temp_k = c * Mat3(e, f, h, i, j, l, m, n, p).Det();
		float temp_l = d * Mat3(e, f, g, i, j, k, m, n, o).Det();
		return temp_i - temp_j + temp_k - temp_l;
	}

	//Calcuates determinant, and returns as a vector.
	Vec4 Mat4::VecDet()
	{
		float temp_i = +a * Mat3(f, g, h, j, k, l, n, o, p).Det();
		float temp_j = -b * Mat3(e, g, h, i, k, l, m, o, p).Det();
		float temp_k = +c * Mat3(e, f, h, i, j, l, m, n, p).Det();
		float temp_l = -d * Mat3(e, f, g, i, j, k, m, n, o).Det();
		return Vec4(temp_i, temp_j, temp_k, temp_l);
	}

	//Swaps the Column/Row Major Ording of the current matrix object.
	void Mat4::Transpose()
	{
		a = a;
		f = f;
		k = k;
		p = p;

		float temp_b = b;
		float temp_c = c;
		float temp_d = d;
		float temp_g = g;
		float temp_h = h;
		float temp_l = l;

		b = e;
		c = i;
		d = m;
		g = j;
		h = n;
		l = o;

		e = temp_b;
		i = temp_c;
		m = temp_d;
		j = temp_g;
		n = temp_h;
		o = temp_l;
	}

	//Swaps the Column/Row Major Ording of the input matrix object, return to a new Mat4 object.
	Mat4 Mat4::Transpose(const Mat4& input)
	{
		return Mat4(input.a, input.e, input.i, input.m, input.b, input.f, input.j, input.n,
			input.c, input.g, input.k, input.o, input.d, input.h, input.l, input.p);
	}

	//Inverts the current matrix object.
	void Mat4::Inverse()
	{
		float det = this->Det();
		if (det == 0.0f)
			return;

		Mat4 result = Mat4
		(
			+Mat3(f, g, h, j, k, l, n, o, p).Det() / det,
			-Mat3(b, c, d, j, k, l, n, o, p).Det() / det,
			+Mat3(b, c, d, f, g, h, n, o, p).Det() / det,
			-Mat3(b, c, d, f, g, h, j, k, l).Det() / det,

			-Mat3(e, g, h, i, k, l, m, o, p).Det() / det,
			+Mat3(a, c, d, i, k, l, m, o, p).Det() / det,
			-Mat3(a, c, d, e, g, h, m, o, p).Det() / det,
			+Mat3(a, c, d, e, g, h, i, k, l).Det() / det,

			+Mat3(e, f, h, i, j, l, m, n, p).Det() / det,
			-Mat3(a, b, d, i, j, l, m, n, p).Det() / det,
			+Mat3(a, b, d, e, f, h, m, n, p).Det() / det,
			-Mat3(a, b, d, e, f, h, i, j, l).Det() / det,

			-Mat3(e, f, g, i, j, k, m, n, o).Det() / det,
			+Mat3(a, b, c, i, j, k, m, n, o).Det() / det,
			-Mat3(a, b, c, e, f, g, m, n, o).Det() / det,
			+Mat3(a, b, c, e, f, g, i, j, k).Det() / det
		);
		*this = result;
	}

	//Inverts the input matrix object, return to a new Mat4 object.
	Mat4 Mat4::Inverse(const Mat4& input)
	{
		Mat4 temp = input;
		float det = temp.Det();
		if (det == 0.0f)
			return input;

		Mat4 result = Mat4
		(
			+Mat3(input.f, input.g, input.h, input.j, input.k, input.l, input.n, input.o, input.p).Det() / det,
			-Mat3(input.b, input.c, input.d, input.j, input.k, input.l, input.n, input.o, input.p).Det() / det,
			+Mat3(input.b, input.c, input.d, input.f, input.g, input.h, input.n, input.o, input.p).Det() / det,
			-Mat3(input.b, input.c, input.d, input.f, input.g, input.h, input.j, input.k, input.l).Det() / det,

			-Mat3(input.e, input.g, input.h, input.i, input.k, input.l, input.m, input.o, input.p).Det() / det,
			+Mat3(input.a, input.c, input.d, input.i, input.k, input.l, input.m, input.o, input.p).Det() / det,
			-Mat3(input.a, input.c, input.d, input.e, input.g, input.h, input.m, input.o, input.p).Det() / det,
			+Mat3(input.a, input.c, input.d, input.e, input.g, input.h, input.i, input.k, input.l).Det() / det,

			+Mat3(input.e, input.f, input.h, input.i, input.j, input.l, input.m, input.n, input.p).Det() / det,
			-Mat3(input.a, input.b, input.d, input.i, input.j, input.l, input.m, input.n, input.p).Det() / det,
			+Mat3(input.a, input.b, input.d, input.e, input.f, input.h, input.m, input.n, input.p).Det() / det,
			-Mat3(input.a, input.b, input.d, input.e, input.f, input.h, input.i, input.j, input.l).Det() / det,

			-Mat3(input.e, input.f, input.g, input.i, input.j, input.k, input.m, input.n, input.o).Det() / det,
			+Mat3(input.a, input.b, input.c, input.i, input.j, input.k, input.m, input.n, input.o).Det() / det,
			-Mat3(input.a, input.b, input.c, input.e, input.f, input.g, input.m, input.n, input.o).Det() / det,
			+Mat3(input.a, input.b, input.c, input.e, input.f, input.g, input.i, input.j, input.k).Det() / det
		);
		return result;
	}

	//Constructs a Mat4 where the diagonal is 1.
	Mat4 Mat4::Identity()
	{
		return Mat4(1);
	}

	//Constructs a orthographic matrix (Mat4).
	Mat4 Mat4::Orthographic(float left, float right, float bottom, float top, float near, float far)
	{
		return Mat4((2 / (right - left)), (0), (0), (-(right + left) / (right - left)),
			           (0), (2 / (top - bottom)), (0), (-(top + bottom) / (top - bottom)),
			           (0), (0), (-2 / (far - near)), (-(far + near) / (far - near)),
			           (0), (0), (0),(1));
	}
	//Constructs a perspective matrix (Mat4). Input fov is in radians.
	Mat4 Mat4::Perspective(double fov, float aspectRatio, float near, float far)
	{
		return Mat4 ((1 / (aspectRatio * static_cast<float>(tan(fov / 2)))), (0), (0), (0),
					   (0), (1 / static_cast<float>(tan(fov / 2))), (0), (0),
					   (0), (0), -((far + near) / (far - near)), -((2 * far * near) / (far - near)),
					   (0), (0), (-1), (0));
	}

	//Constructs a translation matrix (Mat4).
	Mat4 Mat4::Translation(const Vec3& translation)
	{
		Mat4 result(1);
		result.d = translation.x;
		result.h = translation.y;
		result.l = translation.z;
		return result;
	}

	//Constructs a rotation matrix (Mat4). Input angle is in radians.
	Mat4 Mat4::Rotation(double angle, const Vec3& axis)
	{
		Mat4 result(1);
		float c_angle = static_cast<float>(cos(angle));
		float s_angle = static_cast<float>(sin(angle));
		float omcos   = static_cast<float>(1 - c_angle);

		float x = axis.x;
		float y = axis.y;
		float z = axis.z;

		result.a = x * x * omcos + c_angle;
		result.e = x * y * omcos + z * s_angle;
		result.i = x * z * omcos - y * s_angle;
		result.m = 0;

		result.b = y * x * omcos - z * s_angle;
		result.f = y * y * omcos + c_angle;
		result.j = y * z * omcos + x * s_angle;
		result.n = 0;

		result.c = z * x * omcos + y * s_angle;
		result.g = z * y * omcos - x * s_angle;
		result.k = z * z * omcos + c_angle;
		result.o = 0;

		result.d = 0;
		result.h = 0;
		result.l = 0;
		result.p = 1;

		return result;
	}

	//Constructs a scale matrix (Mat4).
	Mat4 Mat4::Scale(const Vec3& scale)
	{
		Mat4 result(1);
		result.a = scale.x;
		result.f = scale.y;
		result.k = scale.z;
		return result;
	}
}
