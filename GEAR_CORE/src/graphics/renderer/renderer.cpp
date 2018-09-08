#include "renderer.h"

using namespace GEAR;
using namespace GRAPHICS;

void Renderer::Submit(const Object* obj)
{
	m_RenderQueue.push_back((const Object*) obj);
}

void Renderer::Flush()
{
	while (!m_RenderQueue.empty())
	{
		const Object* obj = m_RenderQueue.front();

		obj->BindTexture(0);
		obj->SetUniformModlMatrix();

		obj->GetShader().Enable();
		obj->GetVAO()->Bind();
		obj->GetIBO()->Bind();
		glDrawElements(GL_TRIANGLES, obj->GetIBO()->GetCount(), GL_UNSIGNED_INT, nullptr);
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
	glDrawElements(GL_TRIANGLES, ibo.GetCount(), GL_UNSIGNED_INT, nullptr);
	ibo.Unbind();
	vao.Unbind();
	shader.Disable();

}

void Renderer::Draw(const Object* obj) const
{
	obj->GetShader().Enable();
	obj->GetVAO()->Bind();
	obj->GetIBO()->Bind();
	glDrawElements(GL_TRIANGLES, obj->GetIBO()->GetCount(), GL_UNSIGNED_INT, nullptr);
	obj->GetIBO()->Unbind();
	obj->GetVAO()->Unbind();
	obj->GetShader().Disable();

}

void Renderer::Clear() const
{
	glClear(GL_COLOR_BUFFER_BIT);
}