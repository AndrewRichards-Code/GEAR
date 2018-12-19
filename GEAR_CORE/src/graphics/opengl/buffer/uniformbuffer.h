#pragma once

#include "GL/glew.h"
#include <iostream>
#include <vector>

namespace GEAR {
namespace GRAPHICS {
namespace OPENGL {
	class UniformBuffer
	{
	private:
		unsigned int m_UniformID;
		unsigned int m_Size;
		unsigned int m_BindingIndex;

	public:
		UniformBuffer(unsigned int count, unsigned int componentcount);
		~UniformBuffer();

		void Bind() const;
		void Unbind() const;

		void SubDataBind(const float * data, unsigned int size, unsigned int offset) const;

		inline unsigned int GetSize() { return m_Size; }
		inline unsigned int GetBindingIndex() { return m_BindingIndex; }
	};
}
}
}