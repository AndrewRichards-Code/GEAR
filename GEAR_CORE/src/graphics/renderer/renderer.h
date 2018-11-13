#pragma once

#include "GL/glew.h"
#include "../../maths/ARMLib.h"
#include "../buffer/vertexarray.h"
#include "../buffer/indexbuffer.h"
#include "../shader.h"
#include "../object.h"
#include "../light.h"
#include <deque>

namespace GEAR {
namespace GRAPHICS {
class Renderer
{
private:
	std::deque<const Object*> m_RenderQueue;

protected:
	std::vector<Light*> m_Lights;

public:
	void AddLight(Light* light);
	virtual void Submit(const Object* obj);
	virtual void Flush();

	void Draw(const VertexArray& vao, const IndexBuffer& ibo, const Shader& shader) const;
	void Draw(const Object* obj) const;

	void Clear() const;

	inline std::vector<Light*> GetLights() const { return m_Lights; }
};
}
}
