#pragma once

#include "GL/glew.h"
#include "../../../maths/ARMLib.h"
#include "../buffer/vertexarray.h"
#include "../buffer/indexbuffer.h"
#include "../shader.h"
#include "../../crossplatform/camera.h"
#include "../../crossplatform/object.h"
#include "../../crossplatform/light.h"
#include <deque>

namespace GEAR {
namespace GRAPHICS {
namespace OPENGL {
class Renderer
{
private:
	std::deque<CROSSPLATFORM::Object*> m_RenderQueue;

protected:
	std::vector<CROSSPLATFORM::Light*> m_Lights;

public:
	void AddLight(CROSSPLATFORM::Light* light);
	virtual void Submit(CROSSPLATFORM::Object* obj);
	virtual void Flush();

	void Draw(const VertexArray& vao, const IndexBuffer& ibo, const Shader& shader) const;
	void Draw(CROSSPLATFORM::Object* obj) const;

	void Clear() const;

	inline std::vector<CROSSPLATFORM::Light*> GetLights() const { return m_Lights; }
};
}
}
}
