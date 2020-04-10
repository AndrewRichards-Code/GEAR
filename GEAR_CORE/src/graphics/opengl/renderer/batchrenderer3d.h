#pragma once

#include "gear_common.h"
#include "mars.h"
#include "renderer.h"

#define GEAR_BATCH_RENDERER_3D_VERTEX_SIZE	     sizeof(Object::VertexData)
#define GEAR_BATCH_RENDERER_3D_UNIQUE_VERTICES	 1000000
#define GEAR_BATCH_RENDERER_3D_BUFFER_SIZE		 GEAR_BATCH_RENDERER_3D_VERTEX_SIZE	* GEAR_BATCH_RENDERER_3D_UNIQUE_VERTICES
#define GEAR_BATCH_RENDERER_3D_MAX_TEXTURE_SLOTS 32

namespace GEAR {
namespace GRAPHICS {
namespace OPENGL {
class BatchRenderer3D : public Renderer
{
private:
	GLuint m_VAO;
	GLuint m_VBO;

	std::vector<GLsizei> m_Counts;
	std::vector<unsigned int> m_Indicies;
	std::vector<GLvoid*> m_GLIndicies;

	std::vector<unsigned int> m_IBOIndicies;
	std::unique_ptr<IndexBuffer> m_IBO;
	unsigned int m_PreviousNumOfAccumulatedVertices = 0;

	CROSSPLATFORM::Object::VertexData* m_BatchBuffer;
	std::vector<unsigned int> m_TextureSlots;
	CROSSPLATFORM::Object* m_Object;


public:
	void OpenMapBuffer();
	void CloseMapBuffer();
	void Submit(CROSSPLATFORM::Object* obj) override;
	void Flush() override;
	void Flush(const OPENGL::Shader* overrideShader);

	BatchRenderer3D();
	~BatchRenderer3D();

};
}
}
}