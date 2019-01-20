#pragma once

#include "GL/glew.h"
#include "../shader/computeshader.h"
#include "../buffer/buffermanager.h"
#include "../texture.h"
#include <vector>
#include <memory>

namespace GEAR {
namespace GRAPHICS {
namespace OPENGL {
	class Image
	{
	public:
		void Bind()const {};
		void Unbind()const {};
	};
	
	class Compute
	{
	private:
		int m_MaxComputeWorkGroupSize[3];
		int m_MaxComputeWorkGroupCount[3];
		int m_MaxComputeWorkGroupInvocations;

		ComputeShader m_ComputeShader;

		//std::vector<std::unique_ptr<Texture>>m_Textures;
		//std::vector<std::unique_ptr<Image>>m_Images;

	public:
		Compute(const ComputeShader& computeShder);
		~Compute();

		void AddUBO(int size, int bindingIndex) const;
		void AddSSBO(int size, int bindingIndex) const;
		//void AddTexture(Texture::TextureType type, int multisample = 1, int width = 0, int height = 0, int depth = 0);
		//void AddImage();

		void Dispatch(int x, int y, int z);

		inline void AccessSSBO(unsigned int bindingIndex, float* data, unsigned int size, unsigned int offset, ShaderStorageBuffer::ShaderStorageAccess access) const
		{
			BufferManager::AccessSSBO(bindingIndex, data, size, offset, access);
		}

		inline void UpdateUBO(unsigned int bindingIndex, const float* data, unsigned int size, unsigned int offset) const
		{
			BufferManager::UpdateUBO(bindingIndex, data, size, offset);
		}
	};
}
}
}