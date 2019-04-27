#pragma once

#include "gear_common.h"
#include "maths/ARMLib.h"
#include "renderer.h"

#define GEAR_BATCH_RENDERER_2D_MAX_OBJ           10000
#define GEAR_BATCH_RENDERER_2D_VERTEX_SIZE	     sizeof(Object::VertexData)
#define GEAR_BATCH_RENDERER_2D_OBJ_SIZE          GEAR_BATCH_RENDERER_2D_VERTEX_SIZE * 4
#define GEAR_BATCH_RENDERER_2D_BUFFER_SIZE       GEAR_BATCH_RENDERER_2D_OBJ_SIZE * GEAR_BATCH_RENDERER_2D_MAX_OBJ
#define GEAR_BATCH_RENDERER_2D_INDICIES_SIZE     GEAR_BATCH_RENDERER_2D_MAX_OBJ * 6
#define GEAR_BATCH_RENDERER_2D_MAX_TEXTURE_SLOTS 32

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

	BatchRenderer2D();
	~BatchRenderer2D();

	void Init();
};
}
}
}