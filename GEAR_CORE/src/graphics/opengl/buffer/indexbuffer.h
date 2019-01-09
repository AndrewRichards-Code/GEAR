#pragma once

#include "GL/glew.h"


namespace GEAR {
namespace GRAPHICS {
namespace OPENGL {
class IndexBuffer
{
private:
	unsigned int m_IndexID;
	unsigned int m_Count;

public:
	IndexBuffer() {};
	IndexBuffer(const unsigned int* data, unsigned int count);
	~IndexBuffer();

	void Bind() const;
	void Unbind() const;

	inline unsigned int GetCount() const { return m_Count; }
};
}
}
}