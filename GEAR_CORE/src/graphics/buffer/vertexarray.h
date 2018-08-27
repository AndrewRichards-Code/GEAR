#pragma once

#include <vector>
#include "GL/glew.h"
#include "vertexbuffer.h"

#define GEAR_BUFFER_POSITIONS 0
#define GEAR_BUFFER_TEXTCOORDS 1
#define GEAR_BUFFER_NORMALS 2
#define GEAR_BUFFER_COLOUR 3

namespace GEAR {
namespace GRAPHICS {
class VertexArray
{
private:
	unsigned int m_VertexArrayID;
	std::vector<VertexBuffer*> m_VertexBuffer;

public:
	VertexArray();
	~VertexArray();
	void Bind() const;
	void Unbind() const;

	void AddBuffer(VertexBuffer* vbo, GLuint index) const;
};
}
}