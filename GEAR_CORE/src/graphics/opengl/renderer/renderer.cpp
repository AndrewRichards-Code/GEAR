#include "renderer.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace OPENGL;
using namespace CROSSPLATFORM;

void Renderer::Submit(Object* obj)
{
	m_RenderQueue.push_back((Object*) obj);
}

void Renderer::Flush()
{
	while (!m_RenderQueue.empty())
	{
		Object* obj = m_RenderQueue.front();

		if (obj->GetTexture().IsCubeMap() == true)
		{
			obj->BindCubeMap(0);
		}
		else
		{
			obj->BindTexture(0);
		}
		obj->SetUniformModlMatrix();

		obj->GetShader().Enable();
		obj->GetVAO()->Bind();
		obj->GetIBO()->Bind();
		glDrawElements(m_DrawMode, obj->GetIBO()->GetCount(), GL_UNSIGNED_INT, nullptr);
		obj->GetIBO()->Unbind();
		obj->GetVAO()->Unbind();
		obj->GetShader().Disable();

		m_RenderQueue.pop_front();
	}

}

void Renderer::Draw(const VertexArray& vao, const IndexBuffer& ibo, const Shader& shader) const
{
	shader.Enable();
	vao.Bind();
	ibo.Bind();
	glDrawElements(m_DrawMode, ibo.GetCount(), GL_UNSIGNED_INT, nullptr);
	ibo.Unbind();
	vao.Unbind();
	shader.Disable();
}

void Renderer::Draw(Object* obj) const
{
	if (obj->GetTexture().IsCubeMap() == true)
	{
		obj->BindCubeMap(0);
	}
	else
	{
		obj->BindTexture(0);
	}
	obj->SetUniformModlMatrix();
	obj->GetShader().Enable();
	obj->GetVAO()->Bind();
	obj->GetIBO()->Bind();
	glDrawElements(m_DrawMode, obj->GetIBO()->GetCount(), GL_UNSIGNED_INT, nullptr);
	obj->GetIBO()->Unbind();
	obj->GetVAO()->Unbind();
	obj->GetShader().Disable();
}

void Renderer::Clear() const
{
	glClear(GL_COLOR_BUFFER_BIT);
}