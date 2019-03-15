#pragma once

#include "gear_common.h"
#include "GL/glew.h"
#include "vertexbuffer.h"

namespace GEAR {
namespace GRAPHICS {
namespace OPENGL {
	class VertexArray
	{
	private:
		unsigned int m_VertexArrayID;
		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffer;

	public:
		VertexArray();
		~VertexArray();
		void Bind() const;
		void Unbind() const;

		enum class BufferType : GLuint
		{
			GEAR_BUFFER_POSITIONS = 0, 
			GEAR_BUFFER_TEXCOORDS = 1,
			GEAR_BUFFER_TEXIDS = 2,
			GEAR_BUFFER_NORMALS = 3, 
			GEAR_BUFFER_COLOURS = 4 
		};
		void AddBuffer(std::shared_ptr<VertexBuffer> vbo, BufferType type);
	};
}
}
}