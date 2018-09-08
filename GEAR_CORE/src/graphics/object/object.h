#pragma once

#include "GL/glew.h"
#include "../buffer/vertexarray.h"
#include "../buffer/indexbuffer.h"
#include "../shader.h"
#include "../texture.h"
#include "../../utils/fileutils.h"
#include "../../maths/ARMLib.h"

namespace GEAR {
namespace GRAPHICS {

class Object
{
private:
	const char* m_ObjFilePath;
	FileUtils::ObjData m_ObjData;

	std::vector<float> m_Vertices;
	std::vector<float> m_TextCoords;
	std::vector<float> m_Normals;
	std::vector<unsigned int> m_Indices;

	VertexArray* m_VAO;
	IndexBuffer* m_IBO;
	Shader& m_Shader;
	const Texture& m_Texture;

	ARM::Mat4 m_ModlMatrix;

	ARM::Vec3 m_Position;
	ARM::Vec2 m_Size;

public:
	bool forBatchRenderer2D = false;
	struct VertexData
	{
		ARM::Vec3 m_Vertex;
		ARM::Vec2 m_TextCoord;
		float m_TextId;
		ARM::Vec3 m_Normal;
	};

private:
	void LoadObjDataIntoObject();
	void GenVAOandIBO();

public:
	Object(const char* objFilePath, Shader& shader, const Texture& texture, const ARM::Mat4& modl);
	Object(const char* objFilePath, Shader& shader, const Texture& texture, const ARM::Vec3& position, const ARM::Vec2& size); //Doesn't fill the VAO or IBO.
	~Object();

	void BindTexture(int slot = 0) const;
	void UnbindTexture() const;
	static void SetTextureArray(Shader& shader);
	static void UnsetTextureArray(Shader& shader);
			
	void SetUniformModlMatrix() const;
	void SetUniformModlMatrix(const ARM::Mat4& modl);
		
	inline const VertexArray* GetVAO() const { return m_VAO; }
	inline const IndexBuffer* GetIBO() const { return m_IBO; }
	inline Shader& GetShader() const { return m_Shader; }
	inline const Texture& GetTexture() const { return m_Texture; }
	inline const unsigned int GetTextureID() const { return m_Texture.GetTextureID(); }
	inline const ARM::Mat4 GetModlMatrix() const { return m_ModlMatrix; }

	inline const std::vector<float>& GetVertices() const { return m_Vertices; }
	inline const std::vector<float>& GetTextCoords() const { return m_TextCoords; }
	inline const std::vector<float>& GetNormals() const { return m_Normals; }
	inline const std::vector<unsigned int>& GetIndices() const { return m_Indices; }
	inline const char* GetObjFileName() const { return m_ObjFilePath; }
};
}
}
