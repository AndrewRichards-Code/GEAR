#include "gear_core_common.h"
#include "Graphics/Rendering/RenderGraph.h"

#include "Graphics/AllocatorManager.h"
#include "Core/ParseStack.h"

using namespace gear;
using namespace graphics;
using namespace rendering;

using namespace miru;
using namespace base;

///////////////
//RenderGraph//
///////////////

RenderGraph::RenderGraph()
{
}

RenderGraph::RenderGraph(const ContextRef& context, uint32_t commandBufferCount)
{
	m_Context = context;

	//CmdPools and CmdBuffers
	for (uint32_t i = 0; i < 3; i++)
	{
		CommandPool::QueueType type = CommandPool::QueueType(i);
		std::string name;
		switch (type)
		{
		case CommandPool::QueueType::GRAPHICS:
			name = "Graphics";
			break;
		case CommandPool::QueueType::COMPUTE:
			name = "Compute";
			break;
		case CommandPool::QueueType::TRANSFER:
			name = "Transfer";
			break;
		default:
			break;
		}

		auto& cmdPoolandBuffer = m_CommandPoolAndBuffers[type];
		cmdPoolandBuffer.CmdPoolCI.debugName = "GEAR_CORE_CommandPool_RenderGraph_" + name;
		cmdPoolandBuffer.CmdPoolCI.context = m_Context;
		cmdPoolandBuffer.CmdPoolCI.flags = CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
		cmdPoolandBuffer.CmdPoolCI.queueType = type;
		cmdPoolandBuffer.CmdPool = CommandPool::Create(&cmdPoolandBuffer.CmdPoolCI);

		cmdPoolandBuffer.CmdBufferCI.debugName = "GEAR_CORE_CommandBuffer_RenderGraph_" + name;
		cmdPoolandBuffer.CmdBufferCI.commandPool = cmdPoolandBuffer.CmdPool;
		cmdPoolandBuffer.CmdBufferCI.level = CommandBuffer::Level::PRIMARY;
		cmdPoolandBuffer.CmdBufferCI.commandBufferCount = commandBufferCount;
		cmdPoolandBuffer.CmdBuffer = CommandBuffer::Create(&cmdPoolandBuffer.CmdBufferCI);
	}

	m_FrameData.resize(commandBufferCount);
	for (uint32_t i = 0; i < commandBufferCount; i++)
	{
		Reset(i);
	}
	m_FrameIndex = 0;
}

RenderGraph::~RenderGraph()
{
	for(uint32_t i = 0; i < static_cast<uint32_t>(m_FrameData.size()); i++)
		Reset(i);
}

void RenderGraph::Reset(uint32_t frameIndex)
{
	m_FrameIndex = frameIndex;
	m_FrameData[m_FrameIndex].Clear();
}

Ref<Pass> RenderGraph::AddPass(const std::string& passName, const Ref<PassParameters>& passParameters, CommandPool::QueueType queueType, RenderGraphPassFunction renderFunction)
{
	//Check for any resources
	if (passParameters->GetInputResourceViews().empty() && passParameters->GetOutputResourceViews().empty())
		return nullptr;

	//Create and Setup Pass
	Ref<Pass> pass = CreateRef<Pass>(passName, passParameters, queueType, renderFunction);
	pass->GetPassParameters()->Setup();
	FrameData& data = m_FrameData[m_FrameIndex];
	
	//Save current scope into Pass
	pass->m_ScopeStack = data.ScopeStack; 

	//Add Pass ResourceViews to the RenderGraph's 'master' Resource list
	auto AddResourcesToRenderPass = [&](std::vector<ResourceView>& resourceViews) -> void
	{
		for (ResourceView& resourceView : resourceViews)
		{
			const Resource& resource = resourceView.GetResource();
			std::vector<Resource>& previousFrameResources = GetPreviousFrameData().Resources;

			if (!arc::FindInVector(data.Resources, resource))
			{
				data.Resources.push_back(resource);
			}
			if (arc::FindInVector(previousFrameResources, resource)) //Resource is now guaranteed to be in data.Resources
			{
				auto it0 = arc::FindPositionInVector(previousFrameResources, resource);
				auto it1 = arc::FindPositionInVector(data.Resources, resource);
				it1->subresourceMap = it0->subresourceMap;
			}
		}
	};
	AddResourcesToRenderPass(pass->GetInputResourceViews());
	AddResourcesToRenderPass(pass->GetOutputResourceViews());

	//Add pass to the RenderGraph
	data.Passes.push_back(pass);
	pass->m_UnorderedListIndex = data.Passes.size() - 1;
	return pass;
}

void RenderGraph::BeginEventScope(const std::string& scopeName)
{
	FrameData& data = m_FrameData[m_FrameIndex];
	data.ScopeStack.push(scopeName);
}

void RenderGraph::EndEventScope()
{
	FrameData& data = m_FrameData[m_FrameIndex];
	if (!data.ScopeStack.empty())
	{
		data.ScopeStack.pop();
	}
	else
	{
		GEAR_FATAL(ErrorCode::GRAPHICS | ErrorCode::INVALID_STATE, "RenderGraph::FrameData::ScopeStack is empty. Unable to pop element from the stack.")
	}
}

void RenderGraph::Compile()
{
	//Organizing GPU Work with Directed Acyclic Graphs
	//https://levelup.gitconnected.com/organizing-gpu-work-with-directed-acyclic-graphs-f3fd5f2c2af3

	FrameData& data = m_FrameData[m_FrameIndex];
	if (!data.ScopeStack.empty())
	{
		GEAR_FATAL(ErrorCode::GRAPHICS | ErrorCode::INVALID_STATE, "RenderGraph Scope stack in not empty.");
	}

	//Build Adjacency Lists
	{
		data.AdjacencyLists.resize(data.Passes.size());
		for (size_t passIndex = 0; passIndex < data.Passes.size(); passIndex++)
		{
			Ref<Pass>& pass = data.Passes[passIndex];
			std::vector<uint64_t>& adjacentPassIndices = data.AdjacencyLists[passIndex];

			for (size_t otherPassIndex = 0; otherPassIndex < data.Passes.size(); otherPassIndex++)
			{
				if (passIndex >= otherPassIndex)
					continue; // Do not check dependencies on itself

				Ref<Pass>& otherPass = data.Passes[otherPassIndex];

				for (auto& otherPassReadResourceView : otherPass->GetInputResourceViews())
				{
					bool otherPassDependsOnCurrentPass = arc::FindInVector(pass->GetOutputResourceViews(), otherPassReadResourceView);
					bool sameOutputs = pass->GetOutputResourceViews() == otherPass->GetOutputResourceViews();

					bool passIsTask = pass->m_PassParameters->GetType() == PassParameters::Type::TASK;
					bool otherPassIsTask = otherPass->m_PassParameters->GetType() == PassParameters::Type::TASK;

					if (otherPassIsTask && passIsTask)
					{
						if (sameOutputs)
						{
							Ref<TaskPassParameters> tpp = ref_cast<TaskPassParameters>(otherPass->m_PassParameters);
							for (auto& colourAttachment : tpp->m_RenderingInfo.colourAttachments)
								colourAttachment.loadOp = RenderPass::AttachmentLoadOp::LOAD;
							if (tpp->m_RenderingInfo.pDepthAttachment)
								tpp->m_RenderingInfo.pDepthAttachment->loadOp = RenderPass::AttachmentLoadOp::LOAD;
						}
					}

					otherPassDependsOnCurrentPass |= sameOutputs;
					if (otherPassDependsOnCurrentPass)
					{
						adjacentPassIndices.push_back(otherPassIndex);
					}
				}
			}
		}
	}

	//Topological Sort
	{
		std::function<void(size_t, std::vector<bool>&, std::vector<bool>&)> DepthFirstSearch
			= [&](size_t passIndex, std::vector<bool>& visited, std::vector<bool>& onStack)
		{
			visited[passIndex] = true;
			onStack[passIndex] = true;

			size_t adjacencyListIndex = data.Passes[passIndex]->m_UnorderedListIndex;
			for (size_t neighbour : data.AdjacencyLists[adjacencyListIndex])
			{
				if (!visited[neighbour])
				{
					DepthFirstSearch(neighbour, visited, onStack);
				}
			}

			onStack[passIndex] = false;
			data.TopologicallySortedPasses.push_back(data.Passes[passIndex]);
		};

		std::vector<bool> visitedPasses(data.Passes.size(), false);
		std::vector<bool> onStackPasses(data.Passes.size(), false);

		for (size_t passIndex = 0; passIndex < data.Passes.size(); passIndex++)
		{
			if (!visitedPasses[passIndex])
			{
				DepthFirstSearch(passIndex, visitedPasses, onStackPasses);
			}
		}

		std::reverse(data.TopologicallySortedPasses.begin(), data.TopologicallySortedPasses.end());
	}

	//Set Render Pass Begins and Ends
	{
		size_t passCount = data.TopologicallySortedPasses.size();

		for (size_t passIndex = 0; passIndex < passCount; passIndex++)
		{
			size_t nextPassIndex = std::min(passIndex + 1, passCount - 1);
			if (passIndex == nextPassIndex && passCount > 1)
				break;

			Ref<Pass>& pass = data.TopologicallySortedPasses[passIndex];
			Ref<Pass>& nextPass = data.TopologicallySortedPasses[nextPassIndex];

			bool sameOutputs = pass->GetOutputResourceViews() == nextPass->GetOutputResourceViews();

			bool passIsTask = pass->m_PassParameters->GetType() == PassParameters::Type::TASK;
			bool nextPassIsTask = nextPass->m_PassParameters->GetType() == PassParameters::Type::TASK;

			bool passIsGraphics = passIsTask ? ref_cast<TaskPassParameters>(pass->m_PassParameters)->IsGraphics() : false;
			bool nextPassIsGraphics = nextPassIsTask ? ref_cast<TaskPassParameters>(nextPass->m_PassParameters)->IsGraphics() : false;

			//Deal with single pass graph
			if (passCount == 1)
			{
				if (passIsGraphics)
				{
					pass->m_BeginRendering = true;
					pass->m_EndRendering = true;
				}
				break;
			}

			//Terminals
			if (passIndex == 0 && passIsGraphics)
			{
				pass->m_BeginRendering = true;
			}
			if (nextPassIndex == passCount - 1 && nextPassIsGraphics)
			{
				nextPass->m_EndRendering = true;
			}

			//To and From Compute or Transfer
			if (!passIsGraphics && nextPassIsGraphics)
			{
				nextPass->m_BeginRendering = true;
			}
			if (passIsGraphics && !nextPassIsGraphics)
			{
				pass->m_EndRendering = true;
			}

			//Graphics with differnt outputs
			if (passIsGraphics && nextPassIsGraphics && !sameOutputs)
			{
				pass->m_EndRendering = true;
				nextPass->m_BeginRendering = true;
			}

			//Default is to preserve rendering between graphics with the same outputs.
		}

		//Check Input Resource Views
		Ref<Pass> startPass = nullptr;
		for (size_t passIndex = 0; passIndex < passCount; passIndex++)
		{
			Ref<Pass>& pass = data.TopologicallySortedPasses[passIndex];

			if (pass->m_BeginRendering && !pass->m_EndRendering)
				startPass = pass;

			if (startPass && startPass != pass)
			{
				auto& passInputs = pass->GetInputResourceViews();
				auto& startPassInputs = startPass->GetInputResourceViews();
				startPassInputs.insert(startPassInputs.end(), passInputs.begin(), passInputs.end());
				passInputs.clear();
			}

			if (pass->m_EndRendering && !pass->m_BeginRendering)
				startPass = nullptr;
		}
	}

	//Build Dependency Levels
	{
		std::vector<size_t> longestDistances(data.TopologicallySortedPasses.size(), 0);
		size_t dependencyLevelCount = 1;

		for (size_t passIndex = 0; passIndex < data.Passes.size(); passIndex++)
		{
			size_t originalIndex = data.TopologicallySortedPasses[passIndex]->m_UnorderedListIndex;
			size_t adjacencyListIndex = originalIndex;

			for (size_t adjacentPassIndex : data.AdjacencyLists[adjacencyListIndex])
			{
				if (longestDistances[adjacentPassIndex] < longestDistances[originalIndex] + 1)
				{
					size_t newLongestDistance = longestDistances[originalIndex] + 1;
					longestDistances[adjacentPassIndex] = newLongestDistance;
					dependencyLevelCount = std::max(size_t(newLongestDistance + 1), dependencyLevelCount);
				}
			}
		}

		data.DependencyLevels.resize(dependencyLevelCount);
		for (size_t passIndex = 0; passIndex < data.Passes.size(); passIndex++)
		{
			Ref<Pass>& pass = data.Passes[passIndex];
			uint64_t levelIndex = longestDistances[passIndex];
			DependencyLevel& dependencyLevel = data.DependencyLevels[levelIndex];
			dependencyLevel.Passes.push_back(pass);
			dependencyLevel.LevelIndex = levelIndex;
			pass->m_DependencyLevelIndex = levelIndex;
		}
	}
}

void RenderGraph::Execute()
{
	Compile();
	FrameData& data = m_FrameData[m_FrameIndex];

	CommandBufferRef& cmdBuffer = GetCommandBuffer(CommandPool::QueueType::GRAPHICS/*pass->m_QueueType*/);
	cmdBuffer->Reset(m_FrameIndex, false);
	cmdBuffer->Begin(m_FrameIndex, CommandBuffer::UsageBit::SIMULTANEOUS);

	for (const auto& pass : m_FrameData[m_FrameIndex].TopologicallySortedPasses)
	{
		if (data.ScopeStack != pass->m_ScopeStack)
		{
			core::ResolveStacks<std::string>(data.ScopeStack, pass->m_ScopeStack,
				[&]() { cmdBuffer->EndDebugLabel(m_FrameIndex); },
				[&](const std::string& scopeName) { cmdBuffer->BeginDebugLabel(m_FrameIndex, scopeName); });
		}

		pass->Execute(this, cmdBuffer, m_FrameIndex);
	}

	core::ClearStack(data.ScopeStack, [&]() { cmdBuffer->EndDebugLabel(m_FrameIndex); });

	cmdBuffer->End(m_FrameIndex);
}

ImageRef RenderGraph::CreateImage(const ImageCreateInfo& imageCreateInfo, const std::string& name)
{
	Image::UsageBit usage = Image::UsageBit(0);
	if (arc::BitwiseCheck(imageCreateInfo.usage, ImageCreateInfo::UsageBit::COLOUR_ATTACHMENT))
		usage |= Image::UsageBit::COLOUR_ATTACHMENT_BIT;
	if (arc::BitwiseCheck(imageCreateInfo.usage, ImageCreateInfo::UsageBit::DEPTH_STENCIL_ATTACHMENT))
		usage |= Image::UsageBit::DEPTH_STENCIL_ATTACHMENT_BIT;
	if (arc::BitwiseCheck(imageCreateInfo.usage, ImageCreateInfo::UsageBit::SHADER_READ_ONLY))
		usage |= Image::UsageBit::SAMPLED_BIT;
	if (arc::BitwiseCheck(imageCreateInfo.usage, ImageCreateInfo::UsageBit::SHADER_READ_WRITE))
		usage |= Image::UsageBit::STORAGE_BIT;

	Image::CreateInfo imageCI;
	imageCI.debugName = "GEAR_CORE_Image_RenderGraph_" + name;
	imageCI.device = m_Context->GetDevice();
	imageCI.type = imageCreateInfo.type;
	imageCI.format = imageCreateInfo.format;
	imageCI.width = imageCreateInfo.width;
	imageCI.height = imageCreateInfo.height;
	imageCI.depth = imageCreateInfo.depth;
	imageCI.mipLevels = imageCreateInfo.mipLevels;
	imageCI.arrayLayers = imageCreateInfo.arrayLayers;
	imageCI.sampleCount = imageCreateInfo.sampleCount;
	imageCI.usage = usage;
	imageCI.layout = Image::Layout::UNKNOWN;
	imageCI.size = 0;
	imageCI.data = nullptr;
	imageCI.allocator = AllocatorManager::GetGPUAllocator();
	imageCI.externalImage = nullptr;

	return Image::Create(&imageCI);
}

ImageViewRef RenderGraph::CreateImageView(const ImageRef& image, Image::Type type, const Image::SubresourceRange& subresourceRange)
{
	ImageView::CreateInfo imageViewCI;
	imageViewCI.debugName = "GEAR_CORE_ImageView_RenderGraph: " + image->GetCreateInfo().debugName;
	imageViewCI.device = m_Context->GetDevice();
	imageViewCI.image = image;
	imageViewCI.viewType = type;
	imageViewCI.subresourceRange = subresourceRange;

	return ImageView::Create(&imageViewCI);
}

bool RenderGraph::ResourceIsPresent(const Resource& passResource)
{
	FrameData& data = m_FrameData[m_FrameIndex];
	return arc::FindInVector(data.Resources, passResource);
}

bool RenderGraph::ResourceIsPresent(const Resource& passResource) const
{
	const FrameData& data = m_FrameData[m_FrameIndex];
	return arc::FindInVector(data.Resources, passResource);
}

Resource& RenderGraph::GetTrackedResource(const Resource& passResource)
{
	FrameData& data = m_FrameData[m_FrameIndex];
	GEAR_FATAL(!arc::FindInVector(data.Resources, passResource), "Pass's Resource not found in RenderGraph's Resource");

	auto it = arc::FindPositionInVector(data.Resources, passResource);
	return *it;
}

const Resource& RenderGraph::GetTrackedResource(const Resource& passResource) const
{
	const FrameData& data = m_FrameData[m_FrameIndex];
	GEAR_FATAL(!arc::FindInVector(data.Resources, passResource), "Pass's Resource not found in RenderGraph's Resource");

	auto it = arc::FindPositionInVectorConst(data.Resources, passResource);
	return *it;
}

CommandBufferRef& RenderGraph::GetCommandBuffer(CommandPool::QueueType queueType)
{
	return m_CommandPoolAndBuffers[queueType].CmdBuffer;
}

RenderGraph::FrameData& RenderGraph::GetPreviousFrameData()
{
	uint32_t previousFrameIndex = 0;

	int32_t previousFrameIndexTemp = static_cast<int32_t>(m_FrameIndex - 1);
	if (previousFrameIndexTemp < 0)
		previousFrameIndex = static_cast<uint32_t>(m_FrameData.size() - 1);
	else
		previousFrameIndex = static_cast<uint32_t>(previousFrameIndexTemp);

	return m_FrameData[previousFrameIndex];
}