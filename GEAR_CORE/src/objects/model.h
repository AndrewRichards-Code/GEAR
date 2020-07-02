#pragma once

#include "gear_core_common.h"
#include "graphics/miru/buffer/vertexbuffer.h"
#include "graphics/miru/buffer/indexbuffer.h"
#include "graphics/miru/buffer/uniformbuffer.h"
#include "graphics/miru/pipeline.h"
#include "graphics/miru/texture.h"
#include "utils/fileutils.h"
#include "mars.h"

namespace gear {
namespace objects {

class Model
{
private:
	const char* m_ObjFilePath;
	FileUtils::ObjData m_ObjData;

	std::vector<float> m_Vertices;
	std::vector<float> m_TextCoords;
	std::vector<float> m_TextID;
	std::vector<float> m_Normals;
	std::vector<float> m_Colours;
	std::vector<unsigned int> m_Indices;

	void* m_Device;

	std::vector<std::shared_ptr<graphics::VertexBuffer>> m_VBOs;
	std::shared_ptr<graphics::IndexBuffer> m_IBO;
	std::shared_ptr<graphics::Pipeline> m_Pipeline;
	std::shared_ptr<graphics::Texture> m_Texture;
	mars::Vec4 m_Colour{ 0.0f, 0.0f, 0.0f, 0.0f };

	struct ModelUBO
	{
		mars::Mat4 m_ModlMatrix;
	} m_ModelUBO;
	std::shared_ptr<graphics::UniformBuffer> m_UBO;

	mars::Mat4 m_ModlMatrix;
	mars::Vec3 m_Position;
	mars::Vec2 m_Size;

public:
	bool b_ForBatchRenderer2D = false;
	struct VertexData
	{
		mars::Vec4 m_Vertex;
		mars::Vec2 m_TexCoord;
		float m_TexId;
		mars::Vec4 m_Normal;
		mars::Vec4 m_Colour;
	};

private:
	void InitialiseUBO();
	void LoadObjDataIntoObject();
	void GenVBOandIBO();

public:
	Model(void* device, const char* objFilePath, std::shared_ptr<graphics::Pipeline> pipeline, std::shared_ptr<graphics::Texture> texture, const mars::Mat4& modl);
	Model(void* device, const char* objFilePath, std::shared_ptr<graphics::Pipeline> pipeline, const mars::Vec4& colour, const mars::Mat4& modl);
	Model(void* device, const char* objFilePath, std::shared_ptr<graphics::Pipeline> pipeline, std::shared_ptr<graphics::Texture> texture, const mars::Vec4& colour, const mars::Vec3& position, const mars::Vec2& size); //Doesn't fill the VAO or IBO.
	~Model();

	void SetUniformModlMatrix();
	void SetUniformModlMatrix(const mars::Mat4& modl);

	inline static void SetContext(miru::Ref<miru::crossplatform::Context> context)
	{
		graphics::VertexBuffer::SetContext(context);
		graphics::IndexBuffer::SetContext(context);
		graphics::UniformBuffer::SetContext(context);
		graphics::Texture::SetContext(context);
	};

	inline const std::vector<std::shared_ptr<graphics::VertexBuffer>> GetVBOs() const { return m_VBOs; }
	inline const std::shared_ptr<graphics::IndexBuffer> GetIBO() const { return m_IBO; }
	inline const std::shared_ptr<graphics::Pipeline> GetPipeline() const { return m_Pipeline; }
	inline const std::shared_ptr<graphics::Texture> GetTexture() const { return m_Texture; }
	inline const std::shared_ptr<graphics::UniformBuffer> GetUBO() const { return m_UBO; }
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
