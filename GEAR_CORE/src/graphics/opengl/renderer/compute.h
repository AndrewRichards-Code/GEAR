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
		enum class ImageAccess : unsigned int
		{
			GEAR_WRITE_ONLY = GL_WRITE_ONLY,
			GEAR_READ_ONLY = GL_READ_ONLY,
			GEAR_READ_WRITE = GL_READ_WRITE
		};

	private:
		Texture m_Texture;
		Texture::TextureType m_Type; 
		int m_BindingIndex;
		int m_Multisample; 
		int m_Width, m_Height, m_Depth;
		ImageAccess m_Access;

	public:
		Image(Texture::TextureType type, Texture::ImageFormat format, int multisample, int width, int height, int depth, int bindingIndex)
			:m_Type(type), m_BindingIndex(bindingIndex), m_Multisample(multisample), m_Width(width), m_Height(height), m_Depth(depth)
		{
			m_Texture = Texture(type, format, multisample, width, height, depth);
		};
		~Image() 
		{
			m_Texture.~Texture();
		};

		void Bind(ImageAccess access)
		{
			m_Access = access;
			m_Texture.Bind();
;			glBindImageTexture(m_BindingIndex, m_Texture.GetTextureID(), 0, GL_FALSE, 0, (unsigned int)access, (unsigned int)m_Type);
			
		};
		void Unbind()const 
		{
;			glBindImageTexture(m_BindingIndex, 0, 0, GL_FALSE, 0, (unsigned int)m_Access, (unsigned int)m_Type);
			m_Texture.Unbind();
		};

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

		std::vector<Image> m_Images;

		void AddImage(Texture::TextureType type, Texture::ImageFormat format, int multisample, int width, int height, int depth, int bindingIndex);
		void AddUBO(int size, int bindingIndex) const;
		void AddSSBO(int size, int bindingIndex) const;
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