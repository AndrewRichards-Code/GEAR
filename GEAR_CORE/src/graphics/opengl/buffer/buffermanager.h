#pragma once

#include "shaderstoragebuffer.h"
#include "uniformbuffer.h"
#include <vector>
#include <memory>

namespace GEAR {
namespace GRAPHICS {
namespace OPENGL {
	class BufferManager
	{
	private:
		static std::vector<std::unique_ptr<ShaderStorageBuffer>> m_SSBOs;
		static std::vector<std::unique_ptr<UniformBuffer>> m_UBOs;

	public:
		static void AddSSBO(unsigned int size, unsigned int bindingIndex);
		static void AccessSSBO(unsigned int bindingIndex, void* data, unsigned int size, unsigned int offset, ShaderStorageBuffer::ShaderStorageAccess access);
		static void AddUBO(unsigned int size, unsigned int bindingIndex);
		static void UpdateUBO(unsigned int bindingIndex, const void* data, unsigned int size, unsigned int offset);
		

	private:
		static inline ShaderStorageBuffer& GetSSBO(int bindingIndex)
		{
			unsigned int index = -1;
			for (unsigned int i = 0; i < m_SSBOs.size(); i++)
			{
				if (m_SSBOs[i]->GetBindingIndex() == bindingIndex)
				{
					index = i;
					break;
				}
			}
			if (index == -1)
			{
				std::cout << "ERROR: GEAR::GRAPHICS::OPENGL::BufferManager: Failed to find SSBO at Binding Index " << bindingIndex << "." << std::endl;
				throw;
			}
			return *m_SSBOs[index];
		}
		static inline UniformBuffer& GetUBO(int bindingIndex)
		{
			unsigned int index = -1;
			for (unsigned int i = 0; i < m_UBOs.size(); i++)
			{
				if (m_UBOs[i]->GetBindingIndex() == bindingIndex)
				{
					index = i;
					break;
				}
			}
			if (index == -1)
			{
				std::cout << "ERROR: GEAR::GRAPHICS::OPENGL::BufferManager: Failed to find UBO at Binding Index " << bindingIndex << "." << std::endl;
				throw;
			}
			return *m_UBOs[index];
		}
	};
}
}
}