#include "miru_core.h"
#pragma comment(lib, "MIRU_CORE.lib")

#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include "STBI/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "STBI/stb_image_write.h"

#include "ErrorCodes.h"
#include "GMMDocumentation.h"

using namespace gear;
using namespace mipmap;

using namespace miru;
using namespace crossplatform;

static ErrorCode error = ErrorCode::GEAR_MIPMAP_OK;
#define ASSERT(ptr) if (!ptr) { error = ErrorCode::GEAR_MIPMAP_ERROR; GEAR_MIPMAP_RETURN(error, "Failed to create MIRU renderering primitive"); }

int main(int argc, const char** argv)
{
	system("BuildShaders.bat");
	system("CLS");

	//Null arguments
	if (!argc)
	{
		error = ErrorCode::GEAR_MIPMAP_NO_ARGS;
		GEAR_MIPMAP_RETURN(error, "No arguments passed to GEAR_MIPMAP.");
	}

	//Application Header, Help documentation and Debug
	bool logo = true;
	bool pause = false;
	bool help = false;
	for (int i = 0; i < argc; i++)
	{
		if (!_stricmp(argv[i], "-h") || !_stricmp(argv[i], "-help"))
			help = true;
		if (!_stricmp(argv[i], "-pause"))
		{
			pause = true; help = true;
		}
		if (!_stricmp(argv[i], "-nologo"))
			logo = false;
		if (!_stricmp(argv[i], "-nooutput"))
			output = false;
	}
	if (logo)
		GEAR_MIPMAP_PRINTF("GEAR_MIPMAP: Copyright © 2020 Andrew Richards.\n\n");
	if (help)
	{
		GEAR_MIPMAP_PRINTF(help_doucumentation);
		GEAR_MIPMAP_PRINTF("\n");
	}

	//Get Filepath, Directories and others
	std::string filepath, outputDir;
	uint32_t levels = 1;
	const size_t tagSize = std::string("-X:").size();
	for (int i = 0; i < argc; i++)
	{
		std::string tempFilepath = argv[i];
		if (tempFilepath.find("-f:") != std::string::npos || tempFilepath.find("-F:") != std::string::npos)
		{
			tempFilepath.erase(0, tagSize);
			filepath = tempFilepath;
		}
		if (tempFilepath.find("-o:") != std::string::npos || tempFilepath.find("-O:") != std::string::npos)
		{
			tempFilepath.erase(0, tagSize);
			outputDir = tempFilepath;
		}
		if (tempFilepath.find("-levels:") != std::string::npos || tempFilepath.find("-LEVELS:") != std::string::npos)
		{
			tempFilepath.erase(0, std::string("-levels:").size());
			levels = static_cast<uint32_t>(atoi(tempFilepath.c_str()));
		}
	}
	if (filepath.empty())
	{
		error = ErrorCode::GEAR_MIPMAP_NO_IMAGE_FILE;
		GEAR_MIPMAP_RETURN(error, "No shader has been passed to GEAR_MIPMAP.");
	}
	if (outputDir.empty())
	{
		size_t fileNamePos = filepath.find_last_of('/');
		outputDir = filepath.substr(0, fileNamePos + 1);
	}

	//STB Image load file
	uint32_t imageWidth, imageHeight, imageChannels;
	uint8_t* imageData = stbi_load(filepath.c_str(), (int*)&imageWidth, (int*)&imageHeight, (int*)&imageChannels, STBI_rgb_alpha);
	size_t imageDataSize = imageWidth * imageHeight * STBI_rgb_alpha;
	if (imageWidth == 0 || imageHeight == 0 || imageChannels == 0 || imageDataSize == 0 || imageData == nullptr)
	{
		error = ErrorCode::GEAR_MIPMAP_IMAGE_FILE_INVALID;
		GEAR_MIPMAP_RETURN(error, "No valid image file has been passed to GEAR_MIPMAP.");
	}
	auto IsPowerOf2 = [](uint32_t x) -> bool { return !(x == 0) && !(x & (x - 1)); };
	if (!(IsPowerOf2(imageWidth) && IsPowerOf2(imageHeight)))
	{
		error = ErrorCode::GEAR_MIPMAP_IMAGE_FILE_NOT_POW_OF_2;
		GEAR_MIPMAP_RETURN(error, "An image file with non-power of 2 dimensions has been passed to GEAR_MIPMAP.");
	}
	uint32_t maxLevels = static_cast<uint32_t>(log2(static_cast<double>(std::min(imageWidth, imageHeight)))) + 1;
	if (levels == 1)
		levels = maxLevels;
	else
		levels = std::min(maxLevels, levels);
	levels = 2; //TODO: !!! REMOVE !!!

	size_t imageMipmapSize = static_cast<size_t>(imageDataSize * 1.5); //Allocate more the mipmaps!
	std::vector<uint8_t> imageDataArray(imageMipmapSize);
	memcpy(imageDataArray.data(), imageData, imageDataSize);
	stbi_image_free(imageData);

	//---MIRU---
	//Set Graphics API
	GraphicsAPI::API api = GraphicsAPI::API::UNKNOWN;
	for (int i = 0; i < argc; i++)
	{
		if (!_stricmp(argv[i], "-vk") || !_stricmp(argv[i], "-vulkan"))
			api = GraphicsAPI::API::VULKAN;
		if (!_stricmp(argv[i], "-dx12") || !_stricmp(argv[i], "-d3d12"))
			api = GraphicsAPI::API::D3D12;
	}
	if (api == GraphicsAPI::API::UNKNOWN)
	{
		error = ErrorCode::GEAR_MIPMAP_MIRU_NO_GRAPHICS_API;
		GEAR_MIPMAP_RETURN(error, "No no graphics api arguements passed to GEAR_MIPMAP.");
	}
	GraphicsAPI::SetAPI(api);
	//GraphicsAPI::LoadGraphicsDebugger(debug::GraphicsDebugger::DebuggerType::RENDER_DOC);
	//auto rdc = (debug::RenderDoc*)GraphicsAPI::GetGraphicsDebugger().get();

	//Create Context
	Context::CreateInfo contextCI;
	#ifdef _DEBUG
	contextCI.applicationName = "GEAR_MIPMAP(x64) Debug";
	contextCI.instanceLayers = { "VK_LAYER_KHRONOS_validation" };
	contextCI.instanceExtensions = {};
	contextCI.deviceLayers = { "VK_LAYER_KHRONOS_validation" };
	contextCI.deviceExtensions = {};
	#else
	contextCI.applicationName = "GEAR_MIPMAP(x64)";
	contextCI.instanceLayers = {};
	contextCI.instanceExtensions = {};
	contextCI.deviceLayers = {};
	contextCI.deviceExtensions = {};
	#endif
	contextCI.api_version_major = GraphicsAPI::IsD3D12() ? 11 : 1;
	contextCI.api_version_minor = 1;
	contextCI.deviceDebugName = "GEAR_MIPMAP_Context";
	Ref<Context> context = Context::Create(&contextCI);
	ASSERT(context);

	//Create Memory Blocks
	MemoryBlock::BlockSize blockSize = MemoryBlock::BlockSize(imageMipmapSize);

	MemoryBlock::CreateInfo cpuMBCI;
	cpuMBCI.debugName = "GEAR_MIPMAP_CPU_MB";
	cpuMBCI.pContext = context;
	cpuMBCI.blockSize = blockSize;
	cpuMBCI.properties = MemoryBlock::PropertiesBit::HOST_VISIBLE_BIT | MemoryBlock::PropertiesBit::HOST_COHERENT_BIT;
	Ref<MemoryBlock> cpuMB = MemoryBlock::Create(&cpuMBCI);
	ASSERT(cpuMB);

	MemoryBlock::CreateInfo gpuMBCI;
	gpuMBCI.debugName = "GEAR_MIPMAP_GPU_MB";
	gpuMBCI.pContext = context;
	gpuMBCI.blockSize = blockSize;
	gpuMBCI.properties = MemoryBlock::PropertiesBit::DEVICE_LOCAL_BIT;
	Ref<MemoryBlock> gpuMB = MemoryBlock::Create(&gpuMBCI);
	ASSERT(gpuMB);

	//Create CPU Side storage
	Buffer::CreateInfo imageBufferCI;
	imageBufferCI.debugName = "GEAR_MIPMAP_ImageBufferCPU";
	imageBufferCI.device = context->GetDevice();
	imageBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC | Buffer::UsageBit::TRANSFER_DST;
	imageBufferCI.imageDimension.width = imageWidth;
	imageBufferCI.imageDimension.height = imageHeight;
	imageBufferCI.imageDimension.channels = STBI_rgb_alpha;
	imageBufferCI.size = imageDataArray.size();
	imageBufferCI.data = imageDataArray.data();
	imageBufferCI.pMemoryBlock = cpuMB;
	Ref<Buffer> imageBuffer = Buffer::Create(&imageBufferCI);
	ASSERT(imageBuffer);

	//Create GPU Side storage
	Image::CreateInfo imageCI;
	imageCI.debugName = "GEAR_MIPMAP_ImageGPU";
	imageCI.device = context->GetDevice();
	imageCI.type = Image::Type::TYPE_2D;
	imageCI.format = Image::Format::R8G8B8A8_UNORM;
	imageCI.width = imageWidth;
	imageCI.height = imageHeight;
	imageCI.depth = 1;
	imageCI.mipLevels = levels;
	imageCI.arrayLayers = 1;
	imageCI.sampleCount = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	imageCI.usage = Image::UsageBit::TRANSFER_DST_BIT | Image::UsageBit::TRANSFER_SRC_BIT | Image::UsageBit::SAMPLED_BIT | Image::UsageBit::STORAGE_BIT;
	imageCI.layout = Image::Layout::UNKNOWN;
	imageCI.size = 0;
	imageCI.data = nullptr;
	imageCI.pMemoryBlock = gpuMB;
	Ref<Image> image = Image::Create(&imageCI);
	ASSERT(image);

	//Create ImageView and Sampler
	std::vector<Ref<ImageView>> imageViews;
	imageViews.reserve(levels);
	ImageView::CreateInfo imageViewCI;
	imageViewCI.debugName = "GEAR_MIPMAP_ImageView";
	imageViewCI.device = context->GetDevice();
	imageViewCI.pImage = image;
	for (uint32_t i = 0; i < levels; i++)
	{
		imageViewCI.subresourceRange = { Image::AspectBit::COLOUR_BIT, i, 1, 0, 1 };
		imageViews.emplace_back(ImageView::Create(&imageViewCI));
		ASSERT(imageViews.back());
	}

	Sampler::CreateInfo samplerCI;
	samplerCI.debugName = "GEAR_MIPMAP_Sampler";
	samplerCI.device = context->GetDevice();
	samplerCI.magFilter = Sampler::Filter::LINEAR;
	samplerCI.minFilter = Sampler::Filter::LINEAR;;
	samplerCI.mipmapMode = Sampler::MipmapMode::LINEAR;
	samplerCI.addressModeU = Sampler::AddressMode::CLAMP_TO_EDGE;
	samplerCI.addressModeV = Sampler::AddressMode::CLAMP_TO_EDGE;
	samplerCI.addressModeW = Sampler::AddressMode::CLAMP_TO_EDGE;
	samplerCI.mipLodBias = 1.0f;
	samplerCI.anisotropyEnable = false;
	samplerCI.maxAnisotropy = 1.0f;
	samplerCI.compareEnable = false;
	samplerCI.compareOp = CompareOp::NEVER;
	samplerCI.minLod = 0.0f;
	samplerCI.maxLod = 1.0f;
	samplerCI.borderColour = Sampler::BorderColour::FLOAT_OPAQUE_BLACK;
	samplerCI.unnormalisedCoordinates = false;
	Ref<Sampler> sampler = Sampler::Create(&samplerCI);

	//CommandPool and CommandBuffer
	CommandPool::CreateInfo computeCmdPoolCI;
	computeCmdPoolCI.debugName = "GEAR_MIPMAP_CommandPoolCompute";
	computeCmdPoolCI.pContext = context;
	computeCmdPoolCI.flags = CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
	computeCmdPoolCI.queueFamilyIndex = 1;
	Ref<CommandPool> computeCmdPool = CommandPool::Create(&computeCmdPoolCI);
	ASSERT(computeCmdPool);

	CommandBuffer::CreateInfo computeCmdBufferCI;
	computeCmdBufferCI.debugName = "GEAR_MIPMAP_CommandBufferCompute";
	computeCmdBufferCI.pCommandPool = computeCmdPool;
	computeCmdBufferCI.level = CommandBuffer::Level::PRIMARY;
	computeCmdBufferCI.commandBufferCount = 1;
	computeCmdBufferCI.allocateNewCommandPoolPerBuffer = false;
	Ref<CommandBuffer> computeCmdBuffer = CommandBuffer::Create(&computeCmdBufferCI);
	ASSERT(computeCmdBuffer);

	//Compute Shader and Pipeline
	Shader::CreateInfo shaderCI;
	shaderCI.debugName = "GEAR_MIPMAP_ShaderCSGenerateMipMaps";
	shaderCI.device = context->GetDevice();
	shaderCI.stage = Shader::StageBit::COMPUTE_BIT;
	shaderCI.entryPoint = "main";
	shaderCI.binaryFilepath = "res/shaders/bin/GenerateMipMaps.comp.spv";
	shaderCI.binaryCode = {};
	shaderCI.recompileArguments;
	shaderCI.recompileArguments.mscDirectory = "../GEAR_CORE/dep/MIRU/MIRU_SHADER_COMPILER/exe/x64/Debug";
	shaderCI.recompileArguments.hlslFilepath = "res/shaders/HLSL/GenerateMipMaps.comp.hlsl";
	shaderCI.recompileArguments.outputDirectory = "res/shaders/bin/";
	shaderCI.recompileArguments.includeDirectories = { "../GEAR_CORE/dep/MIRU/MIRU_SHADER_COMPILER/shaders/includes" };
	shaderCI.recompileArguments.entryPoint = "";
	shaderCI.recompileArguments.shaderModel = "";
	shaderCI.recompileArguments.macros = {};
	shaderCI.recompileArguments.cso = true;
	shaderCI.recompileArguments.spv = true;
	shaderCI.recompileArguments.dxcLocation = "";
	shaderCI.recompileArguments.glslangLocation = "";
	shaderCI.recompileArguments.additioalArguments = "";
	shaderCI.recompileArguments.nologo = false;
	shaderCI.recompileArguments.nooutput = false;
	Ref<Shader> shader = Shader::Create(&shaderCI);
	ASSERT(shader);

	DescriptorSetLayout::CreateInfo descSetLayoutCI;
	descSetLayoutCI.debugName = "GEAR_MIPMAP_DescriptorSetLayout";;
	descSetLayoutCI.device = context->GetDevice();
	for (auto& rbdSet : shader->GetRBDs())
	{
		for (auto& rbd : rbdSet.second)
		{
			descSetLayoutCI.descriptorSetLayoutBinding.push_back({
				rbd.second.binding,
				rbd.second.type,
				rbd.second.descriptorCount,
				rbd.second.stage });
		}
	}
	Ref<DescriptorSetLayout>descSetLayout = DescriptorSetLayout::Create(&descSetLayoutCI);

	Pipeline::CreateInfo computePipelineCI;
	computePipelineCI.debugName = "GEAR_MIPMAP_ComputePipelineCSGenerateMipMaps";
	computePipelineCI.device = context->GetDevice();
	computePipelineCI.type = PipelineType::COMPUTE;
	computePipelineCI.shaders = { shader };
	computePipelineCI.layout = { {descSetLayout}, {} };
	Ref<Pipeline> computePipeline = Pipeline::Create(&computePipelineCI);
	ASSERT(computePipeline);

	//Create DescriptorPool and DescriptorSet
	DescriptorPool::CreateInfo descPoolCI;
	descPoolCI.debugName = "GEAR_MIPMAP_DescriptorPool";
	descPoolCI.device = context->GetDevice();
	descPoolCI.poolSizes = { {DescriptorType::STORAGE_IMAGE, (levels - 1)}, {DescriptorType::COMBINED_IMAGE_SAMPLER, (levels - 1)} };
	descPoolCI.maxSets = levels - 1;
	Ref<DescriptorPool> descPool = DescriptorPool::Create(&descPoolCI);
	ASSERT(descPool);

	std::vector<Ref<DescriptorSet>> descSets;
	descSets.reserve(levels);
	DescriptorSet::CreateInfo descSetCI;
	descSetCI.debugName = "GEAR_MIPMAP_DescriptorSet";
	descSetCI.pDescriptorPool = descPool;
	descSetCI.pDescriptorSetLayouts = { descSetLayout };
	for (uint32_t i = 0; i < descPoolCI.maxSets; i++)
	{
		descSets.emplace_back(DescriptorSet::Create(&descSetCI));
		ASSERT(descSets.back());
		descSets[i]->AddImage(0, 0, { { sampler, nullptr, Image::Layout::UNKNOWN} });
		descSets[i]->AddImage(0, 1, { { nullptr, imageViews[i + 0], Image::Layout::SHADER_READ_ONLY_OPTIMAL } });
		descSets[i]->AddImage(0, 2, { { nullptr, imageViews[i + 1], Image::Layout::GENERAL } });
		descSets[i]->Update();
	}

	//Record CommandBuffer
	Fence::CreateInfo fenceCI;
	fenceCI.debugName = "GEAR_MIPMAP_FenceComputeTask";
	fenceCI.device = context->GetDevice();
	fenceCI.signaled = false;
	fenceCI.timeout = UINT64_MAX;
	Ref<Fence> fence = Fence::Create(&fenceCI);
	{
		computeCmdBuffer->Begin(0, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);

		Barrier::CreateInfo bCI;
		bCI.type = Barrier::Type::IMAGE;
		bCI.srcAccess = Barrier::AccessBit::NONE;
		bCI.dstAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
		bCI.srcQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
		bCI.dstQueueFamilyIndex = MIRU_QUEUE_FAMILY_IGNORED;
		bCI.pImage = image;
		bCI.oldLayout = Image::Layout::UNKNOWN;
		bCI.newLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
		bCI.subresoureRange = { Image::AspectBit::COLOUR_BIT, 0, levels, 0, 1 };
		Ref<Barrier> b0 = Barrier::Create(&bCI);

		computeCmdBuffer->PipelineBarrier(0, PipelineStageBit::TOP_OF_PIPE_BIT, PipelineStageBit::TRANSFER_BIT, DependencyBit::NONE_BIT, { b0 });

		computeCmdBuffer->CopyBufferToImage(0, imageBuffer, image, Image::Layout::TRANSFER_DST_OPTIMAL, { { 0, 0, 0, {Image::AspectBit::COLOUR_BIT, 0, 0, 1}, {0, 0, 0}, {imageWidth, imageHeight, 1} } }); //Copy only top mip.

		bCI.type = Barrier::Type::IMAGE;
		bCI.srcAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
		bCI.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
		bCI.pImage = image;
		bCI.oldLayout = Image::Layout::TRANSFER_DST_OPTIMAL;
		bCI.newLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
		bCI.subresoureRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
		Ref<Barrier> b1 = Barrier::Create(&bCI);

		bCI.newLayout = Image::Layout::GENERAL;
		bCI.subresoureRange = { Image::AspectBit::COLOUR_BIT, 1, (levels - 1), 0, 1 };
		Ref<Barrier> b1a = Barrier::Create(&bCI);

		bCI.type = Barrier::Type::BUFFER;
		bCI.srcAccess = Barrier::AccessBit::HOST_WRITE_BIT;
		bCI.dstAccess = Barrier::AccessBit::SHADER_READ_BIT;
		bCI.pBuffer = imageBuffer;
		bCI.offset = 0;
		bCI.size = imageMipmapSize;
		Ref<Barrier> b1b = Barrier::Create(&bCI);

		computeCmdBuffer->PipelineBarrier(0, PipelineStageBit::TRANSFER_BIT | PipelineStageBit::HOST_BIT, PipelineStageBit::COMPUTE_SHADER_BIT, DependencyBit::NONE_BIT, { b1, b1a, b1b });

		computeCmdBuffer->BindPipeline(0, computePipeline);
		computeCmdBuffer->BindDescriptorSets(0, { descSets[0] }, computePipeline);
		computeCmdBuffer->Dispatch(0, imageWidth / 8, imageHeight / 8, 1);

		bCI.type = Barrier::Type::IMAGE;
		bCI.srcAccess = Barrier::AccessBit::SHADER_WRITE_BIT;
		bCI.dstAccess = Barrier::AccessBit::TRANSFER_READ_BIT;
		bCI.pImage = image;
		bCI.oldLayout = Image::Layout::SHADER_READ_ONLY_OPTIMAL;
		bCI.newLayout = Image::Layout::TRANSFER_SRC_OPTIMAL;
		bCI.subresoureRange = { Image::AspectBit::COLOUR_BIT, 0, 1, 0, 1 };
		Ref<Barrier> b2 = Barrier::Create(&bCI);
		
		bCI.oldLayout = Image::Layout::GENERAL;
		bCI.subresoureRange = { Image::AspectBit::COLOUR_BIT, 1, (levels - 1), 0, 1 };
		Ref<Barrier> b2a = Barrier::Create(&bCI);

		computeCmdBuffer->PipelineBarrier(0, PipelineStageBit::COMPUTE_SHADER_BIT, PipelineStageBit::TRANSFER_BIT, DependencyBit::NONE_BIT, { b2, b2a });

		std::vector<Image::BufferImageCopy> bics;
		uint64_t bufferOffset = 0;
		for (uint32_t i = 0; i < levels; i++)
		{
			Image::BufferImageCopy bic;
			bic.bufferOffset = bufferOffset;
			bic.bufferRowLength = 0;
			bic.bufferImageHeight = 0;
			bic.imageSubresource = { Image::AspectBit::COLOUR_BIT, i, 0, 1 };
			bic.imageOffset = { 0, 0 ,0 };
			bic.imageExtent = { imageWidth >> i , imageHeight >> i, 1 };
			bics.push_back(bic);

			bufferOffset += (imageWidth >> i) * (imageHeight >> i) * STBI_rgb_alpha;
		}
		computeCmdBuffer->CopyImageToBuffer(0, image, imageBuffer, Image::Layout::TRANSFER_SRC_OPTIMAL, bics); //Copy all mips.

		bCI.type = Barrier::Type::BUFFER;
		bCI.srcAccess = Barrier::AccessBit::TRANSFER_WRITE_BIT;
		bCI.dstAccess = Barrier::AccessBit::HOST_READ_BIT;
		bCI.pBuffer = imageBuffer;
		bCI.offset = 0;
		bCI.size = imageMipmapSize;
		Ref<Barrier> b3 = Barrier::Create(&bCI);

		computeCmdBuffer->PipelineBarrier(0, PipelineStageBit::TRANSFER_BIT, PipelineStageBit::HOST_BIT, DependencyBit::NONE_BIT, { b3 });

		computeCmdBuffer->End(0);
	}
	computeCmdBuffer->Submit({ 0 }, {}, {}, PipelineStageBit::NONE_BIT, fence);
	fence->Wait();

	imageDataArray.clear();
	imageDataArray.resize(imageMipmapSize);
	cpuMB->AccessData(imageBuffer->GetResource(), imageDataArray.size(), imageDataArray.data());

	//Save out mipmaps
	size_t offset = 0;
	size_t fileNamePos = filepath.find_last_of('/') + 1;
	size_t extPos = filepath.find_last_of('.');
	for (uint32_t i = 0; i < levels; i++)
	{
		std::string outputFilpath = outputDir + filepath.substr(fileNamePos, extPos - fileNamePos) + "_" + std::to_string(i) + ".png";
		int writeImageWidth = imageWidth >> i;
		int writeImageHeight = imageHeight >> i;
		
		stbi_write_png(outputFilpath.c_str(), writeImageWidth, writeImageHeight, STBI_rgb_alpha, 
			imageDataArray.data() + offset, (writeImageWidth * STBI_rgb_alpha));
		
		offset += writeImageWidth * writeImageHeight * STBI_rgb_alpha;
		if (offset > imageDataArray.size())
		{
			error = ErrorCode::GEAR_MIPMAP_IMAGE_FILE_SAVE_ERROR;
			GEAR_MIPMAP_ERROR_CODE(error, ("GEAR_MIPMAP can not image file of level: " + std::to_string(i + 1) + ".").c_str());
			break;
		}
	}

	if (pause)
	{
		system("PAUSE");
	}
	GEAR_MIPMAP_PRINTF("\n");
	GEAR_MIPMAP_RETURN(error, "GEAR_MIPMAP returned an error.");
}