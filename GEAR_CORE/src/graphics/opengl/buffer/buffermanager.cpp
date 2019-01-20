#include "buffermanager.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace OPENGL;

std::vector<std::unique_ptr<ShaderStorageBuffer>> BufferManager::m_SSBOs = {};
std::vector<std::unique_ptr<UniformBuffer>> BufferManager::m_UBOs = {};

//SSBO
void BufferManager::AddSSBO(unsigned int size, unsigned int bindingIndex)
{
	for (auto& ssbo : m_SSBOs)
	{
		if (ssbo->GetBindingIndex() == bindingIndex)
		{
			std::cout << "ERROR: GEAR::GRAPHICS::OPENGL::BufferManager: A SSBO at Binding Index " << bindingIndex << " has already been added." << std::endl;
			throw;
		}
	}
	m_SSBOs.emplace_back(std::make_unique<ShaderStorageBuffer>(size, bindingIndex));
}

void BufferManager::AccessSSBO(unsigned int bindingIndex, float* data, unsigned int size, unsigned int offset, ShaderStorageBuffer::ShaderStorageAccess access)
{
	GetSSBO(bindingIndex).Access(data, size, offset, access);
}

//UBO
void BufferManager::AddUBO(unsigned int size, unsigned int bindingIndex)
{
	for (auto& ubo : m_UBOs)
	{
		if (ubo->GetBindingIndex() == bindingIndex)
		{
			std::cout << "ERROR: GEAR::GRAPHICS::OPENGL::BufferManager: A SSBO at Binding Index " << bindingIndex << " has already been added." << std::endl;
			throw;
		}
	}
	m_UBOs.emplace_back(std::make_unique<UniformBuffer>(size, bindingIndex));
}

void BufferManager::UpdateUBO(unsigned int bindingIndex, const float* data, unsigned int size, unsigned int offset)
{
	GetUBO(bindingIndex).SubDataBind(data, size, offset);
}