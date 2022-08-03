#include "gear_core_common.h"
#include "Graphics/Rendering/RenderGraph.h"

#include "Graphics/AllocatorManager.h"

using namespace gear;
using namespace graphics;
using namespace rendering;

using namespace miru;
using namespace base;

///////////////
//RenderGraph//
///////////////

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

		auto& cmd = m_CommandPoolAndBuffers[type];
		cmd.cmdPoolCI.debugName = "GEAR_CORE_CommandPool_RenderGraph_" + name;
		cmd.cmdPoolCI.context = m_Context;
		cmd.cmdPoolCI.flags = CommandPool::FlagBit::RESET_COMMAND_BUFFER_BIT;
		cmd.cmdPoolCI.queueType = type;
		cmd.cmdPool = CommandPool::Create(&cmd.cmdPoolCI);

		cmd.cmdBufferCI.debugName = "GEAR_CORE_CommandBuffer_RenderGraph_" + name;
		cmd.cmdBufferCI.commandPool = cmd.cmdPool;
		cmd.cmdBufferCI.level = CommandBuffer::Level::PRIMARY;
		cmd.cmdBufferCI.commandBufferCount = commandBufferCount;
		cmd.cmdBuffer = CommandBuffer::Create(&cmd.cmdBufferCI);
	}

}

RenderGraph::~RenderGraph()
{
	Reset();
}

Ref<Pass> RenderGraph::AddPass(const std::string& passName, const Ref<PassParameters>& passParameters, CommandPool::QueueType queueType, RenderGraphPassFunction renderFunction)
{
	//Check for any resources
	if (passParameters->GetInputResources().empty() && passParameters->GetOutputResources().empty())
		return nullptr;

	//Create and Setup Pass
	Ref<Pass> pass = CreateRef<Pass>(passName, passParameters, queueType, renderFunction);
	pass->GetPassParameters()->Setup();
	
	//Search and Resolve any multiple write dependencies
	for (auto& previousPass : m_Passes)
	{
		if (previousPass->GetOutputResources() == pass->GetOutputResources())
		{
			pass->m_BackwardGraphicsDependentPasses.push_back(previousPass);
			previousPass->m_ForwardGraphicsDependentPasses.push_back(pass);
		}
	}

	//Add Pass Resources to the RenderGraph 'master' list
	for (Resource& resource : pass->GetInputResources())
	{
		if (!arc::FindInVector(m_Resources, resource))
		{
			m_Resources.push_back(resource);
		}
		if (arc::FindInVector(m_PreviousFrameResources, resource))
		{
			auto& it = arc::FindPositionInVector(m_PreviousFrameResources, resource);
			resource.oldState = it->newState;
		}
	}
	for (Resource& resource : pass->GetOutputResources())
	{
		if (!arc::FindInVector(m_Resources, resource))
			m_Resources.push_back(resource);
	}

	//Add pass to the RenderGraph
	m_Passes.push_back(pass);
	pass->m_UnorderedListIndex = m_Passes.size() - 1;
	return pass;
}

void RenderGraph::Compile()
{
	//Organizing GPU Work with Directed Acyclic Graphs
	//https://levelup.gitconnected.com/organizing-gpu-work-with-directed-acyclic-graphs-f3fd5f2c2af3

	//Build Adjacency Lists
	{
		m_AdjacencyLists.resize(m_Passes.size());
		for (size_t passIndex = 0; passIndex < m_Passes.size(); passIndex++)
		{
			Ref<Pass>& pass = m_Passes[passIndex];
			std::vector<uint64_t>& adjacentPassIndices = m_AdjacencyLists[passIndex];

			for (size_t otherPassIndex = 0; otherPassIndex < m_Passes.size(); otherPassIndex++)
			{
				if (passIndex == otherPassIndex)
					continue; // Do not check dependencies on itself

				Ref<Pass>& otherPass = m_Passes[otherPassIndex];

				for (auto& otherPassReadResource : otherPass->GetInputResources())
				{
					bool otherPassDependsOnCurrentPass = arc::FindInVector(pass->GetOutputResources(), otherPassReadResource);
					otherPassDependsOnCurrentPass |= pass->GetOutputResources() == otherPass->GetOutputResources();
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

			size_t adjacencyListIndex = m_Passes[passIndex]->m_UnorderedListIndex;
			for (size_t neighbour : m_AdjacencyLists[adjacencyListIndex])
			{
				if (!visited[neighbour])
				{
					DepthFirstSearch(neighbour, visited, onStack);
				}
			}

			onStack[passIndex] = false;
			m_TopologicallySortedPasses.push_back(m_Passes[passIndex]);
		};

		std::vector<bool> visitedPasses(m_Passes.size(), false);
		std::vector<bool> onStackPasses(m_Passes.size(), false);

		for (size_t passIndex = 0; passIndex < m_Passes.size(); passIndex++)
		{
			if (!visitedPasses[passIndex])
			{
				DepthFirstSearch(passIndex, visitedPasses, onStackPasses);
			}
		}

		std::reverse(m_TopologicallySortedPasses.begin(), m_TopologicallySortedPasses.end());
	}

	//Build Dependency Levels
	{
		std::vector<size_t> longestDistances(m_TopologicallySortedPasses.size(), 0);
		size_t dependencyLevelCount = 1;

		for (size_t passIndex = 0; passIndex < m_Passes.size(); passIndex++)
		{
			size_t originalIndex = m_TopologicallySortedPasses[passIndex]->m_UnorderedListIndex;
			size_t adjacencyListIndex = originalIndex;

			for (size_t adjacentPassIndex : m_AdjacencyLists[adjacencyListIndex])
			{
				if (longestDistances[adjacentPassIndex] < longestDistances[originalIndex] + 1)
				{
					size_t newLongestDistance = longestDistances[originalIndex] + 1;
					longestDistances[adjacentPassIndex] = newLongestDistance;
					dependencyLevelCount = std::max(size_t(newLongestDistance + 1), dependencyLevelCount);
				}
			}
		}

		m_DependencyLevels.resize(dependencyLevelCount);
		for (size_t passIndex = 0; passIndex < m_Passes.size(); passIndex++)
		{
			Ref<Pass>& pass = m_Passes[passIndex];
			uint64_t levelIndex = longestDistances[passIndex];
			DependencyLevel& dependencyLevel = m_DependencyLevels[levelIndex];
			dependencyLevel.m_Passes.push_back(pass);
			dependencyLevel.m_LevelIndex = levelIndex;
			pass->m_DependencyLevelIndex = levelIndex;
		}
	}
}

void RenderGraph::Execute(uint32_t frameIndex)
{
	Compile();

	CommandBufferRef& cmdBuffer = GetCommandBuffer(CommandPool::QueueType::GRAPHICS/*pass->m_QueueType*/);
	cmdBuffer->Reset(frameIndex, false);
	cmdBuffer->Begin(frameIndex, CommandBuffer::UsageBit::SIMULTANEOUS);
	for (const auto& pass : m_TopologicallySortedPasses)
	{
		pass->Execute(this, cmdBuffer, frameIndex);
	}
	cmdBuffer->End(frameIndex);

	//Store resource for the next frame to reference
	m_PreviousFrameResources.clear();
	m_PreviousFrameResources = m_Resources;
}

void RenderGraph::Reset()
{
	m_DependencyLevels.clear();
	m_AdjacencyLists.clear();
	m_TopologicallySortedPasses.clear();
	m_Resources.clear();
	m_Passes.clear();
}

ImageRef RenderGraph::CreateImage(const ImageDesc& desc, const std::string& name)
{
	Image::UsageBit usage = Image::UsageBit(0);
	if (arc::BitwiseCheck(desc.usage, ImageDesc::UsageBit::COLOUR_ATTACHMENT))
		usage |= Image::UsageBit::COLOUR_ATTACHMENT_BIT;
	if (arc::BitwiseCheck(desc.usage, ImageDesc::UsageBit::DEPTH_STENCIL_ATTACHMENT))
		usage |= Image::UsageBit::DEPTH_STENCIL_ATTACHMENT_BIT;
	if (arc::BitwiseCheck(desc.usage, ImageDesc::UsageBit::SHADER_READ_ONLY))
		usage |= Image::UsageBit::SAMPLED_BIT;
	if (arc::BitwiseCheck(desc.usage, ImageDesc::UsageBit::SHADER_READ_WRITE))
		usage |= Image::UsageBit::STORAGE_BIT;

	Image::CreateInfo imageCI;
	imageCI.debugName = "GEAR_CORE_Image_RenderGraph_" + name;
	imageCI.device = m_Context->GetDevice();
	imageCI.type = desc.type;
	imageCI.format = desc.format;
	imageCI.width = desc.width;
	imageCI.height = desc.height;
	imageCI.depth = desc.depth;
	imageCI.mipLevels = desc.mipLevels;
	imageCI.arrayLayers = desc.arrayLayers;
	imageCI.sampleCount = desc.sampleCount;
	imageCI.usage = usage;
	imageCI.layout = Image::Layout::UNKNOWN;
	imageCI.size = 0;
	imageCI.data = nullptr;
	imageCI.allocator = AllocatorManager::GetGPUAllocator();

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

Resource& RenderGraph::GetTrackedResource(const Resource& passResource)
{
	GEAR_ASSERT(!arc::FindInVector(m_Resources, passResource), "Pass's Resource not found in RenderGraph's Resource");

	auto it = arc::FindPositionInVector(m_Resources, passResource);
	return *it;
}

const Resource& RenderGraph::GetTrackedResource(const Resource& passResource) const
{
	GEAR_ASSERT(!arc::FindInVector(m_Resources, passResource), "Pass's Resource not found in RenderGraph's Resource");

	auto it = arc::FindPositionInVectorConst(m_Resources, passResource);
	return *it;
}

CommandBufferRef& RenderGraph::GetCommandBuffer(CommandPool::QueueType queueType)
{
	return m_CommandPoolAndBuffers[queueType].cmdBuffer;
}