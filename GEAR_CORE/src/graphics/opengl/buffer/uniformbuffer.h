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
		UniformBuffer(unsigned int size, unsigned int bindingIndex);
		~UniformBuffer();

		void Bind() const;
		void Unbind() const;

		void SubDataBind(const void* data, unsigned int size, unsigned int offset) const;

		void PrintUBOData() const;
		const float* const GetUBOData() const;

		inline unsigned int GetSize() { return m_Size; }
		inline unsigned int GetBindingIndex() { return m_BindingIndex; }
	};
}
}
}