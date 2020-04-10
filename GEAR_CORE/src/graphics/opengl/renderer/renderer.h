#pragma once

#include "gear_common.h"
#include "mars.h"
#include "graphics/opengl/buffer/vertexarray.h"
#include "graphics/opengl/buffer/indexbuffer.h"
#include "graphics/opengl/shader/shader.h"
#include "graphics/crossplatform/object.h"

namespace GEAR {
namespace GRAPHICS {
namespace OPENGL {
class Renderer
{
private:
	std::deque<CROSSPLATFORM::Object*> m_RenderQueue;
	int m_DrawMode = GL_TRIANGLES;

public:
	virtual void Submit(CROSSPLATFORM::Object* obj);
	virtual void Flush();

	void Draw(const VertexArray& vao, const IndexBuffer& ibo, const Shader& shader) const;
	void Draw(CROSSPLATFORM::Object* obj) const;

	void Clear() const;

	inline std::deque<CROSSPLATFORM::Object*>& GetRenderQueue() { return m_RenderQueue; };
};
}
}
}
