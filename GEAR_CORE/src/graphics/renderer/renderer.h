#pragma once

#include "GL/glew.h"
#include "../../maths/ARMLib.h"
#include "../buffer/vertexarray.h"
#include "../buffer/indexbuffer.h"
#include "../shader.h"
#include "../object/object.h"
#include <deque>

namespace GEAR {
namespace GRAPHICS {
class Renderer
{
private:
	std::deque<const Object*> m_RenderQueue;
public:
	virtual void Submit(const Object* obj);
	virtual void Flush();

	void Draw(const VertexArray& vao, const IndexBuffer& ibo, const Shader& shader) const;
	void Draw(const Object* obj) const;

	void Clear() const;
};
}
}
