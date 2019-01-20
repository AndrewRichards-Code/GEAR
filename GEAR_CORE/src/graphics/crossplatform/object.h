#pragma once

#include "GL/glew.h"
#include "../opengl/buffer/vertexarray.h"
#include "../opengl/buffer/indexbuffer.h"
#include "../opengl/shader/shader.h"
#include "../opengl/texture.h"
#include "../../utils/fileutils.h"
#include "../../maths/ARMLib.h"

namespace GEAR {
namespace GRAPHICS {
namespace CROSSPLATFORM {

#ifdef GEAR_OPENGL

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

	std::shared_ptr<OPENGL::VertexArray> m_VAO;
	std::shared_ptr<OPENGL::IndexBuffer> m_IBO;
	OPENGL::Shader& m_Shader;
	const OPENGL::Texture& m_Texture = OPENGL::Texture("res/gear_core/GEAR_logo_square.png");
	ARM::Vec4 m_Colour{ 0.0f, 0.0f, 0.0f, 0.0f };

	static bool s_InitialiseUBO;
	struct ModelUBO
	{
		ARM::Mat4 m_ModlMatrix;
	} m_ModelUBO;

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
	void InitialiseUBO();
	void LoadObjDataIntoObject();
	void GenVAOandIBO();

public:
	Object(const char* objFilePath, OPENGL::Shader& shader, const OPENGL::Texture& texture, const ARM::Mat4& modl);
	Object(const char* objFilePath, OPENGL::Shader& shader, const ARM::Vec4& colour, const ARM::Mat4& modl);
	Object(const char* objFilePath, OPENGL::Shader& shader, const OPENGL::Texture& texture, const ARM::Vec3& position, const ARM::Vec2& size); //Doesn't fill the VAO or IBO.
	~Object();

	void BindTexture(int slot = 0) const;
	void UnbindTexture() const;
	static void SetTextureArray(const OPENGL::Shader& shader);
	static void UnsetTextureArray(const OPENGL::Shader& shader);
	void BindCubeMap(int slot) const;
	void UnbindCubeMap() const;

	void SetUniformModlMatrix();
	void SetUniformModlMatrix(const ARM::Mat4& modl);


	inline const std::shared_ptr<OPENGL::VertexArray> GetVAO() const { return m_VAO; }
	inline const std::shared_ptr<OPENGL::IndexBuffer> GetIBO() const { return m_IBO; }
	inline const OPENGL::Shader& GetShader() const { return m_Shader; }
	inline const OPENGL::Texture& GetTexture() const { return m_Texture; }
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
}
#endif
