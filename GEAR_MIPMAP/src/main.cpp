#include "gear_core.h"

#include "stb_image.h"
#include "stb_image_write.h"

#include "ErrorCodes.h"
#include "GMMDocumentation.h"

using namespace gear;
using namespace graphics;
using namespace mipmap;

using namespace miru;
using namespace miru::crossplatform;

static ErrorCode error = ErrorCode::GEAR_MIPMAP_OK;
#define ASSERT(ptr) if (!ptr) { error = ErrorCode::GEAR_MIPMAP_ERROR; GEAR_MIPMAP_RETURN(error, "Failed to create MIRU renderering primitive"); }

int main(int argc, const char** argv)
{
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
	if (!(mars::Utility::IsPowerOf2(imageWidth) && mars::Utility::IsPowerOf2(imageHeight)))
	{
		error = ErrorCode::GEAR_MIPMAP_IMAGE_FILE_NOT_POW_OF_2;
		GEAR_MIPMAP_RETURN(error, "An image file with non-power of 2 dimensions has been passed to GEAR_MIPMAP.");
	}
	uint32_t maxLevels = static_cast<uint32_t>(log2(static_cast<double>(std::min(imageWidth, imageHeight)))) + 1;
	if (levels == 1)
		levels = maxLevels;
	else
		levels = std::min(maxLevels, levels);

	size_t imageMipmapSize = static_cast<size_t>(imageDataSize * 1.5); //Allocate more the mipmaps!
	std::vector<uint8_t> imageDataArray(imageMipmapSize);
	memcpy(imageDataArray.data(), imageData, imageDataSize);
	stbi_image_free(imageData);

	//---MIRU--- and ---GEAR---
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
	miru::Ref<Context> context = Context::Create(&contextCI);
	ASSERT(context);

	AllocatorManager::CreateInfo mbmCI;
	mbmCI.pContext = context;
	mbmCI.defaultBlockSize = Allocator::BlockSize::BLOCK_SIZE_128MB;
	AllocatorManager::Initialise(&mbmCI);

	Texture::CreateInfo texCI;
	texCI.debugName = "GEAR_MIPMAP_InputTexture";
	texCI.device = context->GetDevice();
	texCI.dataType = Texture::DataType::DATA;
	texCI.data.data = imageDataArray.data();
	texCI.data.size = imageDataArray.size();
	texCI.data.width = imageWidth;
	texCI.data.height = imageHeight;
	texCI.data.depth = 1;
	texCI.mipLevels = levels;
	texCI.arrayLayers = 1;
	texCI.type = miru::crossplatform::Image::Type::TYPE_2D;
	texCI.format = miru::crossplatform::Image::Format::R8G8B8A8_UNORM;
	texCI.samples = miru::crossplatform::Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	texCI.usage = miru::crossplatform::Image::UsageBit(0);
	texCI.generateMipMaps = true;
	gear::Ref<Texture> inputTexture = gear::CreateRef<Texture>(&texCI);

	//CommandPool and CommandBuffer
	CommandPool::CreateInfo transferCmdPoolCI;
	transferCmdPoolCI.debugName = "GEAR_MIPMAP_CommandPoolCompute";
	transferCmdPoolCI.pContext = context;
	transferCmdPoolCI.flags = CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
	transferCmdPoolCI.queueType = CommandPool::QueueType::TRANSFER;
	miru::Ref<CommandPool> transferCmdPool = CommandPool::Create(&transferCmdPoolCI);
	ASSERT(transferCmdPool);

	CommandBuffer::CreateInfo transferCmdBufferCI;
	transferCmdBufferCI.debugName = "GEAR_MIPMAP_CommandBufferCompute";
	transferCmdBufferCI.pCommandPool = transferCmdPool;
	transferCmdBufferCI.level = CommandBuffer::Level::PRIMARY;
	transferCmdBufferCI.commandBufferCount = 1;
	transferCmdBufferCI.allocateNewCommandPoolPerBuffer = false;
	miru::Ref<CommandBuffer> transferCmdBuffer = CommandBuffer::Create(&transferCmdBufferCI);
	ASSERT(transferCmdBuffer);

	//Fences
	Fence::CreateInfo fenceCI;
	fenceCI.debugName = "GEAR_MIPMAP_FenceUpload";
	fenceCI.device = context->GetDevice();
	fenceCI.signaled = false;
	fenceCI.timeout = UINT64_MAX;
	miru::Ref<Fence> uploadFence = Fence::Create(&fenceCI);
	fenceCI.debugName = "GEAR_MIPMAP_FenceDownload";
	miru::Ref<Fence> downloadFence = Fence::Create(&fenceCI);

	//Record upload
	{
		transferCmdBuffer->Reset(0, false);
		transferCmdBuffer->Begin(0, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);

		std::vector<miru::Ref<Barrier>> barriers;
		inputTexture->TransitionSubResources(barriers, { { Barrier::AccessBit::NONE, Barrier::AccessBit::TRANSFER_WRITE_BIT, Image::Layout::UNKNOWN, Image::Layout::TRANSFER_DST_OPTIMAL, {}, true } });
		transferCmdBuffer->PipelineBarrier(0, PipelineStageBit::TOP_OF_PIPE_BIT, PipelineStageBit::TRANSFER_BIT, DependencyBit::NONE_BIT, barriers);

		inputTexture->Upload(transferCmdBuffer);
		transferCmdBuffer->End(0);
	}
	transferCmdBuffer->Submit({ 0 }, {}, {}, {}, uploadFence);
	uploadFence->Wait();

	//GenerateMipmaps
	inputTexture->GenerateMipMaps();

	//Record download
	{
		transferCmdBuffer->Reset(0, false);
		transferCmdBuffer->Begin(0, CommandBuffer::UsageBit::ONE_TIME_SUBMIT);
		
		std::vector<miru::Ref<Barrier>> barriers;
		inputTexture->TransitionSubResources(barriers, { { Barrier::AccessBit::NONE, Barrier::AccessBit::TRANSFER_READ_BIT, Image::Layout::TRANSFER_DST_OPTIMAL, Image::Layout::TRANSFER_SRC_OPTIMAL, {}, true } });
		transferCmdBuffer->PipelineBarrier(0, PipelineStageBit::TOP_OF_PIPE_BIT, PipelineStageBit::TRANSFER_BIT, DependencyBit::NONE_BIT, barriers);
	
		inputTexture->Download(transferCmdBuffer);
		transferCmdBuffer->End(0);
	}
	transferCmdBuffer->Submit({ 0 }, {}, {}, {}, downloadFence);
	downloadFence->Wait();

	inputTexture->AccessImageData(imageDataArray);

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