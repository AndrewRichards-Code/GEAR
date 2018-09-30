#pragma once

#include "GL/glew.h"

namespace GEAR {
namespace GRAPHICS {
class VertexBuffer
{
private:
	unsigned int m_VertexID;
	unsigned int m_Count;
	unsigned int m_ComponentCount;

public:
	VertexBuffer(const float* data, unsigned int count, unsigned int componentcount);
	VertexBuffer(const float* data, unsigned int count, unsigned int componentcount, GLenum usage);
	~VertexBuffer();

	void Bind() const;
	void Unbind() const;

	inline unsigned int GetCount() { return m_Count; }
	inline unsigned int GetComponentCount() { return m_ComponentCount; }
};
}
}