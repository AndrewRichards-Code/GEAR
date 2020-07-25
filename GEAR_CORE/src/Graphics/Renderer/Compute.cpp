#include "compute.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace OPENGL;

Compute::Compute(const ComputeShader& computeShder)
	:m_ComputeShader(computeShder)
{
	//Num of Work Groups
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &m_MaxComputeWorkGroupCount[0]); //X
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &m_MaxComputeWorkGroupCount[1]); //Y
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &m_MaxComputeWorkGroupCount[2]); //Z
	//Size of Work Groups
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &m_MaxComputeWorkGroupSize[0]); //X
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &m_MaxComputeWorkGroupSize[1]); //Y
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &m_MaxComputeWorkGroupSize[2]); //Z
	//Max num of Work Groups 'calls' > WORK_GROUP_SIZE_OF(X) * WORK_GROUP_SIZE_OF(Y) * WORK_GROUP_SIZE_OF(Z)
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &m_MaxComputeWorkGroupInvocations);
}

Compute::~Compute()
{
}

void Compute::AddImage(Texture::TextureType type, Texture::ImageFormat format, int multisample, int width, int height, int depth, int bindingIndex)
{
	m_Images.emplace_back(type, format, multisample, width, height, depth, bindingIndex);
}

void Compute::AddUBO(int size, int bindingIndex) const
{
	BufferManager::AddUBO(size, bindingIndex);
}

void Compute::AddSSBO(int size, int bindingIndex) const
{
	BufferManager::AddSSBO(size, bindingIndex);
}

/*void Compute::AddTexture(Texture::TextureType type, int multisample, int width, int height, int depth)
{
	m_Textures.emplace_back(std::make_unique<Texture>(type, multisample, width, height, depth));
}

void Compute::AddImage()
{
	m_Images.emplace_back(std::make_unique<Image>());
}*/

void Compute::Dispatch(int x, int y, int z)
{
	int ComputeWorkGroupSize[3] = { x, y, z };
	for (int i = 0; i < 3; i++)
	{
		if (m_MaxComputeWorkGroupSize[i] < ComputeWorkGroupSize[i])
		{
			const char* dimError = i == 0 ? "X" : i == 1 ? "Y" : i == 2 ? "Z" : "UNKNOWN";
			std::cout << "ERROR: GEAR::GRAPHICS::OPENGL::ComputeShader: Requested ComputeWorkGroupSize exceeds the MaxComputeWorkGroupSize in the " << dimError << " dimension." << std::endl;
			return;
		}
	}
	if (ComputeWorkGroupSize[0] * ComputeWorkGroupSize[1] * ComputeWorkGroupSize[2] > m_MaxComputeWorkGroupInvocations)
	{
		std::cout << "ERROR: GEAR::GRAPHICS::OPENGL::ComputeShader: Requested ComputeWorkGroupSize.xyz exceeds the MaxComputeWorkGroupInvocations." << std::endl;
		return;
	}

	m_ComputeShader.Enable();
	glDispatchCompute(x, y, z);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT|GL_IMAGE_BIT);
	m_ComputeShader.Disable();
}
