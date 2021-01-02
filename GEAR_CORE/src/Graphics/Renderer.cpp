#include "gear_core_common.h"
#include "Renderer.h"
#include "Core/StringConversion.h"
#include "ImageProcessing.h"
#include "FrameGraph.h"

using namespace gear;
using namespace graphics;
using namespace objects;

using namespace miru;
using namespace miru::crossplatform;

Renderer::Renderer(const miru::Ref<Context>& context)
{
	//Renderer and Transfer CmdPools and CmdBuffers
	m_CmdPoolCI.debugName = "GEAR_CORE_CommandPool_Renderer";
	m_CmdPoolCI.pContext = context;
	m_CmdPoolCI.flags = CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
	m_CmdPoolCI.queueType = CommandPool::QueueType::GRAPHICS;
	m_CmdPool = CommandPool::Create(&m_CmdPoolCI);

	m_CmdBufferCI.debugName = "GEAR_CORE_CommandBuffer_Renderer";
	m_CmdBufferCI.pCommandPool = m_CmdPool;
	m_CmdBufferCI.level = CommandBuffer::Level::PRIMARY;
	m_CmdBufferCI.commandBufferCount = 4;
	m_CmdBufferCI.allocateNewCommandPoolPerBuffer = GraphicsAPI::IsD3D12();
	m_CmdBuffer = CommandBuffer::Create(&m_CmdBufferCI);

	m_TransCmdPoolCI.debugName = "GEAR_CORE_CommandPool_Renderer_Transfer";
	m_TransCmdPoolCI.pContext = context;
	m_TransCmdPoolCI.flags = CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
	m_TransCmdPoolCI.queueType = CommandPool::QueueType::TRANSFER;
	m_TransCmdPool = CommandPool::Create(&m_TransCmdPoolCI);

	m_TransCmdBufferCI.debugName = "GEAR_CORE_CommandBuffer_Renderer_Transfer";
	m_TransCmdBufferCI.pCommandPool = m_TransCmdPool;
	m_TransCmdBufferCI.level = CommandBuffer::Level::PRIMARY;
	m_TransCmdBufferCI.commandBufferCount = 1;
	m_TransCmdBufferCI.allocateNewCommandPoolPerBuffer = GraphicsAPI::IsD3D12();
	m_TransCmdBuffer = CommandBuffer::Create(&m_TransCmdBufferCI);

	m_Context = context;
	m_Device = context->GetDevice();
	
	//Present Synchronisation
	m_DrawFenceCI.debugName = "GEAR_CORE_Fence_Renderer_Draw";
	m_DrawFenceCI.device = m_Device;
	m_DrawFenceCI.signaled = true;
	m_DrawFenceCI.timeout = UINT64_MAX;
	m_DrawFences = { Fence::Create(&m_DrawFenceCI), Fence::Create(&m_DrawFenceCI) };

	m_AcquireSemaphoreCI.debugName = "GEAR_CORE_Seamphore_Renderer_Acquire";
	m_AcquireSemaphoreCI.device = m_Device;
	m_AcquireSemaphores = { Semaphore::Create(&m_AcquireSemaphoreCI), Semaphore::Create(&m_AcquireSemaphoreCI) };

	m_SubmitSemaphoreCI.debugName = "GEAR_CORE_Seamphore_Renderer_Submit";
	m_SubmitSemaphoreCI.device = m_Device;
	m_SubmitSemaphores = { Semaphore::Create(&m_SubmitSemaphoreCI), Semaphore::Create(&m_SubmitSemaphoreCI) };

}

Renderer::~Renderer()
{
	m_Context->DeviceWaitIdle();
}

void Renderer::InitialiseRenderPipelines(const std::vector<std::string>& filepaths, float viewportWidth, float viewportHeight, Image::SampleCountBit samples, const miru::Ref<RenderPass>& renderPass)
{
	RenderPipeline::LoadInfo renderPipelineLI;
	for (auto& filepath : filepaths)
	{
		//TODO: What if the Pipeline is already loaded?
		renderPipelineLI.device = m_Device;
		renderPipelineLI.filepath = filepath;
		renderPipelineLI.viewportWidth = viewportWidth;
		renderPipelineLI.viewportHeight = viewportHeight;
		renderPipelineLI.samples = samples;
		renderPipelineLI.renderPass = renderPass;
		renderPipelineLI.subpassIndex = 0;
		Ref<RenderPipeline> renderPipeline = CreateRef<RenderPipeline>(&renderPipelineLI);
		m_RenderPipelines[renderPipeline->m_CI.debugName] = renderPipeline;
	}
}

void Renderer::SubmitFramebuffer(const miru::Ref<miru::crossplatform::Framebuffer>* framebuffers)
{ 
	m_Framebuffers = framebuffers; 
}

void Renderer::SubmitCamera(const gear::Ref<objects::Camera>& camera)
{ 
	m_Camera = camera; 
}

void Renderer::SubmitFontCamera(const gear::Ref<Camera>& fontCamera)
{
	m_FontCamera = fontCamera; 
}

void Renderer::SubmitLights(const std::vector<gear::Ref<Light>>& lights)
{ 
	m_Lights = lights; 
}

void Renderer::SubmitSkybox(const gear::Ref<Skybox>& skybox)
{ 
	m_Skybox = skybox; 
	SubmitModel(skybox->GetModel()); 
}

void Renderer::SubmitModel(const gear::Ref<Model>& obj)
{
	m_RenderQueue.push_back(obj);
}

void Renderer::Upload(bool forceUploadCamera, bool forceUploadLights, bool forceUploadSkybox, bool forceUploadMeshes)
{
	//Get Texture Barries and/or Reload Textures
	std::set<gear::Ref<Texture>> texturesToProcess;
	std::vector<gear::Ref<Texture>> texturesToGenerateMipmaps;

	std::vector<miru::Ref<Barrier>> textureShaderReadOnlyBarrierToTransferDst;
	std::vector<miru::Ref<Barrier>> textureUnknownToTransferDstBarrier;
	std::vector<miru::Ref<Barrier>> textureTransferDstToShaderReadOnlyBarrier;
	std::vector<miru::Ref<Barrier>> textureGeneralToShaderReadOnlyBarrier;

	//Get all unique textures
	for (auto& model : m_RenderQueue)
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
	{
		if (!m_Skybox->m_Cubemap && !m_Skybox->m_Generated)
		{
			m_Skybox->GetGeneratedSpecularBRDF_LUT()->TransitionSubResources(textureGeneralToShaderReadOnlyBarrier,
				{ { Barrier::AccessBit::SHADER_WRITE_BIT, Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT,
				Image::Layout::GENERAL, Image::Layout::SHADER_READ_ONLY_OPTIMAL, {}, true } });

			m_Skybox->GetGeneratedSpecularCubemap()->TransitionSubResources(textureGeneralToShaderReadOnlyBarrier,
				{ { Barrier::AccessBit::SHADER_WRITE_BIT, Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT,
				Image::Layout::GENERAL, Image::Layout::SHADER_READ_ONLY_OPTIMAL, {}, true } });
			
			m_Skybox->GetGeneratedDiffuseCubemap()->TransitionSubResources(textureGeneralToShaderReadOnlyBarrier,
				{ { Barrier::AccessBit::SHADER_WRITE_BIT, Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT,
				Image::Layout::GENERAL, Image::Layout::SHADER_READ_ONLY_OPTIMAL, {}, true } });
			
			m_Skybox->GetGeneratedCubemap()->TransitionSubResources(textureGeneralToShaderReadOnlyBarrier,
				{ { Barrier::AccessBit::SHADER_WRITE_BIT, Barrier::AccessBit::SHADER_READ_BIT | Barrier::AccessBit::SHADER_WRITE_BIT,
				Image::Layout::GENERAL, Image::Layout::SHADER_READ_ONLY_OPTIMAL, {}, true } });
	
			m_Skybox->GetTexture()->TransitionSubResources(textureUnknownToTransferDstBarrier,
				{ { Barrier::AccessBit::NONE_BIT, Barrier::AccessBit::TRANSFER_WRITE_BIT,
				Image::Layout::UNKNOWN, Image::Layout::TRANSFER_DST_OPTIMAL, {}, true } });
			m_Skybox->GetTexture()->m_PreUpload = false;
	
			m_Skybox->GetTexture()->TransitionSubResources(textureGeneralToShaderReadOnlyBarrier,
				{ { Barrier::AccessBit::SHADER_WRITE_BIT, Barrier::AccessBit::SHADER_READ_BIT,
				Image::Layout::GENERAL, Image::Layout::SHADER_READ_ONLY_OPTIMAL, {}, true } });
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
			texture->TransitionSubResources(textureShaderReadOnlyBarrierToTransferDst,
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

	bool preTransferGraphicsTask = textureShaderReadOnlyBarrierToTransferDst.size();
	bool preUploadTransferTask = textureUnknownToTransferDstBarrier.size();

	bool transferTask = forceUploadCamera || forceUploadLights || forceUploadMeshes || forceUploadSkybox;
	bool asyncComputeTask = texturesToGenerateMipmaps.size() || !m_Skybox->m_Generated;

	bool postComputeGraphicsTask = textureGeneralToShaderReadOnlyBarrier.size();
	bool postTransferGraphicsTask = textureTransferDstToShaderReadOnlyBarrier.size();

	gear::Ref<Node> preTransferGraphicsNode, preUploadForTextrueTransferNode, uploadTransferNode, postComputeGraphicsNode, postTransferGraphicsNode;
	Node::TransitionResourcesTaskInfo trti;

	//Pre-Transfer Graphics Task
	{
		trti.srcPipelineStage = PipelineStageBit::FRAGMENT_SHADER_BIT;
		trti.dstPipelineStage = PipelineStageBit::TRANSFER_BIT;
		trti.barriers = textureShaderReadOnlyBarrierToTransferDst;

		Node::CreateInfo preTransferGraphicsNodeCI;
		preTransferGraphicsNodeCI.debugName = "Pre-Transfer - Graphics";
		preTransferGraphicsNodeCI.task = Node::Task::TRANSITION_RESOURCES;
		preTransferGraphicsNodeCI.pTaskInfo = &trti;
		preTransferGraphicsNodeCI.srcNodes = {};
		preTransferGraphicsNodeCI.srcPipelineStages = {};
		preTransferGraphicsNodeCI.cmdBuffer = m_CmdBuffer;
		preTransferGraphicsNodeCI.cmdBufferIndex = 3;
		preTransferGraphicsNodeCI.resetCmdBuffer = true;
		preTransferGraphicsNodeCI.submitCmdBuffer = true;
		preTransferGraphicsNodeCI.skipTask = !preTransferGraphicsTask;

		preTransferGraphicsNode = gear::CreateRef<Node>(&preTransferGraphicsNodeCI);
		preTransferGraphicsNode->Execute();
	}

	//Pre-Upload Transfer Task
	{
		trti.srcPipelineStage = PipelineStageBit::TOP_OF_PIPE_BIT;
		trti.dstPipelineStage = PipelineStageBit::TRANSFER_BIT;
		trti.barriers = textureUnknownToTransferDstBarrier;

		Node::CreateInfo preUploadForTextrueTransferNodeCI;
		preUploadForTextrueTransferNodeCI.debugName = "Pre-Upload - Transfer";
		preUploadForTextrueTransferNodeCI.task = Node::Task::TRANSITION_RESOURCES;
		preUploadForTextrueTransferNodeCI.pTaskInfo = &trti;
		preUploadForTextrueTransferNodeCI.srcNodes = { preTransferGraphicsNode };
		preUploadForTextrueTransferNodeCI.srcPipelineStages = { PipelineStageBit::TRANSFER_BIT };
		preUploadForTextrueTransferNodeCI.cmdBuffer = m_TransCmdBuffer;
		preUploadForTextrueTransferNodeCI.cmdBufferIndex = 0;
		preUploadForTextrueTransferNodeCI.resetCmdBuffer = true;
		preUploadForTextrueTransferNodeCI.submitCmdBuffer = false;
		preUploadForTextrueTransferNodeCI.skipTask = !preUploadTransferTask;

		preUploadForTextrueTransferNode = gear::CreateRef<Node>(&preUploadForTextrueTransferNodeCI);
		preUploadForTextrueTransferNode->Execute();
	}

	//Upload Transfer Task
	{
		Node::UploadResourceTaskInfo urti;
		urti.camera = m_Camera;
		urti.cameraForce = forceUploadCamera;
		urti.fontCamera = m_FontCamera;
		urti.fontCameraForce = forceUploadCamera;
		urti.skybox = m_Skybox;
		urti.skyboxForce = forceUploadSkybox;
		urti.lights = m_Lights;
		urti.lightsForce = forceUploadLights;
		urti.models = m_RenderQueue;
		urti.modelsForce = forceUploadMeshes;
		urti.materialsForce = false;

		Node::CreateInfo uploadTransferNodeCI;
		uploadTransferNodeCI.debugName = "Upload - Transfer";
		uploadTransferNodeCI.task = Node::Task::UPLOAD_RESOURCES;
		uploadTransferNodeCI.pTaskInfo = &urti;
		uploadTransferNodeCI.srcNodes = { preUploadForTextrueTransferNode };
		uploadTransferNodeCI.srcPipelineStages = { (PipelineStageBit)0 };
		uploadTransferNodeCI.cmdBuffer = m_TransCmdBuffer;
		uploadTransferNodeCI.cmdBufferIndex = 0;
		uploadTransferNodeCI.resetCmdBuffer = false;
		uploadTransferNodeCI.submitCmdBuffer = true;
		uploadTransferNodeCI.skipTask = !transferTask;

		uploadTransferNode = gear::CreateRef<Node>(&uploadTransferNodeCI);
		uploadTransferNode->Execute();
	}

	//Async Compute Task
	{
		if (asyncComputeTask && transferTask)
			uploadTransferNode->GetFence()->Wait();

		if (asyncComputeTask)
		{
			for (auto& texture : texturesToGenerateMipmaps)
			{
				ImageProcessing::GenerateMipMaps({ texture, Barrier::AccessBit::TRANSFER_WRITE_BIT, Image::Layout::TRANSFER_DST_OPTIMAL, PipelineStageBit::TRANSFER_BIT });
			}
			if (!m_Skybox->m_Cubemap && !m_Skybox->m_Generated)
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
			}
		}
	}

	//Post-Compute Graphics Task
	{
		trti.srcPipelineStage = PipelineStageBit::COMPUTE_SHADER_BIT;
		trti.dstPipelineStage = PipelineStageBit::FRAGMENT_SHADER_BIT;
		trti.barriers = textureGeneralToShaderReadOnlyBarrier;

		Node::CreateInfo postComputeGraphicsNodeCI;
		postComputeGraphicsNodeCI.debugName = "Post-Compute - Graphics";
		postComputeGraphicsNodeCI.task = Node::Task::TRANSITION_RESOURCES;
		postComputeGraphicsNodeCI.pTaskInfo = &trti;
		postComputeGraphicsNodeCI.srcNodes = { uploadTransferNode };
		postComputeGraphicsNodeCI.srcPipelineStages = { PipelineStageBit::COMPUTE_SHADER_BIT };
		postComputeGraphicsNodeCI.cmdBuffer = m_CmdBuffer;
		postComputeGraphicsNodeCI.cmdBufferIndex = 2;
		postComputeGraphicsNodeCI.resetCmdBuffer = true;
		postComputeGraphicsNodeCI.submitCmdBuffer = false;
		postComputeGraphicsNodeCI.skipTask = !postComputeGraphicsTask;

		postComputeGraphicsNode = gear::CreateRef<Node>(&postComputeGraphicsNodeCI);
		postComputeGraphicsNode->Execute();
	}

	//Post-Transfer Graphics CmdBuffer
	{
		trti.srcPipelineStage = PipelineStageBit::TRANSFER_BIT;
		trti.dstPipelineStage = PipelineStageBit::FRAGMENT_SHADER_BIT;
		trti.barriers = textureTransferDstToShaderReadOnlyBarrier;
	
		Node::CreateInfo postTransferGraphicsNodeCI;
		postTransferGraphicsNodeCI.debugName = "Post-Transfer - Graphics";
		postTransferGraphicsNodeCI.task = Node::Task::TRANSITION_RESOURCES;
		postTransferGraphicsNodeCI.pTaskInfo = &trti;
		postTransferGraphicsNodeCI.srcNodes = { postComputeGraphicsNode };
		postTransferGraphicsNodeCI.srcPipelineStages = { (PipelineStageBit)0 };
		postTransferGraphicsNodeCI.cmdBuffer = m_CmdBuffer;
		postTransferGraphicsNodeCI.cmdBufferIndex = 2;
		postTransferGraphicsNodeCI.resetCmdBuffer = false;
		postTransferGraphicsNodeCI.submitCmdBuffer = true;
		postTransferGraphicsNodeCI.skipTask = !postTransferGraphicsTask;
	
		postTransferGraphicsNode = gear::CreateRef<Node>(&postTransferGraphicsNodeCI);
		postTransferGraphicsNode->Execute();
	}

	//Wait for all Task Fences
	preTransferGraphicsNode->GetFence()->Wait();
	uploadTransferNode->GetFence()->Wait();
	postTransferGraphicsNode->GetFence()->Wait();
}

void Renderer::Flush()
{
	if(!m_BuiltDescPoolsAndSets)
	{
		//Desriptor Pool
		std::map<DescriptorType, uint32_t> poolSizesMap;
		size_t m_RenderQueueMaterialCount = 0;
		for (auto& model : m_RenderQueue)
		{
			const miru::Ref<Pipeline>& pipeline = m_RenderPipelines[model->GetPipelineName()]->GetPipeline();
			const std::vector<std::vector<Shader::ResourceBindingDescription>> rbds = m_RenderPipelines[model->GetPipelineName()]->GetRBDs();
			size_t materialCount = model->GetMesh()->GetMaterials().size();
			m_RenderQueueMaterialCount += materialCount;

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
		}
		m_DescPoolCI.debugName = "GEAR_CORE_DescriptorPool_Renderer";
		m_DescPoolCI.device = m_Device;
		for (auto& poolSize : poolSizesMap)
			m_DescPoolCI.poolSizes.push_back({ poolSize.first, poolSize.second });
		m_DescPoolCI.maxSets = static_cast<uint32_t>(m_RenderPipelines.size() + m_RenderQueue.size() + m_RenderQueueMaterialCount);
		m_DescPool = DescriptorPool::Create(&m_DescPoolCI);

		//Per view Descriptor Set
		for (auto& pipeline : m_RenderPipelines)
		{
			const std::vector<miru::Ref<DescriptorSetLayout>>& descriptorSetLayouts = pipeline.second->GetDescriptorSetLayouts();
			const std::vector<std::vector<Shader::ResourceBindingDescription>> rbds = pipeline.second->GetRBDs();

			DescriptorSet::CreateInfo descSetPerViewCI;
			descSetPerViewCI.debugName = "GEAR_CORE_DescriptorSet_PerView: " + pipeline.second->GetPipeline()->GetCreateInfo().debugName;
			descSetPerViewCI.pDescriptorPool = m_DescPool;
			descSetPerViewCI.pDescriptorSetLayouts = { descriptorSetLayouts[0] };
			m_DescSetPerView[pipeline.second] = DescriptorSet::Create(&descSetPerViewCI);
			
			for (auto& rbd : rbds[0])
			{
				const uint32_t& binding = rbd.binding;
				const std::string& name = core::toupper(rbd.name);
				if (rbd.structSize > 0)
				{
					if (SetUpdateTypeMap.find(name) == SetUpdateTypeMap.end())
						continue;
					if (SetUpdateTypeMap[name] != SetUpdateType::PER_VIEW)
						continue;
				}

				if (name.compare("CAMERA") == 0)
				{
					m_DescSetPerView[pipeline.second]->AddBuffer(0, binding, { { m_Camera->GetUB()->GetBufferView() } });
				}
				else if (name.compare("FONTCAMERA") == 0)
				{
					m_DescSetPerView[pipeline.second]->AddBuffer(0, binding, { { m_FontCamera->GetUB()->GetBufferView() } });
				}
				else if (name.compare("LIGHTS") == 0)
				{
					m_DescSetPerView[pipeline.second]->AddBuffer(0, binding, { { m_Lights[0]->GetUB()->GetBufferView() } });
				}

				else if (name.find("DIFFUSEIRRADIANCE") == 0)
				{
					const gear::Ref<Texture>& skyboxTexture = m_Skybox->GetGeneratedDiffuseCubemap();
					m_DescSetPerView[pipeline.second]->AddImage(0, binding, { { skyboxTexture->GetTextureSampler(), skyboxTexture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
				}
				else if (name.find("SPECULARIRRADIANCE") == 0)
				{
					const gear::Ref<Texture>& skyboxTexture = /*m_Skybox->GetGeneratedCubemap();*/ m_Skybox->GetGeneratedSpecularCubemap();
					m_DescSetPerView[pipeline.second]->AddImage(0, binding, { { skyboxTexture->GetTextureSampler(), skyboxTexture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
				}
				else if (name.find("SPECULARBRDF_LUT") == 0)
				{
					const gear::Ref<Texture>& skyboxTexture = m_Skybox->GetGeneratedSpecularBRDF_LUT();
					m_DescSetPerView[pipeline.second]->AddImage(0, binding, { { skyboxTexture->GetTextureSampler(), skyboxTexture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
				}
			
				
				else
					continue;
			}
			m_DescSetPerView[pipeline.second]->Update();
		}

		//Per model Descriptor Sets
		for (auto& model : m_RenderQueue)
		{
			const std::vector<miru::Ref<DescriptorSetLayout>>& descriptorSetLayouts = m_RenderPipelines[model->GetPipelineName()]->GetDescriptorSetLayouts();
			const std::vector<std::vector<Shader::ResourceBindingDescription>> rbds = m_RenderPipelines[model->GetPipelineName()]->GetRBDs();

			DescriptorSet::CreateInfo descSetPerModelCI;
			descSetPerModelCI.debugName = "GEAR_CORE_DescriptorSet_PerModel: " + model->GetDebugName();
			descSetPerModelCI.pDescriptorPool = m_DescPool;
			descSetPerModelCI.pDescriptorSetLayouts = { descriptorSetLayouts[1] };
			m_DescSetPerModel[model] = DescriptorSet::Create(&descSetPerModelCI);

			for (auto& rbd : rbds[1])
			{
				const uint32_t& binding = rbd.binding;
				const std::string& name = core::toupper(rbd.name);
				if (rbd.structSize > 0)
				{
					if (SetUpdateTypeMap.find(name) == SetUpdateTypeMap.end())
						continue;
					if (SetUpdateTypeMap[name] != SetUpdateType::PER_MODEL)
						continue;

					if (name.compare("MODEL") == 0)
					{
						m_DescSetPerModel[model]->AddBuffer(0, binding, { { model->GetUB()->GetBufferView() } });
					}
					else
						continue;
				}
			}
			m_DescSetPerModel[model]->Update();
		}

		//Per material Descriptor Sets
		for (auto& model : m_RenderQueue)
		{
			const std::vector<miru::Ref<DescriptorSetLayout>>& descriptorSetLayouts = m_RenderPipelines[model->GetPipelineName()]->GetDescriptorSetLayouts();
			const std::vector<std::vector<Shader::ResourceBindingDescription>> rbds = m_RenderPipelines[model->GetPipelineName()]->GetRBDs();
			
			for (auto& material : model->GetMesh()->GetMaterials())
			{
				DescriptorSet::CreateInfo descSetPerMaterialCI;
				descSetPerMaterialCI.debugName = "GEAR_CORE_DescriptorSet_PerMaterial: " + material->GetDebugName();
				descSetPerMaterialCI.pDescriptorPool = m_DescPool;
				descSetPerMaterialCI.pDescriptorSetLayouts = { descriptorSetLayouts[2] };
				m_DescSetPerMaterial[material] = DescriptorSet::Create(&descSetPerMaterialCI);

				for (auto& rbd : rbds[2])
				{
					const uint32_t& binding = rbd.binding;
					const std::string& name = core::toupper(rbd.name);
					if (rbd.structSize > 0)
					{
						if (SetUpdateTypeMap.find(name) == SetUpdateTypeMap.end())
							continue;
						if (SetUpdateTypeMap[name] != SetUpdateType::PER_MATERIAL)
							continue;
					}
					
					if (name.compare("SKYBOXINFO") == 0)
					{
						const gear::Ref<Material>& material = m_Skybox->GetModel()->GetMesh()->GetMaterials()[0];
						m_DescSetPerMaterial[material]->AddBuffer(0, binding, { { m_Skybox->GetUB()->GetBufferView() } });
					}
					else if (name.find("SKYBOX") == 0)
					{
						const gear::Ref<Material>& material = m_Skybox->GetModel()->GetMesh()->GetMaterials()[0];
						const gear::Ref<Texture>& skyboxTexture = m_Skybox->GetGeneratedCubemap();
						m_DescSetPerMaterial[material]->AddImage(0, binding, { { skyboxTexture->GetTextureSampler(), skyboxTexture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
					}

					else if (name.find("FONTATLAS") == 0)
					{
						const gear::Ref<Texture>& texture = material->GetTextures()[Material::TextureType::ALBEDO];
						m_DescSetPerMaterial[material]->AddImage(0, binding, { { texture->GetTextureSampler(), texture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
					}

					else if (name.compare("PBRCONSTANTS") == 0)
					{
						m_DescSetPerMaterial[material]->AddBuffer(0, binding, { { material->GetUB()->GetBufferView() } });
					}
					else if (name.find("NORMAL") == 0)
					{
						const gear::Ref<Texture>& texture = material->GetTextures()[Material::TextureType::NORMAL];
						m_DescSetPerMaterial[material]->AddImage(0, binding, { {texture->GetTextureSampler(), texture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } }); continue;
					}
					else if (name.find("ALBEDO") == 0)
					{
						const gear::Ref<Texture>& texture = material->GetTextures()[Material::TextureType::ALBEDO];
						m_DescSetPerMaterial[material]->AddImage(0, binding, { {texture->GetTextureSampler(), texture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } }); continue;
					}
					else if (name.find("METALLIC") == 0)
					{
						const gear::Ref<Texture>& texture = material->GetTextures()[Material::TextureType::METALLIC];
						m_DescSetPerMaterial[material]->AddImage(0, binding, { {texture->GetTextureSampler(), texture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } }); continue;
					}
					else if (name.find("ROUGHNESS") == 0)
					{
						const gear::Ref<Texture>& texture = material->GetTextures()[Material::TextureType::ROUGHNESS];
						m_DescSetPerMaterial[material]->AddImage(0, binding, { {texture->GetTextureSampler(), texture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } }); continue;
					}
					else if (name.find("AMBIENTOCCLUSION") == 0)
					{
						const gear::Ref<Texture>& texture = material->GetTextures()[Material::TextureType::AMBIENT_OCCLUSION];
						m_DescSetPerMaterial[material]->AddImage(0, binding, { {texture->GetTextureSampler(), texture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } }); continue;
					}
					else if (name.find("EMISSIVE") == 0)
					{
						const gear::Ref<Texture>& texture = material->GetTextures()[Material::TextureType::EMISSIVE];
						m_DescSetPerMaterial[material]->AddImage(0, binding, { {texture->GetTextureSampler(), texture->GetTextureImageView(), Image::Layout::SHADER_READ_ONLY_OPTIMAL } }); continue;
					}
					else
						continue;
				
				}
				m_DescSetPerMaterial[material]->Update();
			}
		}

		m_BuiltDescPoolsAndSets = true;
	}

	//Record Present CmdBuffers
	m_DrawFences[m_FrameIndex]->Wait();
	{
		m_CmdBuffer->Reset(m_FrameIndex, false);
		m_CmdBuffer->Begin(m_FrameIndex, CommandBuffer::UsageBit::SIMULTANEOUS);
		m_CmdBuffer->BeginRenderPass(m_FrameIndex, m_Framebuffers[m_FrameIndex], { {0.25f, 0.25f, 0.25f, 1.0f}, {1.0f, 0} });

		for (auto& model : m_RenderQueue)
		{
			const gear::Ref<graphics::RenderPipeline>& renderPipeline = m_RenderPipelines[model->GetPipelineName()];
			const miru::Ref<Pipeline>& pipeline = renderPipeline->GetPipeline();

			m_CmdBuffer->BindPipeline(m_FrameIndex, pipeline);

			for (size_t i = 0; i < model->GetMesh()->GetVertexBuffers().size(); i++)
			{
				gear::Ref<objects::Material> material = model->GetMesh()->GetMaterials()[i];
				m_CmdBuffer->BindDescriptorSets(m_FrameIndex, { m_DescSetPerView[renderPipeline], m_DescSetPerModel[model], m_DescSetPerMaterial[material] }, pipeline);
				
				m_CmdBuffer->BindVertexBuffers(m_FrameIndex, { model->GetMesh()->GetVertexBuffers()[i]->GetVertexBufferView() });
				m_CmdBuffer->BindIndexBuffer(m_FrameIndex, model->GetMesh()->GetIndexBuffers()[i]->GetIndexBufferView());

				m_CmdBuffer->DrawIndexed(m_FrameIndex, model->GetMesh()->GetIndexBuffers()[i]->GetCount());
			}
		}
		m_CmdBuffer->EndRenderPass(m_FrameIndex);
		m_CmdBuffer->End(m_FrameIndex);
	}
	m_RenderQueue.clear();
}

void Renderer::Present(const miru::Ref<Swapchain>& swapchain, bool& windowResize)
{
	m_CmdBuffer->Present({ 0, 1 }, swapchain, m_DrawFences, m_AcquireSemaphores, m_SubmitSemaphores, windowResize);
	m_FrameIndex = (m_FrameIndex + 1) % swapchain->GetCreateInfo().swapchainCount;
	m_FrameCount++;
}

void Renderer::ResizeRenderPipelineViewports(uint32_t width, uint32_t height)
{
	m_Context->DeviceWaitIdle();
	for (auto& renderPipeline : m_RenderPipelines)
	{
		renderPipeline.second->m_CI.viewportState.viewports = { { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f } };
		renderPipeline.second->m_CI.viewportState.scissors = { { { 0, 0 },{ width, height } } };
		renderPipeline.second->Rebuild();
	}
}

void Renderer::RecompileRenderPipelineShaders()
{
	m_Context->DeviceWaitIdle();
	for (auto& renderPipeline : m_RenderPipelines)
		renderPipeline.second->RecompileShaders();
}

void Renderer::ReloadTextures()
{
	m_Context->DeviceWaitIdle();
	m_ReloadTextures = true;
}
