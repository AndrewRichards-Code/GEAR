#include "gear_core_common.h"
#include "Renderer.h"
#include "ARC/src/StringConversion.h"
#include "ImageProcessing.h"
#include "FrameGraph.h"
#include "Window.h"
#include "PostProcessing.h"

using namespace gear;
using namespace graphics;
using namespace objects;

using namespace miru;
using namespace miru::crossplatform;

std::map<std::string, Ref<RenderPipeline>> Renderer::s_RenderPipelines;

Renderer::Renderer(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	m_Context = m_CI.window->GetContext();
	m_Device = m_Context->GetDevice();
	m_SwapchainImageCount = m_CI.window->GetSwapchain()->GetCreateInfo().swapchainCount;

	//CmdPools and CmdBuffers
	for (uint32_t i = 0; i < (uint32_t(CommandPool::QueueType::TRANSFER) + uint32_t(1)); i++)
	{
		CommandPool::QueueType type = CommandPool::QueueType(i);
		uint32_t cmdBufferCount;
		std::string name;
		switch (type)
		{
		case CommandPool::QueueType::GRAPHICS:
			name = "Graphics";
			cmdBufferCount = m_SwapchainImageCount + 2; // Extra for upload assistant for the transfer queue.
			break;
		case CommandPool::QueueType::COMPUTE:
			name = "Compute";
			cmdBufferCount = 1;
			break;
		case CommandPool::QueueType::TRANSFER:
			name = "Transfer";
			cmdBufferCount = 1;
			break;
		default:
			break;
		}

		auto& cmd = m_CommandPoolAndBuffers[type];
		cmd.cmdPoolCI.debugName = "GEAR_CORE_CommandPool_Renderer_" + name;
		cmd.cmdPoolCI.pContext = m_Context;
		cmd.cmdPoolCI.flags = CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
		cmd.cmdPoolCI.queueType = type;
		cmd.cmdPool = CommandPool::Create(&cmd.cmdPoolCI);

		cmd.cmdBufferCI.debugName = "GEAR_CORE_CommandBuffer_Renderer_" + name;
		cmd.cmdBufferCI.pCommandPool = cmd.cmdPool;
		cmd.cmdBufferCI.level = CommandBuffer::Level::PRIMARY;
		cmd.cmdBufferCI.commandBufferCount = cmdBufferCount;
		cmd.cmdBufferCI.allocateNewCommandPoolPerBuffer = GraphicsAPI::IsD3D12();
		cmd.cmdBuffer = CommandBuffer::Create(&cmd.cmdBufferCI);
	}

	//Descriptor Pool and Sets
	m_DescPoolAndSets.resize(m_SwapchainImageCount);
	
	//Present Synchronisation
	for (uint32_t i = 0; i < m_SwapchainImageCount; i++)
	{
		m_DrawFenceCI.debugName = "GEAR_CORE_Fence_Renderer_Draw_" + std::to_string(i);
		m_DrawFenceCI.device = m_Device;
		m_DrawFenceCI.signaled = true;
		m_DrawFenceCI.timeout = UINT64_MAX;
		m_DrawFences.emplace_back(Fence::Create(&m_DrawFenceCI));

		m_AcquireSemaphoreCI.debugName = "GEAR_CORE_Seamphore_Renderer_Acquire_" + std::to_string(i);
		m_AcquireSemaphoreCI.device = m_Device;
		m_AcquireSemaphores.emplace_back(Semaphore::Create(&m_AcquireSemaphoreCI));

		m_SubmitSemaphoreCI.debugName = "GEAR_CORE_Seamphore_Renderer_Submit_" + std::to_string(i);
		m_SubmitSemaphoreCI.device = m_Device;
		m_SubmitSemaphores.emplace_back(Semaphore::Create(&m_SubmitSemaphoreCI));
	}
	SubmitRenderSurface(m_CI.window->GetRenderSurface());
	InitialiseRenderPipelines(m_RenderSurface);
}

Renderer::~Renderer()
{
	m_Context->DeviceWaitIdle();
}

void Renderer::InitialiseRenderPipelines(const Ref<RenderSurface>& renderSurface)
{
	if (!s_RenderPipelines.empty())
		return;
	
	std::vector<std::tuple<std::string, uint32_t, uint32_t>> filepaths =
	{
		{ "res/pipelines/PBROpaque.grpf.json",				0, 0 },
		{ "res/pipelines/HDR.grpf.json",					1, 0 },
		{ "res/pipelines/Cube.grpf.json",					0, 0 },
		{ "res/pipelines/Text.grpf.json",					1, 0 },
		{ "res/pipelines/DebugCoordinateAxes.grpf.json",	1, 0 },
		{ "res/pipelines/DebugCopy.grpf.json",				2, 0 }
	};

	RenderPipeline::LoadInfo renderPipelineLI;
	for (auto& filepath : filepaths)
	{
		renderPipelineLI.device = m_Device;
		renderPipelineLI.filepath = filepath._Myfirst._Val;
		renderPipelineLI.viewportWidth = static_cast<float>(renderSurface->GetWidth());
		renderPipelineLI.viewportHeight = static_cast<float>(renderSurface->GetHeight());
		renderPipelineLI.samples = renderSurface->GetCreateInfo().samples;
		renderPipelineLI.renderPass = 
			filepath._Get_rest()._Myfirst._Val == 2 ? 
			m_CI.window->GetSwapchainRenderPass() :
			filepath._Get_rest()._Myfirst._Val == 0 ? 
			renderSurface->GetMainRenderPass() : renderSurface->GetHDRRenderPass();
		renderPipelineLI.subpassIndex = filepath._Get_rest()._Get_rest()._Myfirst._Val;
		Ref<RenderPipeline> renderPipeline = CreateRef<RenderPipeline>(&renderPipelineLI);
		s_RenderPipelines[renderPipeline->m_CI.debugName] = renderPipeline;
	}
}

void Renderer::SubmitRenderSurface(const Ref<RenderSurface>& renderSurface)
{ 
	if (m_RenderSurface != renderSurface)
	{
		m_RenderSurface = renderSurface;
	}
}

void Renderer::SubmitCamera(const Ref<objects::Camera>& camera)
{ 
	if (m_Camera != camera)
	{
		m_Camera = camera;
	}
}

void Renderer::SubmitTextCamera(const Ref<Camera>& textCamera)
{
	m_TextCamera = textCamera;
}

void Renderer::SubmitLights(const std::vector<Ref<Light>>& lights)
{ 
	m_Lights = lights; 
}

void Renderer::SubmitSkybox(const Ref<Skybox>& skybox)
{ 
	if (m_Skybox != skybox)
	{
		m_Skybox = skybox; 
	}
	if (m_Skybox)
	{
		SubmitModel(m_Skybox->GetModel());
	}
}

void Renderer::SubmitModel(const Ref<Model>& obj)
{
	m_ModelQueue.push_back(obj);
}

void Renderer::SubmitTextLine(const Ref<Model>& obj)
{
	m_TextQueue.push_back(obj);
}

void Renderer::Upload()
{
	//Get Texture Barries and/or Reload Textures
	std::set<Ref<Texture>> texturesToProcess;
	std::vector<Ref<Texture>> texturesToGenerateMipmaps;

	std::vector<Ref<Barrier>> textureUnknownToTransferDstBarrier;
	std::vector<Ref<Barrier>> textureTransferDstToShaderReadOnlyBarrier;
	std::vector<Ref<Barrier>> textureShaderReadOnlyToTransferDst;
	std::vector<Ref<Barrier>> textureGeneralToShaderReadOnlyBarrier;
	std::vector<Ref<Barrier>> textureShaderReadOnlyToGeneralBarrier;

	std::vector<Ref<objects::Model>> allQueue;
	allQueue.insert(allQueue.end(), m_ModelQueue.begin(), m_ModelQueue.end());
	allQueue.insert(allQueue.end(), m_TextQueue.begin(), m_TextQueue.end());

	//Get all unique textures
	for (auto& model : allQueue)
	{
		for (auto& material : model->GetMesh()->GetMaterials())
		{
			for (auto& texture : material->GetTextures())
			{
				texturesToProcess.insert(texture.second);
			}
		}
	}

	//Deal with Skybox Textures first
	if (m_Skybox)
	{
		if (!m_Skybox->m_Cubemap && !m_Skybox->m_Generated)
		{
			auto SkyboxTextureStateUpdate = [&](Ref<Texture> texture)
			{
				if (texture->m_ShaderReadable)
				{
					texture->TransitionSubResources(textureShaderReadOnlyToGeneralBarrier,
						{ { Barrier::AccessBit::SHADER_READ_BIT, 
						Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT,
						Image::Layout::SHADER_READ_ONLY_OPTIMAL, 
						Image::Layout::GENERAL, 
						{}, true } });
					texture->m_ShaderReadable = false;
				}
				texture->TransitionSubResources(textureGeneralToShaderReadOnlyBarrier,
					{ { Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT, 
					Barrier::AccessBit::SHADER_READ_BIT ,
					Image::Layout::GENERAL,
					Image::Layout::SHADER_READ_ONLY_OPTIMAL, 
					{}, true } });
				texture->m_ShaderReadable = true;
			};
			SkyboxTextureStateUpdate(m_Skybox->GetGeneratedSpecularBRDF_LUT());
			SkyboxTextureStateUpdate(m_Skybox->GetGeneratedSpecularCubemap());
			SkyboxTextureStateUpdate(m_Skybox->GetGeneratedDiffuseCubemap());
			SkyboxTextureStateUpdate(m_Skybox->GetGeneratedCubemap());
			SkyboxTextureStateUpdate(m_Skybox->GetTexture());
				
			if (m_Skybox->GetTexture()->m_PreUpload)
			{
				m_Skybox->GetTexture()->TransitionSubResources(textureUnknownToTransferDstBarrier,
					{ { Barrier::AccessBit::NONE_BIT, Barrier::AccessBit::TRANSFER_WRITE_BIT,
					Image::Layout::UNKNOWN, Image::Layout::TRANSFER_DST_OPTIMAL, {}, true } });
				m_Skybox->GetTexture()->m_PreUpload = false;
			}
			
		}
		texturesToProcess.erase(m_Skybox->GetGeneratedSpecularBRDF_LUT());
		texturesToProcess.erase(m_Skybox->GetGeneratedSpecularCubemap());
		texturesToProcess.erase(m_Skybox->GetGeneratedDiffuseCubemap());
		texturesToProcess.erase(m_Skybox->GetGeneratedCubemap());
		texturesToProcess.erase(m_Skybox->GetTexture());
	}

	//Process them
	for (auto& texture : texturesToProcess)
	{
		if (m_ReloadTextures)
		{
			texture->Reload();
			texture->m_ShaderReadable = false;
			texture->TransitionSubResources(textureShaderReadOnlyToTransferDst,
				{ { Barrier::AccessBit::SHADER_READ_BIT, Barrier::AccessBit::TRANSFER_WRITE_BIT,
				Image::Layout::SHADER_READ_ONLY_OPTIMAL, Image::Layout::TRANSFER_DST_OPTIMAL, {}, true } });
		}

		if (texture->m_PreUpload)
		{
			texture->TransitionSubResources(textureUnknownToTransferDstBarrier,
				{ { Barrier::AccessBit::NONE_BIT, Barrier::AccessBit::TRANSFER_WRITE_BIT,
				Image::Layout::UNKNOWN, Image::Layout::TRANSFER_DST_OPTIMAL, {}, true } });
			texture->m_PreUpload = false;
		}

		if (!texture->m_ShaderReadable)
		{
			if (texture->m_GenerateMipMaps && !texture->m_Generated)
			{
				texturesToGenerateMipmaps.push_back(texture);
				texture->TransitionSubResources(textureGeneralToShaderReadOnlyBarrier,
					{ { Barrier::AccessBit::SHADER_WRITE_BIT, Barrier::AccessBit::SHADER_READ_BIT,
					Image::Layout::GENERAL, Image::Layout::SHADER_READ_ONLY_OPTIMAL, {}, true } });
			}
			else
			{
				texture->TransitionSubResources(textureTransferDstToShaderReadOnlyBarrier,
					{ { Barrier::AccessBit::TRANSFER_WRITE_BIT, Barrier::AccessBit::SHADER_READ_BIT,
					Image::Layout::TRANSFER_DST_OPTIMAL, Image::Layout::SHADER_READ_ONLY_OPTIMAL, {}, true } });
			}
			texture->m_ShaderReadable = true;
		}
	}
	m_ReloadTextures = false;

	bool preTransferGraphicsTask1 = textureShaderReadOnlyToTransferDst.size();
	bool preTransferGraphicsTask2 = textureShaderReadOnlyToGeneralBarrier.size();
	bool preUploadTransferTask = textureUnknownToTransferDstBarrier.size();

	bool transferTask = true;
	bool asyncComputeTask = texturesToGenerateMipmaps.size() || (m_Skybox && !m_Skybox->m_Generated);

	bool postComputeGraphicsTask = textureGeneralToShaderReadOnlyBarrier.size();
	bool postTransferGraphicsTask = textureTransferDstToShaderReadOnlyBarrier.size();

	Ref<GPUTask> preTransferGraphicsGPUTask1, preTransferGraphicsGPUTask2, preUploadForTextrueTransferGPUTask, uploadTransferGPUTask, postComputeGraphicsGPUTask, postTransferGraphicsGPUTask;
	Ref<GPUTask> beginAsyncComputeGPUTask, endAsyncComputeGPUTask;
	std::vector<Ref<GPUTask>> generateMipmapsComputeGPUTasks;
	std::vector<Ref<GPUTask>> generateSkyboxComputeGPUTasks;

	GPUTask::TransitionResourcesTaskInfo trti;

	//Pre-Transfer Graphics Task
	{
		trti.srcPipelineStage = PipelineStageBit::FRAGMENT_SHADER_BIT;
		trti.dstPipelineStage = PipelineStageBit::TRANSFER_BIT;
		trti.barriers = textureShaderReadOnlyToTransferDst;
		GPUTask::CreateInfo preTransferGraphicsGPUTaskCI;
		preTransferGraphicsGPUTaskCI.debugName = "Pre-Transfer - Graphics 1";
		preTransferGraphicsGPUTaskCI.task = GPUTask::Task::TRANSITION_RESOURCES;
		preTransferGraphicsGPUTaskCI.pTaskInfo = &trti;
		preTransferGraphicsGPUTaskCI.srcGPUTasks = {};
		preTransferGraphicsGPUTaskCI.srcPipelineStages = {};
		preTransferGraphicsGPUTaskCI.cmdBuffer = m_CommandPoolAndBuffers[CommandPool::QueueType::GRAPHICS].cmdBuffer;
		preTransferGraphicsGPUTaskCI.cmdBufferIndex = 3;
		preTransferGraphicsGPUTaskCI.cmdBufferControls = GPUTask::CommandBufferBasicControlsBit::RESET_BEGIN;
		preTransferGraphicsGPUTaskCI.resetCmdBufferReleaseResource = false;
		preTransferGraphicsGPUTaskCI.beginCmdBufferUsage = CommandBuffer::UsageBit::ONE_TIME_SUBMIT;
		preTransferGraphicsGPUTaskCI.skipTask = !preTransferGraphicsTask1;
		preTransferGraphicsGPUTask1 = CreateRef<GPUTask>(&preTransferGraphicsGPUTaskCI);
		preTransferGraphicsGPUTask1->Execute();

		trti.srcPipelineStage = PipelineStageBit::FRAGMENT_SHADER_BIT;
		trti.dstPipelineStage = PipelineStageBit::COMPUTE_SHADER_BIT;
		trti.barriers = textureShaderReadOnlyToGeneralBarrier;
		preTransferGraphicsGPUTaskCI.debugName = "Pre-Transfer - Graphics 2 ";
		preTransferGraphicsGPUTaskCI.pTaskInfo = &trti;
		preTransferGraphicsGPUTaskCI.srcGPUTasks = { preTransferGraphicsGPUTask1 };
		preTransferGraphicsGPUTaskCI.srcPipelineStages = { PipelineStageBit::TRANSFER_BIT };
		preTransferGraphicsGPUTaskCI.cmdBufferControls = GPUTask::CommandBufferBasicControlsBit::END_SUBMIT;
		preTransferGraphicsGPUTaskCI.skipTask = !preTransferGraphicsTask2;
		preTransferGraphicsGPUTask2 = CreateRef<GPUTask>(&preTransferGraphicsGPUTaskCI);
		preTransferGraphicsGPUTask2->Execute();
	}

	//Pre-Upload Transfer Task
	{
		trti.srcPipelineStage = PipelineStageBit::TOP_OF_PIPE_BIT;
		trti.dstPipelineStage = PipelineStageBit::TRANSFER_BIT;
		trti.barriers = textureUnknownToTransferDstBarrier;
		GPUTask::CreateInfo preUploadForTextrueTransferGPUTaskCI;
		preUploadForTextrueTransferGPUTaskCI.debugName = "Pre-Upload - Transfer";
		preUploadForTextrueTransferGPUTaskCI.task = GPUTask::Task::TRANSITION_RESOURCES;
		preUploadForTextrueTransferGPUTaskCI.pTaskInfo = &trti;
		preUploadForTextrueTransferGPUTaskCI.srcGPUTasks = { preTransferGraphicsGPUTask2 };
		preUploadForTextrueTransferGPUTaskCI.srcPipelineStages = { PipelineStageBit::TRANSFER_BIT };
		preUploadForTextrueTransferGPUTaskCI.cmdBuffer = m_CommandPoolAndBuffers[CommandPool::QueueType::TRANSFER].cmdBuffer;
		preUploadForTextrueTransferGPUTaskCI.cmdBufferIndex = 0;
		preUploadForTextrueTransferGPUTaskCI.cmdBufferControls = GPUTask::CommandBufferBasicControlsBit::RESET_BEGIN;
		preUploadForTextrueTransferGPUTaskCI.resetCmdBufferReleaseResource = false;
		preUploadForTextrueTransferGPUTaskCI.beginCmdBufferUsage = CommandBuffer::UsageBit::ONE_TIME_SUBMIT;
		preUploadForTextrueTransferGPUTaskCI.skipTask = !preUploadTransferTask;
		preUploadForTextrueTransferGPUTask = CreateRef<GPUTask>(&preUploadForTextrueTransferGPUTaskCI);
		preUploadForTextrueTransferGPUTask->Execute();
	}

	//Upload Transfer Task
	{
		GPUTask::UploadResourceTaskInfo urti;
		urti.camera = m_Camera;
		urti.cameraForce = true;
		urti.textCamera = m_TextCamera;
		urti.textCameraForce = true;
		urti.skybox = m_Skybox;
		urti.skyboxForce = true;
		urti.lights = m_Lights;
		urti.lightsForce = false;
		urti.models = allQueue;
		urti.modelsForce = false;
		urti.materialsForce = false;
		GPUTask::CreateInfo uploadTransferGPUTaskCI;
		uploadTransferGPUTaskCI.debugName = "Upload - Transfer";
		uploadTransferGPUTaskCI.task = GPUTask::Task::UPLOAD_RESOURCES;
		uploadTransferGPUTaskCI.pTaskInfo = &urti;
		uploadTransferGPUTaskCI.srcGPUTasks = { preUploadForTextrueTransferGPUTask };
		uploadTransferGPUTaskCI.srcPipelineStages = { (PipelineStageBit)0 };
		uploadTransferGPUTaskCI.cmdBuffer = m_CommandPoolAndBuffers[CommandPool::QueueType::TRANSFER].cmdBuffer;
		uploadTransferGPUTaskCI.cmdBufferIndex = 0;
		uploadTransferGPUTaskCI.cmdBufferControls = GPUTask::CommandBufferBasicControlsBit::END_SUBMIT;
		uploadTransferGPUTaskCI.resetCmdBufferReleaseResource = false;
		uploadTransferGPUTaskCI.beginCmdBufferUsage = CommandBuffer::UsageBit::ONE_TIME_SUBMIT;
		uploadTransferGPUTaskCI.skipTask = !transferTask;
		uploadTransferGPUTask = CreateRef<GPUTask>(&uploadTransferGPUTaskCI);
		uploadTransferGPUTask->Execute();
	}

	//Async Compute Task
	{
		GPUTask::CreateInfo beginAsyncComputeGPUTaskCI;
		beginAsyncComputeGPUTaskCI.debugName = "Async Compute Begin";
		beginAsyncComputeGPUTaskCI.task = GPUTask::Task::NONE;
		beginAsyncComputeGPUTaskCI.pTaskInfo = nullptr;
		beginAsyncComputeGPUTaskCI.srcGPUTasks = { uploadTransferGPUTask };
		beginAsyncComputeGPUTaskCI.srcPipelineStages = { PipelineStageBit::TRANSFER_BIT };
		beginAsyncComputeGPUTaskCI.cmdBuffer = m_CommandPoolAndBuffers[CommandPool::QueueType::COMPUTE].cmdBuffer;
		beginAsyncComputeGPUTaskCI.cmdBufferIndex = 0;
		beginAsyncComputeGPUTaskCI.cmdBufferControls = GPUTask::CommandBufferBasicControlsBit::RESET_BEGIN;
		beginAsyncComputeGPUTaskCI.resetCmdBufferReleaseResource = false;
		beginAsyncComputeGPUTaskCI.beginCmdBufferUsage = CommandBuffer::UsageBit::ONE_TIME_SUBMIT;
		beginAsyncComputeGPUTaskCI.skipTask = !asyncComputeTask;
		beginAsyncComputeGPUTask = CreateRef<GPUTask>(&beginAsyncComputeGPUTaskCI);
		beginAsyncComputeGPUTask->Execute();

		for (auto& texture : texturesToGenerateMipmaps)
		{
			GPUTask::ImageProcessingFunctionTaskInfo1 ipfti1;
			ipfti1.pfn = ImageProcessing::GenerateMipMaps;
			ipfti1.tri1 = { texture, Barrier::AccessBit::TRANSFER_WRITE_BIT, Image::Layout::TRANSFER_DST_OPTIMAL, PipelineStageBit::TRANSFER_BIT };
			GPUTask::CreateInfo generateMipmapsGPUTaskCI;
			generateMipmapsGPUTaskCI.debugName = "Generate Mips : " + texture->GetCreateInfo().debugName;
			generateMipmapsGPUTaskCI.task = GPUTask::Task::IMAGE_PROCESSING_FUNCTION_1;
			generateMipmapsGPUTaskCI.pTaskInfo = &ipfti1;
			if (generateMipmapsComputeGPUTasks.empty())
				generateMipmapsGPUTaskCI.srcGPUTasks = { beginAsyncComputeGPUTask };
			else
				generateMipmapsGPUTaskCI.srcGPUTasks = { generateMipmapsComputeGPUTasks.back() };

			generateMipmapsGPUTaskCI.srcPipelineStages = { (PipelineStageBit)0 };
			generateMipmapsGPUTaskCI.cmdBuffer = m_CommandPoolAndBuffers[CommandPool::QueueType::COMPUTE].cmdBuffer;
			generateMipmapsGPUTaskCI.cmdBufferIndex = 0;
			generateMipmapsGPUTaskCI.cmdBufferControls = GPUTask::CommandBufferBasicControlsBit::NONE;
			generateMipmapsGPUTaskCI.resetCmdBufferReleaseResource = false;
			generateMipmapsGPUTaskCI.beginCmdBufferUsage = CommandBuffer::UsageBit::ONE_TIME_SUBMIT;
			generateMipmapsGPUTaskCI.skipTask = texture->m_Generated;
			generateMipmapsComputeGPUTasks.emplace_back(CreateRef<GPUTask>(&generateMipmapsGPUTaskCI));
			generateMipmapsComputeGPUTasks.back()->Execute();
		}

		GPUTask::ImageProcessingFunctionTaskInfo2 ipfti2;
		ipfti2.pfn = ImageProcessing::EquirectangularToCube;
		ipfti2.tri1 = { m_Skybox->GetGeneratedCubemap(), Barrier::AccessBit::NONE_BIT, Image::Layout::UNKNOWN, PipelineStageBit::TOP_OF_PIPE_BIT };
		ipfti2.tri2 = { m_Skybox->GetTexture(), Barrier::AccessBit::TRANSFER_WRITE_BIT, Image::Layout::TRANSFER_DST_OPTIMAL, PipelineStageBit::TRANSFER_BIT };
		GPUTask::CreateInfo generateSkyboxTaskCI;
		generateSkyboxTaskCI.debugName = "EquirectangularToCube : " + m_Skybox->GetTexture()->GetCreateInfo().debugName;
		generateSkyboxTaskCI.task = GPUTask::Task::IMAGE_PROCESSING_FUNCTION_2;
		generateSkyboxTaskCI.pTaskInfo = &ipfti2;
		if (!generateMipmapsComputeGPUTasks.empty())
			generateSkyboxTaskCI.srcGPUTasks = { generateMipmapsComputeGPUTasks.back() };
		else
			generateSkyboxTaskCI.srcGPUTasks = { beginAsyncComputeGPUTask };
		generateSkyboxTaskCI.srcPipelineStages = { (PipelineStageBit)0 };
		generateSkyboxTaskCI.cmdBuffer = m_CommandPoolAndBuffers[CommandPool::QueueType::COMPUTE].cmdBuffer;
		generateSkyboxTaskCI.cmdBufferIndex = 0;
		generateSkyboxTaskCI.cmdBufferControls = GPUTask::CommandBufferBasicControlsBit::NONE;
		generateSkyboxTaskCI.resetCmdBufferReleaseResource = false;
		generateSkyboxTaskCI.beginCmdBufferUsage = CommandBuffer::UsageBit::ONE_TIME_SUBMIT;
		generateSkyboxTaskCI.skipTask = m_Skybox->m_Generated;
		generateSkyboxComputeGPUTasks.emplace_back(CreateRef<GPUTask>(&generateSkyboxTaskCI));
		generateSkyboxComputeGPUTasks.back()->Execute();

		GPUTask::ImageProcessingFunctionTaskInfo1 ipfti1;
		ipfti1.pfn = ImageProcessing::GenerateMipMaps;
		ipfti1.tri1 = { m_Skybox->GetGeneratedCubemap(), Barrier::AccessBit::SHADER_WRITE_BIT, Image::Layout::GENERAL, PipelineStageBit::COMPUTE_SHADER_BIT };
		generateSkyboxTaskCI.debugName = "GenerateMipMaps : " + m_Skybox->GetTexture()->GetCreateInfo().debugName;
		generateSkyboxTaskCI.task = GPUTask::Task::IMAGE_PROCESSING_FUNCTION_1;
		generateSkyboxTaskCI.pTaskInfo = &ipfti1;
		generateSkyboxTaskCI.srcGPUTasks = { generateSkyboxComputeGPUTasks.back() };
		generateSkyboxTaskCI.srcPipelineStages = { PipelineStageBit::COMPUTE_SHADER_BIT };
		generateSkyboxTaskCI.cmdBuffer = m_CommandPoolAndBuffers[CommandPool::QueueType::COMPUTE].cmdBuffer;
		generateSkyboxTaskCI.cmdBufferIndex = 0;
		generateSkyboxTaskCI.cmdBufferControls = GPUTask::CommandBufferBasicControlsBit::NONE;
		generateSkyboxTaskCI.resetCmdBufferReleaseResource = false;
		generateSkyboxTaskCI.beginCmdBufferUsage = CommandBuffer::UsageBit::ONE_TIME_SUBMIT;
		generateSkyboxTaskCI.skipTask = m_Skybox->m_Generated;
		generateSkyboxComputeGPUTasks.emplace_back(CreateRef<GPUTask>(&generateSkyboxTaskCI));
		generateSkyboxComputeGPUTasks.back()->Execute();

		ipfti2.pfn = ImageProcessing::DiffuseIrradiance;
		ipfti2.tri1 = { m_Skybox->GetGeneratedDiffuseCubemap(), Barrier::AccessBit::NONE_BIT, Image::Layout::UNKNOWN, PipelineStageBit::TOP_OF_PIPE_BIT };
		ipfti2.tri2 = { m_Skybox->GetGeneratedCubemap(), Barrier::AccessBit::SHADER_WRITE_BIT, Image::Layout::GENERAL, PipelineStageBit::COMPUTE_SHADER_BIT };
		generateSkyboxTaskCI.debugName = "DiffuseIrradiance : " + m_Skybox->GetTexture()->GetCreateInfo().debugName;
		generateSkyboxTaskCI.task = GPUTask::Task::IMAGE_PROCESSING_FUNCTION_2;
		generateSkyboxTaskCI.pTaskInfo = &ipfti2;
		generateSkyboxTaskCI.srcGPUTasks = { generateSkyboxComputeGPUTasks.back() };
		generateSkyboxTaskCI.srcPipelineStages = { PipelineStageBit::COMPUTE_SHADER_BIT };
		generateSkyboxTaskCI.cmdBuffer = m_CommandPoolAndBuffers[CommandPool::QueueType::COMPUTE].cmdBuffer;
		generateSkyboxTaskCI.cmdBufferIndex = 0;
		generateSkyboxTaskCI.cmdBufferControls = GPUTask::CommandBufferBasicControlsBit::NONE;
		generateSkyboxTaskCI.resetCmdBufferReleaseResource = false;
		generateSkyboxTaskCI.beginCmdBufferUsage = CommandBuffer::UsageBit::ONE_TIME_SUBMIT;
		generateSkyboxTaskCI.skipTask = m_Skybox->m_Generated;
		generateSkyboxComputeGPUTasks.emplace_back(CreateRef<GPUTask>(&generateSkyboxTaskCI));
		generateSkyboxComputeGPUTasks.back()->Execute();

		ipfti2.pfn = ImageProcessing::SpecularIrradiance;
		ipfti2.tri1 = { m_Skybox->GetGeneratedSpecularCubemap(), Barrier::AccessBit::NONE_BIT, Image::Layout::UNKNOWN, PipelineStageBit::TOP_OF_PIPE_BIT };
		ipfti2.tri2 = { m_Skybox->GetGeneratedCubemap(), Barrier::AccessBit::SHADER_WRITE_BIT, Image::Layout::GENERAL, PipelineStageBit::COMPUTE_SHADER_BIT };
		generateSkyboxTaskCI.debugName = "SpecularIrradiance : " + m_Skybox->GetTexture()->GetCreateInfo().debugName;
		generateSkyboxTaskCI.task = GPUTask::Task::IMAGE_PROCESSING_FUNCTION_2;
		generateSkyboxTaskCI.pTaskInfo = &ipfti2;
		generateSkyboxTaskCI.srcGPUTasks = { generateSkyboxComputeGPUTasks.back() };
		generateSkyboxTaskCI.srcPipelineStages = { PipelineStageBit::COMPUTE_SHADER_BIT };
		generateSkyboxTaskCI.cmdBuffer = m_CommandPoolAndBuffers[CommandPool::QueueType::COMPUTE].cmdBuffer;
		generateSkyboxTaskCI.cmdBufferIndex = 0;
		generateSkyboxTaskCI.cmdBufferControls = GPUTask::CommandBufferBasicControlsBit::NONE;
		generateSkyboxTaskCI.resetCmdBufferReleaseResource = false;
		generateSkyboxTaskCI.beginCmdBufferUsage = CommandBuffer::UsageBit::ONE_TIME_SUBMIT;
		generateSkyboxTaskCI.skipTask = m_Skybox->m_Generated;
		generateSkyboxComputeGPUTasks.emplace_back(CreateRef<GPUTask>(&generateSkyboxTaskCI));
		generateSkyboxComputeGPUTasks.back()->Execute();

		ipfti1.pfn = ImageProcessing::SpecularBRDF_LUT;
		ipfti1.tri1 = { m_Skybox->GetGeneratedSpecularBRDF_LUT(), Barrier::AccessBit::NONE_BIT, Image::Layout::UNKNOWN, PipelineStageBit::TOP_OF_PIPE_BIT };
		generateSkyboxTaskCI.debugName = "SpecularBRDF_LUT : " + m_Skybox->GetTexture()->GetCreateInfo().debugName;
		generateSkyboxTaskCI.task = GPUTask::Task::IMAGE_PROCESSING_FUNCTION_1;
		generateSkyboxTaskCI.pTaskInfo = &ipfti1;
		generateSkyboxTaskCI.srcGPUTasks = { generateSkyboxComputeGPUTasks.back() };
		generateSkyboxTaskCI.srcPipelineStages = { PipelineStageBit::COMPUTE_SHADER_BIT };
		generateSkyboxTaskCI.cmdBuffer = m_CommandPoolAndBuffers[CommandPool::QueueType::COMPUTE].cmdBuffer;
		generateSkyboxTaskCI.cmdBufferIndex = 0;
		generateSkyboxTaskCI.cmdBufferControls = GPUTask::CommandBufferBasicControlsBit::NONE;
		generateSkyboxTaskCI.resetCmdBufferReleaseResource = false;
		generateSkyboxTaskCI.beginCmdBufferUsage = CommandBuffer::UsageBit::ONE_TIME_SUBMIT;
		generateSkyboxTaskCI.skipTask = m_Skybox->m_Generated;
		generateSkyboxComputeGPUTasks.emplace_back(CreateRef<GPUTask>(&generateSkyboxTaskCI));
		generateSkyboxComputeGPUTasks.back()->Execute();
		
		/*if (!m_Skybox->m_Cubemap && !m_Skybox->m_Generated)
		{
			ImageProcessing::EquirectangularToCube(
				{ m_Skybox->GetGeneratedCubemap(), Barrier::AccessBit::NONE_BIT, Image::Layout::UNKNOWN, PipelineStageBit::TOP_OF_PIPE_BIT },
				{ m_Skybox->GetTexture(), Barrier::AccessBit::TRANSFER_WRITE_BIT, Image::Layout::TRANSFER_DST_OPTIMAL, PipelineStageBit::TRANSFER_BIT });
			m_Skybox->m_Generated = true;

			ImageProcessing::GenerateMipMaps({ m_Skybox->GetGeneratedCubemap(), Barrier::AccessBit::SHADER_WRITE_BIT, Image::Layout::GENERAL, PipelineStageBit::COMPUTE_SHADER_BIT });

			ImageProcessing::DiffuseIrradiance(
				{ m_Skybox->GetGeneratedDiffuseCubemap(), Barrier::AccessBit::NONE_BIT, Image::Layout::UNKNOWN, PipelineStageBit::TOP_OF_PIPE_BIT },
				{ m_Skybox->GetGeneratedCubemap(), Barrier::AccessBit::SHADER_WRITE_BIT, Image::Layout::GENERAL, PipelineStageBit::COMPUTE_SHADER_BIT });

			ImageProcessing::SpecularIrradiance(
				{ m_Skybox->GetGeneratedSpecularCubemap(), Barrier::AccessBit::NONE_BIT, Image::Layout::UNKNOWN, PipelineStageBit::TOP_OF_PIPE_BIT },
				{ m_Skybox->GetGeneratedCubemap(), Barrier::AccessBit::SHADER_WRITE_BIT, Image::Layout::GENERAL, PipelineStageBit::COMPUTE_SHADER_BIT });

			ImageProcessing::SpecularBRDF_LUT(
				{ m_Skybox->GetGeneratedSpecularBRDF_LUT(), Barrier::AccessBit::NONE_BIT, Image::Layout::UNKNOWN, PipelineStageBit::TOP_OF_PIPE_BIT });
		}*/
		m_Skybox->m_Generated = true;

		GPUTask::CreateInfo endAsyncComputeGPUTaskCI;
		endAsyncComputeGPUTaskCI.debugName = "Async Compute End";
		endAsyncComputeGPUTaskCI.task = GPUTask::Task::NONE;
		endAsyncComputeGPUTaskCI.pTaskInfo = nullptr;
		endAsyncComputeGPUTaskCI.srcGPUTasks = { generateSkyboxComputeGPUTasks.back() };
		for(auto& i : endAsyncComputeGPUTaskCI.srcGPUTasks)
			endAsyncComputeGPUTaskCI.srcPipelineStages.push_back(PipelineStageBit::COMPUTE_SHADER_BIT);
		endAsyncComputeGPUTaskCI.cmdBuffer = m_CommandPoolAndBuffers[CommandPool::QueueType::COMPUTE].cmdBuffer;
		endAsyncComputeGPUTaskCI.cmdBufferIndex = 0;
		endAsyncComputeGPUTaskCI.cmdBufferControls = GPUTask::CommandBufferBasicControlsBit::END_SUBMIT;
		endAsyncComputeGPUTaskCI.resetCmdBufferReleaseResource = false;
		endAsyncComputeGPUTaskCI.beginCmdBufferUsage = CommandBuffer::UsageBit::ONE_TIME_SUBMIT;
		endAsyncComputeGPUTaskCI.skipTask = !asyncComputeTask;
		endAsyncComputeGPUTask = CreateRef<GPUTask>(&endAsyncComputeGPUTaskCI);
		endAsyncComputeGPUTask->Execute();
	}

	//Post-Compute Graphics Task
	{
		trti.srcPipelineStage = PipelineStageBit::COMPUTE_SHADER_BIT;
		trti.dstPipelineStage = PipelineStageBit::FRAGMENT_SHADER_BIT;
		trti.barriers = textureGeneralToShaderReadOnlyBarrier;
		GPUTask::CreateInfo postComputeGraphicsGPUTaskCI;
		postComputeGraphicsGPUTaskCI.debugName = "Post-Compute - Graphics";
		postComputeGraphicsGPUTaskCI.task = GPUTask::Task::TRANSITION_RESOURCES;
		postComputeGraphicsGPUTaskCI.pTaskInfo = &trti;
		postComputeGraphicsGPUTaskCI.srcGPUTasks = { endAsyncComputeGPUTask };
		postComputeGraphicsGPUTaskCI.srcPipelineStages = { PipelineStageBit::COMPUTE_SHADER_BIT };
		postComputeGraphicsGPUTaskCI.cmdBuffer = m_CommandPoolAndBuffers[CommandPool::QueueType::GRAPHICS].cmdBuffer;
		postComputeGraphicsGPUTaskCI.cmdBufferIndex = 2;
		postComputeGraphicsGPUTaskCI.cmdBufferControls = GPUTask::CommandBufferBasicControlsBit::RESET_BEGIN;
		postComputeGraphicsGPUTaskCI.resetCmdBufferReleaseResource = false;
		postComputeGraphicsGPUTaskCI.beginCmdBufferUsage = CommandBuffer::UsageBit::ONE_TIME_SUBMIT;
		postComputeGraphicsGPUTaskCI.skipTask = !postComputeGraphicsTask;
		postComputeGraphicsGPUTask = CreateRef<GPUTask>(&postComputeGraphicsGPUTaskCI);
		postComputeGraphicsGPUTask->Execute();
	}

	//Post-Transfer Graphics CmdBuffer
	{
		trti.srcPipelineStage = PipelineStageBit::TRANSFER_BIT;
		trti.dstPipelineStage = PipelineStageBit::FRAGMENT_SHADER_BIT;
		trti.barriers = textureTransferDstToShaderReadOnlyBarrier;
		GPUTask::CreateInfo postTransferGraphicsGPUTaskCI;
		postTransferGraphicsGPUTaskCI.debugName = "Post-Transfer - Graphics";
		postTransferGraphicsGPUTaskCI.task = GPUTask::Task::TRANSITION_RESOURCES;
		postTransferGraphicsGPUTaskCI.pTaskInfo = &trti;
		postTransferGraphicsGPUTaskCI.srcGPUTasks = { postComputeGraphicsGPUTask };
		postTransferGraphicsGPUTaskCI.srcPipelineStages = { (PipelineStageBit)0 };
		postTransferGraphicsGPUTaskCI.cmdBuffer = m_CommandPoolAndBuffers[CommandPool::QueueType::GRAPHICS].cmdBuffer;
		postTransferGraphicsGPUTaskCI.cmdBufferIndex = 2;
		postTransferGraphicsGPUTaskCI.cmdBufferControls = GPUTask::CommandBufferBasicControlsBit::END_SUBMIT;
		postTransferGraphicsGPUTaskCI.resetCmdBufferReleaseResource = false;
		postTransferGraphicsGPUTaskCI.beginCmdBufferUsage = CommandBuffer::UsageBit::ONE_TIME_SUBMIT;
		postTransferGraphicsGPUTaskCI.skipTask = !postTransferGraphicsTask;
		postTransferGraphicsGPUTask = CreateRef<GPUTask>(&postTransferGraphicsGPUTaskCI);
		postTransferGraphicsGPUTask->Execute();
	}

	//Wait for all Task Fences
	preTransferGraphicsGPUTask2->GetFence()->Wait();
	uploadTransferGPUTask->GetFence()->Wait();
	endAsyncComputeGPUTask->GetFence()->Wait();
	postTransferGraphicsGPUTask->GetFence()->Wait();

	ImageProcessing::ClearImageViewsAndDescriptorSets();
}

void Renderer::BuildDescriptorSetandPools()
{
	DescriptorPoolAndSets& descPoolAndSets = m_DescPoolAndSets[m_FrameIndex];
	std::string indexString = std::to_string(m_FrameIndex);

	//Desriptor Pool
	std::map<DescriptorType, uint32_t> poolSizesMap;
	size_t m_RenderQueueMaterialCount = 0;
	auto AddToPoolSizeMap = [&](const std::vector<std::vector<Shader::ResourceBindingDescription>>& rbds, uint32_t materialCount = 1) -> void
	{
		uint32_t set = 0;
		uint32_t binding = 0;
		for (auto& set_rbds : rbds)
		{
			for (auto& binding_rbds : set_rbds)
			{
				uint32_t& descCount = poolSizesMap[binding_rbds.type];
				descCount += (binding_rbds.descriptorCount * (set == 2 ? static_cast<uint32_t>(materialCount) : 1));
				binding++;
			}
			set++;
		}
	};

	std::vector<Ref<objects::Model>> allQueue;
	allQueue.insert(allQueue.end(), m_ModelQueue.begin(), m_ModelQueue.end());
	allQueue.insert(allQueue.end(), m_TextQueue.begin(), m_TextQueue.end());
	for (auto& model : allQueue)
	{
		const Ref<Pipeline>& pipeline = s_RenderPipelines[model->GetPipelineName()]->GetPipeline();
		const std::vector<std::vector<Shader::ResourceBindingDescription>>& rbds = s_RenderPipelines[model->GetPipelineName()]->GetRBDs();
		size_t materialCount = model->GetMesh()->GetMaterials().size();
		m_RenderQueueMaterialCount += materialCount;

		AddToPoolSizeMap(rbds, static_cast<uint32_t>(materialCount));
	}
	AddToPoolSizeMap(s_RenderPipelines["HDR"]->GetRBDs());
	AddToPoolSizeMap(s_RenderPipelines["DebugCopy"]->GetRBDs());

	descPoolAndSets.poolCI.debugName = "GEAR_CORE_DescriptorPool_Renderer_" + indexString;
	descPoolAndSets.poolCI.device = m_Device;
	for (auto& poolSize : poolSizesMap)
		descPoolAndSets.poolCI.poolSizes.push_back({ poolSize.first, poolSize.second });
	descPoolAndSets.poolCI.maxSets = static_cast<uint32_t>(s_RenderPipelines.size() + allQueue.size() + m_RenderQueueMaterialCount);
	descPoolAndSets.pool = DescriptorPool::Create(&descPoolAndSets.poolCI);

	//Per Render Pipeline Descriptor Set
	descPoolAndSets.setPerRenderPipeline.clear();
	for (auto& pipeline : s_RenderPipelines)
	{
		const std::vector<Ref<DescriptorSetLayout>>& descriptorSetLayouts = pipeline.second->GetDescriptorSetLayouts();
		const std::vector<std::vector<Shader::ResourceBindingDescription>> rbds = pipeline.second->GetRBDs();

		if (descriptorSetLayouts.empty() || rbds.empty())
			continue;

		DescriptorSet::CreateInfo descSetPerViewCI;
		descSetPerViewCI.debugName = "GEAR_CORE_DescriptorSet_PerView_" + indexString + ": " + pipeline.second->GetPipeline()->GetCreateInfo().debugName;
		descSetPerViewCI.pDescriptorPool = descPoolAndSets.pool;
		descSetPerViewCI.pDescriptorSetLayouts = { descriptorSetLayouts[0] };
		descPoolAndSets.setPerRenderPipeline[pipeline.second] = DescriptorSet::Create(&descSetPerViewCI);
		
		for (auto& rbd : rbds[0])
		{
			const uint32_t& binding = rbd.binding;
			const std::string& name = arc::ToUpper(rbd.name);
			if (rbd.structSize > 0)
			{
				if (SetUpdateTypeMap.find(name) == SetUpdateTypeMap.end())
					continue;
				if (SetUpdateTypeMap[name] != SetUpdateType::PER_VIEW)
					continue;
			}

			if (name.compare("CAMERA") == 0 && m_Camera)
			{
				descPoolAndSets.setPerRenderPipeline[pipeline.second]->AddBuffer(0, binding, { { m_Camera->GetUB()->GetBufferView() } });
			}
			else if (name.compare("TEXTCAMERA") == 0 && m_TextCamera)
			{
				descPoolAndSets.setPerRenderPipeline[pipeline.second]->AddBuffer(0, binding, { { m_TextCamera->GetUB()->GetBufferView() } });
			}
			else if (name.compare("LIGHTS") == 0 && !m_Lights.empty())
			{
				descPoolAndSets.setPerRenderPipeline[pipeline.second]->AddBuffer(0, binding, { { m_Lights[0]->GetUB()->GetBufferView() } });
			}

			else if (name.find("DIFFUSEIRRADIANCE") == 0 && m_Skybox)
			{
				const Ref<Texture>& skyboxTexture = m_Skybox->GetGeneratedDiffuseCubemap();
				descPoolAndSets.setPerRenderPipeline[pipeline.second]->AddImage(0, binding, { { skyboxTexture->GetTextureSampler(), skyboxTexture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
			}
			else if (name.find("SPECULARIRRADIANCE") == 0 && m_Skybox)
			{
				const Ref<Texture>& skyboxTexture = m_Skybox->GetGeneratedSpecularCubemap();
				descPoolAndSets.setPerRenderPipeline[pipeline.second]->AddImage(0, binding, { { skyboxTexture->GetTextureSampler(), skyboxTexture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
			}
			else if (name.find("SPECULARBRDF_LUT") == 0 && m_Skybox)
			{
				const Ref<Texture>& skyboxTexture = m_Skybox->GetGeneratedSpecularBRDF_LUT();
				descPoolAndSets.setPerRenderPipeline[pipeline.second]->AddImage(0, binding, { { skyboxTexture->GetTextureSampler(), skyboxTexture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
			}

			else if (name.compare("HDRINFO") == 0 && m_Skybox)
			{
				descPoolAndSets.setPerRenderPipeline[pipeline.second]->AddBuffer(0, binding, { { m_Skybox->GetUB()->GetBufferView() } });
			}
			else if (name.compare("HDRINPUT") == 0 && m_RenderSurface)
			{
				const Ref<ImageView>& colourImageView = m_RenderSurface->GetHDRFramebuffers()[0]->GetCreateInfo().attachments[1];
				descPoolAndSets.setPerRenderPipeline[pipeline.second]->AddImage(0, binding, { { nullptr, colourImageView, Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
			}
			else if (name.compare("EMISSIVEINPUT") == 0 && m_RenderSurface)
			{
				const Ref<ImageView>& emissiveImageView = m_RenderSurface->GetHDRFramebuffers()[0]->GetCreateInfo().attachments[2];
				descPoolAndSets.setPerRenderPipeline[pipeline.second]->AddImage(0, binding, { { nullptr, emissiveImageView, Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
			}
			else if (name.compare("SOURCEIMAGE") == 0 && m_RenderSurface)
			{
				const Ref<ImageView>& sourceImageView = m_RenderSurface->GetHDRFramebuffers()[0]->GetCreateInfo().attachments[0];
				descPoolAndSets.setPerRenderPipeline[pipeline.second]->AddImage(0, binding, { { nullptr, sourceImageView, Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
			}
			else
				continue;
		}
		descPoolAndSets.setPerRenderPipeline[pipeline.second]->Update();
	}

	//Per model Descriptor Sets
	descPoolAndSets.setPerModel.clear();
	for (auto& model : allQueue)
	{
		const std::vector<Ref<DescriptorSetLayout>>& descriptorSetLayouts = s_RenderPipelines[model->GetPipelineName()]->GetDescriptorSetLayouts();
		const std::vector<std::vector<Shader::ResourceBindingDescription>> rbds = s_RenderPipelines[model->GetPipelineName()]->GetRBDs();

		if (descriptorSetLayouts.empty() || rbds.empty())
			continue;

		DescriptorSet::CreateInfo descSetPerModelCI;
		descSetPerModelCI.debugName = "GEAR_CORE_DescriptorSet_PerModel_" + indexString + ": " + model->GetDebugName();
		descSetPerModelCI.pDescriptorPool = descPoolAndSets.pool;
		descSetPerModelCI.pDescriptorSetLayouts = { descriptorSetLayouts[1] };
		descPoolAndSets.setPerModel[model] = DescriptorSet::Create(&descSetPerModelCI);

		for (auto& rbd : rbds[1])
		{
			const uint32_t& binding = rbd.binding;
			const std::string& name = arc::ToUpper(rbd.name);
			if (rbd.structSize > 0)
			{
				if (SetUpdateTypeMap.find(name) == SetUpdateTypeMap.end())
					continue;
				if (SetUpdateTypeMap[name] != SetUpdateType::PER_MODEL)
					continue;

				if (name.compare("MODEL") == 0)
				{
					descPoolAndSets.setPerModel[model]->AddBuffer(0, binding, { { model->GetUB()->GetBufferView() } });
				}
				else
					continue;
			}
		}
		descPoolAndSets.setPerModel[model]->Update();
	}

	//Per material Descriptor Sets
	descPoolAndSets.setPerMaterial.clear();
	for (auto& model : allQueue)
	{
		const std::vector<Ref<DescriptorSetLayout>>& descriptorSetLayouts = s_RenderPipelines[model->GetPipelineName()]->GetDescriptorSetLayouts();
		const std::vector<std::vector<Shader::ResourceBindingDescription>> rbds = s_RenderPipelines[model->GetPipelineName()]->GetRBDs();
		
		if (descriptorSetLayouts.empty() || rbds.empty())
			continue;

		for (auto& material : model->GetMesh()->GetMaterials())
		{
			DescriptorSet::CreateInfo descSetPerMaterialCI;
			descSetPerMaterialCI.debugName = "GEAR_CORE_DescriptorSet_PerMaterial_" + indexString + ": " + material->GetDebugName();
			descSetPerMaterialCI.pDescriptorPool = descPoolAndSets.pool;
			descSetPerMaterialCI.pDescriptorSetLayouts = { descriptorSetLayouts[2] };
			descPoolAndSets.setPerMaterial[material] = DescriptorSet::Create(&descSetPerMaterialCI);

			for (auto& rbd : rbds[2])
			{
				const uint32_t& binding = rbd.binding;
				const std::string& name = arc::ToUpper(rbd.name);
				if (rbd.structSize > 0)
				{
					if (SetUpdateTypeMap.find(name) == SetUpdateTypeMap.end())
						continue;
					if (SetUpdateTypeMap[name] != SetUpdateType::PER_MATERIAL)
						continue;
				}
				
				if (name.find("SKYBOX") == 0)
				{
					const Ref<Material>& material = m_Skybox->GetModel()->GetMesh()->GetMaterials()[0];
					const Ref<Texture>& skyboxTexture = m_Skybox->GetGeneratedCubemap();
					descPoolAndSets.setPerMaterial[material]->AddImage(0, binding, { { skyboxTexture->GetTextureSampler(), skyboxTexture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
				}

				else if (name.find("FONTATLAS") == 0)
				{
					const Ref<Texture>& texture = material->GetTextures()[Material::TextureType::ALBEDO];
					descPoolAndSets.setPerMaterial[material]->AddImage(0, binding, { { texture->GetTextureSampler(), texture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
				}

				else if (name.compare("PBRCONSTANTS") == 0)
				{
					descPoolAndSets.setPerMaterial[material]->AddBuffer(0, binding, { { material->GetUB()->GetBufferView() } });
				}
				else if (name.find("NORMAL") == 0)
				{
					const Ref<Texture>& texture = material->GetTextures()[Material::TextureType::NORMAL];
					descPoolAndSets.setPerMaterial[material]->AddImage(0, binding, { {texture->GetTextureSampler(), texture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } }); continue;
				}
				else if (name.find("ALBEDO") == 0)
				{
					const Ref<Texture>& texture = material->GetTextures()[Material::TextureType::ALBEDO];
					descPoolAndSets.setPerMaterial[material]->AddImage(0, binding, { {texture->GetTextureSampler(), texture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } }); continue;
				}
				else if (name.find("METALLIC") == 0)
				{
					const Ref<Texture>& texture = material->GetTextures()[Material::TextureType::METALLIC];
					descPoolAndSets.setPerMaterial[material]->AddImage(0, binding, { {texture->GetTextureSampler(), texture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } }); continue;
				}
				else if (name.find("ROUGHNESS") == 0)
				{
					const Ref<Texture>& texture = material->GetTextures()[Material::TextureType::ROUGHNESS];
					descPoolAndSets.setPerMaterial[material]->AddImage(0, binding, { {texture->GetTextureSampler(), texture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } }); continue;
				}
				else if (name.find("AMBIENTOCCLUSION") == 0)
				{
					const Ref<Texture>& texture = material->GetTextures()[Material::TextureType::AMBIENT_OCCLUSION];
					descPoolAndSets.setPerMaterial[material]->AddImage(0, binding, { {texture->GetTextureSampler(), texture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } }); continue;
				}
				else if (name.find("EMISSIVE") == 0)
				{
					const Ref<Texture>& texture = material->GetTextures()[Material::TextureType::EMISSIVE];
					descPoolAndSets.setPerMaterial[material]->AddImage(0, binding, { {texture->GetTextureSampler(), texture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } }); continue;
				}
				else
					continue;
			
			}
			descPoolAndSets.setPerMaterial[material]->Update();
		}
	}
}

void Renderer::Draw()
{
	Ref<CommandBuffer>& graphicsCmdBuffer = m_CommandPoolAndBuffers[CommandPool::QueueType::GRAPHICS].cmdBuffer;

	//Main Render
	GPUTask::GraphicsRenderPassBeginTaskInfo graphicsRenderPassBeginTI;
	graphicsRenderPassBeginTI.framebuffer = m_RenderSurface->GetMainFramebuffers()[m_FrameIndex];
	graphicsRenderPassBeginTI.clearValues = { {1.0f, 0}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} };
	GPUTask::CreateInfo beginMainRenderPassCI;
	beginMainRenderPassCI.debugName = "Begin MainRenderPass";
	beginMainRenderPassCI.task = GPUTask::Task::GRAPHICS_RENDER_PASS_BEGIN;
	beginMainRenderPassCI.pTaskInfo = &graphicsRenderPassBeginTI;
	beginMainRenderPassCI.srcGPUTasks = {};
	beginMainRenderPassCI.srcPipelineStages = {};
	beginMainRenderPassCI.cmdBuffer = graphicsCmdBuffer;
	beginMainRenderPassCI.cmdBufferIndex = m_FrameIndex;
	beginMainRenderPassCI.cmdBufferControls = GPUTask::CommandBufferBasicControlsBit::RESET_BEGIN;
	beginMainRenderPassCI.resetCmdBufferReleaseResource = false;
	beginMainRenderPassCI.beginCmdBufferUsage = CommandBuffer::UsageBit::SIMULTANEOUS;
	beginMainRenderPassCI.skipTask = false;
	Ref<GPUTask> beginMainRenderPass = CreateRef<GPUTask>(&beginMainRenderPassCI);
	beginMainRenderPass->Execute();

	GPUTask::RendererFunctionTaskInfo rendererFunctionTI;
	rendererFunctionTI.pRenderer = this;
	rendererFunctionTI.pfn = &Renderer::MainRenderLoop;
	rendererFunctionTI.pDescPoolAndSets = &(m_DescPoolAndSets[m_FrameIndex]);
	GPUTask::CreateInfo mainRenderLoopCI;
	mainRenderLoopCI.debugName = "MainRenderLoop";
	mainRenderLoopCI.task = GPUTask::Task::RENDERER_FUNCTION;
	mainRenderLoopCI.pTaskInfo = &rendererFunctionTI;
	mainRenderLoopCI.srcGPUTasks = { beginMainRenderPass };
	mainRenderLoopCI.srcPipelineStages = { PipelineStageBit(0) };
	mainRenderLoopCI.cmdBuffer = graphicsCmdBuffer;
	mainRenderLoopCI.cmdBufferIndex = m_FrameIndex;
	mainRenderLoopCI.cmdBufferControls = GPUTask::CommandBufferBasicControlsBit::NONE;
	mainRenderLoopCI.skipTask = false;
	Ref<GPUTask> mainRenderLoop = CreateRef<GPUTask>(&mainRenderLoopCI);
	mainRenderLoop->Execute();

	GPUTask::GraphicsRenderPassEndTaskInfo graphicsRenderPassEndTI;
	GPUTask::CreateInfo endMainRenderPassCI;
	endMainRenderPassCI.debugName = "End MainRenderPass";
	endMainRenderPassCI.task = GPUTask::Task::GRAPHICS_RENDER_PASS_END;
	endMainRenderPassCI.pTaskInfo = &graphicsRenderPassEndTI;
	endMainRenderPassCI.srcGPUTasks = { mainRenderLoop };
	endMainRenderPassCI.srcPipelineStages = { PipelineStageBit(0) };
	endMainRenderPassCI.cmdBuffer = graphicsCmdBuffer;
	endMainRenderPassCI.cmdBufferIndex = m_FrameIndex;
	endMainRenderPassCI.cmdBufferControls = GPUTask::CommandBufferBasicControlsBit::NONE;
	endMainRenderPassCI.skipTask = false;
	Ref<GPUTask>endMainRenderPass = CreateRef<GPUTask>(&endMainRenderPassCI);
	endMainRenderPass->Execute();

	//Post Processing Compute shaders
	graphicsCmdBuffer->BeginDebugLabel(m_FrameIndex, "PostProcessing::Bloom");
	Ref<Image> colourInputImage = graphicsRenderPassBeginTI.framebuffer->GetCreateInfo().attachments.back()->GetCreateInfo().pImage;
	PostProcessing::ClearImageViewsAndDescriptorSets(m_FrameIndex);
	PostProcessing::Bloom(graphicsCmdBuffer, m_FrameIndex, { colourInputImage, Barrier::AccessBit::COLOUR_ATTACHMENT_WRITE_BIT, Image::Layout::COLOUR_ATTACHMENT_OPTIMAL, PipelineStageBit::COLOUR_ATTACHMENT_OUTPUT_BIT });
	graphicsCmdBuffer->EndDebugLabel(m_FrameIndex);

	//HDRMapping
	graphicsRenderPassBeginTI.framebuffer = m_RenderSurface->GetHDRFramebuffers()[m_FrameIndex];
	graphicsRenderPassBeginTI.clearValues = { {0.25f, 0.25f, 0.25f, 1.0f} };
	GPUTask::CreateInfo beginHDRRenderPassCI;
	beginHDRRenderPassCI.debugName = "Begin HDRRenderPass";
	beginHDRRenderPassCI.task = GPUTask::Task::GRAPHICS_RENDER_PASS_BEGIN;
	beginHDRRenderPassCI.pTaskInfo = &graphicsRenderPassBeginTI;
	beginHDRRenderPassCI.srcGPUTasks = { endMainRenderPass };
	beginHDRRenderPassCI.srcPipelineStages = {};
	beginHDRRenderPassCI.cmdBuffer = graphicsCmdBuffer;
	beginHDRRenderPassCI.cmdBufferIndex = m_FrameIndex;
	beginHDRRenderPassCI.cmdBufferControls = GPUTask::CommandBufferBasicControlsBit::NONE;
	beginHDRRenderPassCI.resetCmdBufferReleaseResource = false;
	beginHDRRenderPassCI.beginCmdBufferUsage = CommandBuffer::UsageBit::SIMULTANEOUS;
	beginHDRRenderPassCI.skipTask = false;
	Ref<GPUTask> beginHDRRenderPass = CreateRef<GPUTask>(&beginHDRRenderPassCI);
	beginHDRRenderPass->Execute();

	rendererFunctionTI.pfn = &Renderer::HDRMapping;
	GPUTask::CreateInfo hdrMappingCI;
	hdrMappingCI.debugName = "HDRMapping";
	hdrMappingCI.task = GPUTask::Task::RENDERER_FUNCTION;
	hdrMappingCI.pTaskInfo = &rendererFunctionTI;
	hdrMappingCI.srcGPUTasks = { beginHDRRenderPass };
	hdrMappingCI.srcPipelineStages = { PipelineStageBit(0) };
	hdrMappingCI.cmdBuffer = graphicsCmdBuffer;
	hdrMappingCI.cmdBufferIndex = m_FrameIndex;
	hdrMappingCI.cmdBufferControls = GPUTask::CommandBufferBasicControlsBit::NONE;
	hdrMappingCI.skipTask = false;
	Ref<GPUTask> hdrMapping = CreateRef<GPUTask>(&hdrMappingCI);
	hdrMapping->Execute();

	//OSD Text
	rendererFunctionTI.pfn = &Renderer::DrawTextLines;
	GPUTask::CreateInfo drawTextLinesCI;
	drawTextLinesCI.debugName = "DrawTextLines";
	drawTextLinesCI.task = GPUTask::Task::RENDERER_FUNCTION;
	drawTextLinesCI.pTaskInfo = &rendererFunctionTI;
	drawTextLinesCI.srcGPUTasks = { hdrMapping };
	drawTextLinesCI.srcPipelineStages = { PipelineStageBit(0) };
	drawTextLinesCI.cmdBuffer = graphicsCmdBuffer;
	drawTextLinesCI.cmdBufferIndex = m_FrameIndex;
	drawTextLinesCI.cmdBufferControls = GPUTask::CommandBufferBasicControlsBit::NONE;
	drawTextLinesCI.skipTask = false;
	Ref<GPUTask> drawTextLines = CreateRef<GPUTask>(&drawTextLinesCI);
	drawTextLines->Execute();

	//OSD Axes
	rendererFunctionTI.pfn = &Renderer::DrawCoordinateAxes;
	GPUTask::CreateInfo drawCoordinateAxesCI;
	drawCoordinateAxesCI.debugName = "DrawCoordinateAxes";
	drawCoordinateAxesCI.task = GPUTask::Task::RENDERER_FUNCTION;
	drawCoordinateAxesCI.pTaskInfo = &rendererFunctionTI;
	drawCoordinateAxesCI.srcGPUTasks = { drawTextLines };
	drawCoordinateAxesCI.srcPipelineStages = { PipelineStageBit(0) };
	drawCoordinateAxesCI.cmdBuffer = graphicsCmdBuffer;
	drawCoordinateAxesCI.cmdBufferIndex = m_FrameIndex;
	drawCoordinateAxesCI.cmdBufferControls = GPUTask::CommandBufferBasicControlsBit::NONE;
	drawCoordinateAxesCI.skipTask = false;
	Ref<GPUTask> drawCoordinateAxes = CreateRef<GPUTask>(&drawCoordinateAxesCI);
	drawCoordinateAxes->Execute();

	GPUTask::CreateInfo endHDRRenderPassCI;
	endHDRRenderPassCI.debugName = "End HDRRenderPass";
	endHDRRenderPassCI.task = GPUTask::Task::GRAPHICS_RENDER_PASS_END;
	endHDRRenderPassCI.pTaskInfo = &graphicsRenderPassEndTI;
	endHDRRenderPassCI.srcGPUTasks = { drawCoordinateAxes };
	endHDRRenderPassCI.srcPipelineStages = { PipelineStageBit(0) };
	endHDRRenderPassCI.cmdBuffer = graphicsCmdBuffer;
	endHDRRenderPassCI.cmdBufferIndex = m_FrameIndex;
	endHDRRenderPassCI.cmdBufferControls = GPUTask::CommandBufferBasicControlsBit::NONE;
	endHDRRenderPassCI.skipTask = false;
	Ref<GPUTask>endHDRRenderPass = CreateRef<GPUTask>(&endHDRRenderPassCI);
	endHDRRenderPass->Execute();

	//Copy To Swapchain
	graphicsRenderPassBeginTI.framebuffer = m_CI.window->GetSwapchainFramebuffers()[m_FrameIndex];
	graphicsRenderPassBeginTI.clearValues = { {0.25f, 0.25f, 0.25f, 1.0f} };
	GPUTask::CreateInfo beginSwapchainRenderPassCI;
	beginSwapchainRenderPassCI.debugName = "Begin SwapchainRenderPass";
	beginSwapchainRenderPassCI.task = GPUTask::Task::GRAPHICS_RENDER_PASS_BEGIN;
	beginSwapchainRenderPassCI.pTaskInfo = &graphicsRenderPassBeginTI;
	beginSwapchainRenderPassCI.srcGPUTasks = { endHDRRenderPass };
	beginSwapchainRenderPassCI.srcPipelineStages = {};
	beginSwapchainRenderPassCI.cmdBuffer = graphicsCmdBuffer;
	beginSwapchainRenderPassCI.cmdBufferIndex = m_FrameIndex;
	beginSwapchainRenderPassCI.cmdBufferControls = GPUTask::CommandBufferBasicControlsBit::NONE;
	beginSwapchainRenderPassCI.resetCmdBufferReleaseResource = false;
	beginSwapchainRenderPassCI.beginCmdBufferUsage = CommandBuffer::UsageBit::SIMULTANEOUS;
	beginSwapchainRenderPassCI.skipTask = false;
	Ref<GPUTask> beginSwapchainRenderPass = CreateRef<GPUTask>(&beginSwapchainRenderPassCI);
	beginSwapchainRenderPass->Execute();

	Ref<GPUTask> copyToSwapchain;
	if (m_CI.shouldCopyToSwapchian)
	{
		rendererFunctionTI.pfn = &Renderer::CopyToSwapchain;
		GPUTask::CreateInfo copyToSwapchainCI;
		copyToSwapchainCI.debugName = "CopyToSwapchain";
		copyToSwapchainCI.task = GPUTask::Task::RENDERER_FUNCTION;
		copyToSwapchainCI.pTaskInfo = &rendererFunctionTI;
		copyToSwapchainCI.srcGPUTasks = { beginSwapchainRenderPass };
		copyToSwapchainCI.srcPipelineStages = { PipelineStageBit(0) };
		copyToSwapchainCI.cmdBuffer = graphicsCmdBuffer;
		copyToSwapchainCI.cmdBufferIndex = m_FrameIndex;
		copyToSwapchainCI.cmdBufferControls = GPUTask::CommandBufferBasicControlsBit::NONE;
		copyToSwapchainCI.skipTask = false;
		copyToSwapchain = CreateRef<GPUTask>(&copyToSwapchainCI);
		copyToSwapchain->Execute();
	}
	if (m_CI.shouldDrawExternalUI)
	{
		rendererFunctionTI.pfn = &Renderer::DrawExternalUI;
		GPUTask::CreateInfo copyToSwapchainCI;
		copyToSwapchainCI.debugName = "DrawExternalUI";
		copyToSwapchainCI.task = GPUTask::Task::RENDERER_FUNCTION;
		copyToSwapchainCI.pTaskInfo = &rendererFunctionTI;
		copyToSwapchainCI.srcGPUTasks = { beginSwapchainRenderPass };
		copyToSwapchainCI.srcPipelineStages = { PipelineStageBit(0) };
		copyToSwapchainCI.cmdBuffer = graphicsCmdBuffer;
		copyToSwapchainCI.cmdBufferIndex = m_FrameIndex;
		copyToSwapchainCI.cmdBufferControls = GPUTask::CommandBufferBasicControlsBit::NONE;
		copyToSwapchainCI.skipTask = false;
		copyToSwapchain = CreateRef<GPUTask>(&copyToSwapchainCI);
		copyToSwapchain->Execute();
	}

	GPUTask::CreateInfo endSwapchainRenderPassCI;
	endSwapchainRenderPassCI.debugName = "End SwapchainRenderPass";
	endSwapchainRenderPassCI.task = GPUTask::Task::GRAPHICS_RENDER_PASS_END;
	endSwapchainRenderPassCI.pTaskInfo = &graphicsRenderPassEndTI;
	endSwapchainRenderPassCI.srcGPUTasks = { copyToSwapchain };
	endSwapchainRenderPassCI.srcPipelineStages = { PipelineStageBit(0) };
	endSwapchainRenderPassCI.cmdBuffer = graphicsCmdBuffer;
	endSwapchainRenderPassCI.cmdBufferIndex = m_FrameIndex;
	endSwapchainRenderPassCI.cmdBufferControls = GPUTask::CommandBufferBasicControlsBit::END;
	endSwapchainRenderPassCI.skipTask = false;
	Ref<GPUTask>endSwapchainRenderPass = CreateRef<GPUTask>(&endSwapchainRenderPassCI);
	endSwapchainRenderPass->Execute();
}

void Renderer::Execute()
{
	m_DrawFences[m_FrameIndex]->Wait();
	//Record Upload CmdBuffers
	{
		Upload();
	}
	//Record Present CmdBuffers
	{
		BuildDescriptorSetandPools();
		Draw();
	}
}

void Renderer::MainRenderLoop(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t frameIndex, const DescriptorPoolAndSets& descPoolAndSets)
{
	if (!m_Camera)
		return;

	for (auto& model : m_ModelQueue)
	{
		const Ref<graphics::RenderPipeline>& renderPipeline = s_RenderPipelines[model->GetPipelineName()];
		const Ref<Pipeline>& pipeline = renderPipeline->GetPipeline();

		cmdBuffer->BindPipeline(frameIndex, pipeline);

		for (size_t i = 0; i < model->GetMesh()->GetVertexBuffers().size(); i++)
		{
			Ref<objects::Material> material = model->GetMesh()->GetMaterials()[i];
			cmdBuffer->BindDescriptorSets(frameIndex,
				{
					descPoolAndSets.setPerRenderPipeline.at(renderPipeline),
					descPoolAndSets.setPerModel.at(model),
					descPoolAndSets.setPerMaterial.at(material)
				},
				pipeline);


			cmdBuffer->BindVertexBuffers(frameIndex, { model->GetMesh()->GetVertexBuffers()[i]->GetVertexBufferView() });
			cmdBuffer->BindIndexBuffer(frameIndex, model->GetMesh()->GetIndexBuffers()[i]->GetIndexBufferView());

			cmdBuffer->DrawIndexed(frameIndex, model->GetMesh()->GetIndexBuffers()[i]->GetCount());
		}
	}
	m_ModelQueue.clear();
}

void Renderer::HDRMapping(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t frameIndex, const DescriptorPoolAndSets& descPoolAndSets)
{
	if (!m_Skybox)
		return;

	const Ref<graphics::RenderPipeline>& renderPipeline = s_RenderPipelines["HDR"];
	const Ref<Pipeline>& pipeline = renderPipeline->GetPipeline();

	cmdBuffer->BindPipeline(frameIndex, pipeline);
	cmdBuffer->BindDescriptorSets(frameIndex, { descPoolAndSets.setPerRenderPipeline.at(renderPipeline) }, pipeline);
	cmdBuffer->Draw(frameIndex, 3);
}

void Renderer::DrawTextLines(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t frameIndex, const DescriptorPoolAndSets& descPoolAndSets)
{
	for (auto& model : m_TextQueue)
	{
		const Ref<graphics::RenderPipeline>& renderPipeline = s_RenderPipelines[model->GetPipelineName()];
		const Ref<Pipeline>& pipeline = renderPipeline->GetPipeline();

		cmdBuffer->BindPipeline(frameIndex, pipeline);

		for (size_t i = 0; i < model->GetMesh()->GetVertexBuffers().size(); i++)
		{
			Ref<objects::Material> material = model->GetMesh()->GetMaterials()[i];
			cmdBuffer->BindDescriptorSets(frameIndex, 
				{ 
					descPoolAndSets.setPerRenderPipeline.at(renderPipeline),
					descPoolAndSets.setPerModel.at(model),
					descPoolAndSets.setPerMaterial.at(material)
				},
				pipeline);

			cmdBuffer->BindVertexBuffers(frameIndex, { model->GetMesh()->GetVertexBuffers()[i]->GetVertexBufferView() });
			cmdBuffer->BindIndexBuffer(frameIndex, model->GetMesh()->GetIndexBuffers()[i]->GetIndexBufferView());

			cmdBuffer->DrawIndexed(frameIndex, model->GetMesh()->GetIndexBuffers()[i]->GetCount());
		}
	}
	m_TextQueue.clear();
}

void Renderer::DrawCoordinateAxes(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t frameIndex, const DescriptorPoolAndSets& descPoolAndSets)
{
	if (!m_Camera)
		return;

	const Ref<graphics::RenderPipeline>& renderPipeline = s_RenderPipelines["DebugCoordinateAxes"];
	const Ref<Pipeline>& pipeline = renderPipeline->GetPipeline();

	cmdBuffer->BindPipeline(frameIndex, pipeline);
	cmdBuffer->BindDescriptorSets(frameIndex, { descPoolAndSets.setPerRenderPipeline.at(renderPipeline) }, pipeline);
	cmdBuffer->Draw(frameIndex, 6);
}

void Renderer::CopyToSwapchain(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t frameIndex, const DescriptorPoolAndSets& descPoolAndSets)
{
	const Ref<graphics::RenderPipeline>& renderPipeline = s_RenderPipelines["DebugCopy"];
	const Ref<Pipeline>& pipeline = renderPipeline->GetPipeline();

	cmdBuffer->BindPipeline(frameIndex, pipeline);
	cmdBuffer->BindDescriptorSets(m_FrameIndex, { descPoolAndSets.setPerRenderPipeline.at(renderPipeline) }, pipeline);
	cmdBuffer->Draw(frameIndex, 3);
}

void Renderer::DrawExternalUI(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t frameIndex, const DescriptorPoolAndSets& descPoolAndSets)
{
	if (m_UI_PFN && m_DrawData && m_UI_this)
	{
		m_UI_PFN(cmdBuffer, frameIndex, m_DrawData, m_UI_this);
	}
}

void Renderer::Present(bool& windowResize)
{
	const Ref<CommandBuffer>& graphicsCmdBuffer = m_CommandPoolAndBuffers[CommandPool::QueueType::GRAPHICS].cmdBuffer;
	if (m_CI.shouldPresent)
		graphicsCmdBuffer->Present({ 0, 1 }, m_CI.window->GetSwapchain(), m_DrawFences, m_AcquireSemaphores, m_SubmitSemaphores, windowResize);
	else
		graphicsCmdBuffer->Submit({ m_FrameIndex }, {}, {}, {}, m_DrawFences[m_FrameIndex]); //Otherwise submit the commandbuffer to draw anyway.
	
	m_FrameIndex = (m_FrameIndex + 1) % m_SwapchainImageCount;
	m_FrameCount++;
}

void Renderer::ResizeRenderPipelineViewports(uint32_t width, uint32_t height)
{
	m_Context->DeviceWaitIdle();
	for (auto& renderPipeline : s_RenderPipelines)
	{
		renderPipeline.second->m_CI.viewportState.viewports = { { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f } };
		renderPipeline.second->m_CI.viewportState.scissors = { { { 0, 0 },{ width, height } } };
		renderPipeline.second->Rebuild();
	}
}

void Renderer::RecompileRenderPipelineShaders()
{
	m_Context->DeviceWaitIdle();
	for (auto& renderPipeline : s_RenderPipelines)
		renderPipeline.second->RecompileShaders();
}

void Renderer::ReloadTextures()
{
	m_Context->DeviceWaitIdle();
	m_ReloadTextures = true;
}
