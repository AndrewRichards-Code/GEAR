#pragma once

#include "GL/glew.h"
#include "renderer.h"
#include "../../../maths/ARMLib.h"
#include "../buffer/vertexarray.h"
#include "../buffer/indexbuffer.h"
#include "../shader.h"

#define GEAR_RENDERER_MAX_OBJ           10000
#define GEAR_RENDERER_VERTEX_SIZE	    sizeof(Object::VertexData)
#define GEAR_RENDERER_OBJ_SIZE          GEAR_RENDERER_VERTEX_SIZE * 4
#define GEAR_RENDERER_BUFFER_SIZE       GEAR_RENDERER_OBJ_SIZE * GEAR_RENDERER_MAX_OBJ
#define GEAR_RENDERER_INDICIES_SIZE     GEAR_RENDERER_MAX_OBJ * 6
#define GEAR_RENDERER_MAX_TEXTURE_SLOTS 32

namespace GEAR {
namespace GRAPHICS {
namespace OPENGL {
class BatchRenderer2D : public Renderer
{
private:
	GLuint m_VAO;
	GLuint m_VBO;
	std::unique_ptr<IndexBuffer> m_IBO;
	CROSSPLATFORM::Object* m_Object;
	GLsizei m_IndexCount;

	CROSSPLATFORM::Object::VertexData* m_BatchBuffer;
	std::vector<unsigned int> m_TextureSlots;

public:
	void OpenMapBuffer();
	void CloseMapBuffer();
	void Submit(CROSSPLATFORM::Object* obj) override;
	void Flush() override;
	void CopyLights(const std::vector<CROSSPLATFORM::Light*> lights) { std::copy(lights.begin(), lights.end(), std::back_inserter(m_Lights)); }

	BatchRenderer2D();
	~BatchRenderer2D();

	void Init();
};
}
}
}