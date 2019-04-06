#pragma once

#include "gear_common.h"
#include "maths/ARMLib.h"
#include "graphics/opengl/buffer/vertexarray.h"
#include "graphics/opengl/buffer/indexbuffer.h"
#include "graphics/opengl/shader/shader.h"
#include "graphics/crossplatform/camera.h"
#include "graphics/crossplatform/object.h"
#include "graphics/crossplatform/light.h"

namespace GEAR {
namespace GRAPHICS {
namespace OPENGL {
class Renderer
{
private:
	std::deque<CROSSPLATFORM::Object*> m_RenderQueue;
	int m_DrawMode = GL_TRIANGLES;

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
