#pragma once

#include "GL/glew.h"
#include <iostream>
#include <vector>

namespace GEAR {
namespace GRAPHICS {
namespace OPENGL {
	class ShaderStorageBuffer
	{
	private:
		unsigned int m_ShaderStorageID;
		unsigned int m_Size;
		unsigned int m_BindingIndex;

	public:
		ShaderStorageBuffer(unsigned int size, unsigned int bindingIndex);
		~ShaderStorageBuffer();

		void Bind() const;
		void Unbind() const;

		enum ShaderStorageAccess : int
		{
			GEAR_MAP_READ_BIT = GL_MAP_READ_BIT,
			GEAR_MAP_WRITE_BIT = GL_MAP_WRITE_BIT,
			GEAR_MAP_INVALIDATE_RANGE_BIT = GL_MAP_INVALIDATE_RANGE_BIT,
			GEAR_MAP_INVALIDATE_BUFFER_BIT = GL_MAP_INVALIDATE_BUFFER_BIT,
			GEAR_MAP_FLUSH_EXPLICIT_BIT = GL_MAP_FLUSH_EXPLICIT_BIT,
			GEAR_MAP_UNSYNCHRONIZED_BIT = GL_MAP_UNSYNCHRONIZED_BIT,
			GEAR_MAP_PERSISTENT_BIT = GL_MAP_PERSISTENT_BIT,
			GEAR_MAP_COHERENT_BIT = GL_MAP_COHERENT_BIT
		};
		void Access(void* data, unsigned int size, unsigned int offset, ShaderStorageAccess access) const;
		
		void PrintUBOData() const;
		const float* const GetUBOData() const;

		inline unsigned int GetSize() { return m_Size; }
		inline unsigned int GetBindingIndex() { return m_BindingIndex; }
	};
}
}
}