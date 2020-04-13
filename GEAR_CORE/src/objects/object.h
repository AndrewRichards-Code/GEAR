#pragma once

#include "gear_core_common.h"
#include "graphics/miru/buffer/vertexbuffer.h"
#include "graphics/miru/buffer/indexbuffer.h"
#include "graphics/miru/buffer/uniformbuffer.h"
#include "graphics/miru/pipeline.h"
#include "graphics/miru/texture.h"
#include "utils/fileutils.h"
#include "mars.h"

namespace GEAR {
namespace OBJECTS {

class Object
{
private:
	const char* m_ObjFilePath;
	FileUtils::ObjData m_ObjData;

	std::vector<float> m_Vertices;
	std::vector<float> m_TextCoords;
	std::vector<float> m_Normals;
	std::vector<unsigned int> m_Indices;
	std::vector<float> m_Colours;

	void* m_Device;

	std::vector<std::shared_ptr<GRAPHICS::VertexBuffer>> m_VBOs;
	std::shared_ptr<GRAPHICS::IndexBuffer> m_IBO;
	std::shared_ptr<GRAPHICS::Pipeline> m_Pipeline;
	std::shared_ptr<GRAPHICS::Texture> m_Texture;
	mars::Vec4 m_Colour{ 0.0f, 0.0f, 0.0f, 0.0f };

	static bool s_InitialiseUBO;
	struct ModelUBO
	{
		mars::Mat4 m_ModlMatrix;
	} m_ModelUBO;
	std::shared_ptr<GRAPHICS::UniformBuffer> m_UBO;

	mars::Mat4 m_ModlMatrix;
	mars::Vec3 m_Position;
	mars::Vec2 m_Size;

public:
	bool b_ForBatchRenderer2D = false;
	struct VertexData
	{
		mars::Vec3 m_Vertex;
		mars::Vec2 m_TexCoord;
		float m_TexId;
		mars::Vec3 m_Normal;
		mars::Vec4 m_Colour;
	};

private:
	void InitialiseUBO();
	void LoadObjDataIntoObject();
	void GenVBOandIBO();

public:
	Object(void* device, const char* objFilePath, std::shared_ptr<GRAPHICS::Pipeline> pipeline, std::shared_ptr<GRAPHICS::Texture> texture, const mars::Mat4& modl);
	Object(void* device, const char* objFilePath, std::shared_ptr<GRAPHICS::Pipeline> pipeline, const mars::Vec4& colour, const mars::Mat4& modl);
	Object(void* device, const char* objFilePath, std::shared_ptr<GRAPHICS::Pipeline> pipeline, std::shared_ptr<GRAPHICS::Texture> texture, const mars::Vec4& colour, const mars::Vec3& position, const mars::Vec2& size); //Doesn't fill the VAO or IBO.
	~Object();

	void SetUniformModlMatrix();
	void SetUniformModlMatrix(const mars::Mat4& modl);

	inline const std::vector<std::shared_ptr<GRAPHICS::VertexBuffer>> GetVBOs() const { return m_VBOs; }
	inline const std::shared_ptr<GRAPHICS::IndexBuffer> GetIBO() const { return m_IBO; }
	inline const std::shared_ptr<GRAPHICS::Pipeline> GetPipeline() const { return m_Pipeline; }
	inline const std::shared_ptr<GRAPHICS::Texture> GetTexture() const { return m_Texture; }
	inline const std::shared_ptr<GRAPHICS::UniformBuffer> GetUBO() const { return m_UBO; }
	inline const mars::Mat4 GetModlMatrix() const { return m_ModlMatrix; }

	inline const std::vector<float>& GetVertices() const { return m_Vertices; }
	inline const std::vector<float>& GetTextCoords() const { return m_TextCoords; }
	inline const std::vector<float>& GetNormals() const { return m_Normals; }
	inline const std::vector<float>& GetColours() const { return m_Colours; }
	inline const std::vector<unsigned int>& GetIndices() const { return m_Indices; }
	inline const int GetNumOfUniqueVertices() { return m_ObjData.GetSizeUniqueVertices(); }
	inline const char* GetObjFileName() const { return m_ObjFilePath; }
};
}
}
