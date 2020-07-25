#pragma once

#include "gear_core_common.h"
#include "Graphics/Vertexbuffer.h"
#include "Graphics/Indexbuffer.h"
#include "Utils/FileUtils.h"

namespace gear {
namespace objects {

	class Mesh
	{
	public:
		struct CreateInfo
		{
			const char* debugName;
			void*		device;
			std::string filepath;
		};

		enum class VertexBufferContents : size_t
		{
			POSITION,
			TEXTURE_COORD,
			TEXTURE_ID,
			NORMAL,
			COLOUR,
			BI_NORMAL,
			TANGENT,
		};

	private:
		std::string m_DebugName;

		CreateInfo m_CI;
		file_utils::ObjData m_Data;

		std::map<VertexBufferContents, gear::Ref<graphics::Vertexbuffer>> m_VBs;
		gear::Ref<graphics::Indexbuffer> m_IB;

	public:
		Mesh(CreateInfo* pCreateInfo);
		~Mesh();

		void AddVertexBuffer(VertexBufferContents type, gear::Ref<graphics::Vertexbuffer> vertexBuffer);
		void RemoveVertexBuffer(VertexBufferContents type);

		inline const std::map<VertexBufferContents, gear::Ref<graphics::Vertexbuffer>>& GetVertexBuffers() const { return m_VBs; }
		inline const gear::Ref<graphics::Indexbuffer> GetIndexBuffer() const { return m_IB; }
		inline const file_utils::ObjData& GetObjData() const { return m_Data; }
	};
}
}
