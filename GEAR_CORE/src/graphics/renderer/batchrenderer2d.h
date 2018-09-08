#pragma once

#include "GL/glew.h"
#include "renderer.h"
#include "../../maths/ARMLib.h"
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
class BatchRenderer2D : public Renderer
{
private:
	GLuint m_VAO;
	GLuint m_VBO;
	IndexBuffer* m_IBO;
	const Object* m_Object;
	GLsizei m_IndexCount;

	Object::VertexData* m_BatchBuffer;
	std::vector<unsigned int> m_TextureSlots;
	
public:
	void OpenMapBuffer();
	void CloseMapBuffer();
	void Submit(const Object* obj) override;
	void Flush() override;

	BatchRenderer2D();
	~BatchRenderer2D();

	void Init();
};
}
}