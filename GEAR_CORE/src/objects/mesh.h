#pragma once

#include "gear_core_common.h"
#include "graphics/miru/buffer/vertexbuffer.h"
#include "graphics/miru/buffer/indexbuffer.h"
#include "utils/fileutils.h"

namespace GEAR {
namespace OBJECTS {

	class Mesh
	{
		enum class VertexBufferContents : uint32_t
		{
			POSITION,
			TEXTURE_COORD,
			TEXTURE_ID,
			NORMAL,
			BI_NORMAL,
			TANGENT,
			COLOUR
		};

	private:
		void* m_Device;

		const char* m_Filepath;
		FileUtils::ObjData m_Data;

		std::map<VertexBufferContents, std::shared_ptr<GRAPHICS::VertexBuffer>> m_VBs;
		std::shared_ptr<GRAPHICS::IndexBuffer> m_IB;

	public:
		Mesh(void* device, const char* filepath);
		~Mesh();

		inline const std::map<VertexBufferContents, std::shared_ptr<GRAPHICS::VertexBuffer>>&
			GetVertexBuffers() const { return m_VBs; }

		inline const std::shared_ptr<GRAPHICS::IndexBuffer>
			GetIndexBuffer() const { return m_IB; }

		inline const FileUtils::ObjData& GetObjData() const { return m_Data; }
	};
}
}
