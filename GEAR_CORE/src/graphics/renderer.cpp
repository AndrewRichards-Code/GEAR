#include "renderer.h"

using namespace GEAR;
using namespace GRAPHICS;

void Renderer::Submit(const Object* obj)
{
	m_RenderQueue.push_back((Object*) obj);
}

void Renderer::Draw(const VertexArray& vao, const IndexBuffer& ibo, const Shader& shader) const
{
	shader.Enable();
	vao.Bind();
	ibo.Bind();
	glDrawElements(GL_TRIANGLES, ibo.GetCount(), GL_UNSIGNED_INT, nullptr);
	shader.Disable();
	vao.Unbind();
	ibo.Unbind();

}

void Renderer::Draw(const Object* obj) const
{
	obj->GetShader().Enable();
	obj->GetVAO()->Bind();
	obj->GetIBO()->Bind();
	glDrawElements(GL_TRIANGLES, obj->GetIBO()->GetCount(), GL_UNSIGNED_INT, nullptr);
	obj->GetShader().Disable();
	obj->GetVAO()->Unbind();
	obj->GetIBO()->Unbind();

}

void Renderer::Flush()
{
	while (!m_RenderQueue.empty())
	{
		const Object* obj = m_RenderQueue.front();

		obj->GetShader().Enable();
		obj->GetTexture().Bind(0);
		obj->GetShader().SetUniform<int>("u_Texture", { 0 });
		obj->GetShader().SetUniformMatrix<4>("u_Modl", 1, GL_TRUE, obj->GetModlMatrix().a);
		obj->GetVAO()->Bind();
		obj->GetIBO()->Bind();
		glDrawElements(GL_TRIANGLES, obj->GetIBO()->GetCount(), GL_UNSIGNED_INT, nullptr);
		obj->GetShader().Disable();
		obj->GetVAO()->Unbind();
		obj->GetIBO()->Unbind();

		m_RenderQueue.pop_front();
	}
}

void Renderer::Clear() const
{
	glClear(GL_COLOR_BUFFER_BIT);
}