#include "renderer.h"

using namespace GEAR;
using namespace GRAPHICS;

void Renderer::AddLight(Light* light)
{
	m_Lights.push_back(light);
}

void Renderer::Submit(const Object* obj)
{
	m_RenderQueue.push_back((const Object*) obj);
}

void Renderer::Flush()
{
	while (!m_RenderQueue.empty())
	{
		const Object* obj = m_RenderQueue.front();

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
		glDrawElements(GL_TRIANGLES, obj->GetIBO()->GetCount(), GL_UNSIGNED_INT, nullptr);
		obj->GetIBO()->Unbind();
		obj->GetVAO()->Unbind();
		obj->GetShader().Disable();

		//DepthRender
		/*for (int i = 0; i < (signed int)m_Lights.size(); i++)
		{
			m_Lights[i]->GetDepthShader().Enable();
			m_Lights[i]->CalculateLightMVP();
			obj->GetVAO()->Bind();
			obj->GetIBO()->Bind();
			glDrawElements(GL_TRIANGLES, obj->GetIBO()->GetCount(), GL_UNSIGNED_INT, nullptr);
			obj->GetIBO()->Unbind();
			obj->GetVAO()->Unbind();
			m_Lights[i]->GetDepthShader().Disable();
		}*/

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

	//DepthRender
	/*for (int i = 0; i < (signed int)m_Lights.size(); i++)
	{
		m_Lights[i]->GetDepthShader().Enable();
		m_Lights[i]->CalculateLightMVP();
		vao.Bind();
		ibo.Bind();
		glDrawElements(GL_TRIANGLES, ibo.GetCount(), GL_UNSIGNED_INT, nullptr);
		ibo.Unbind();
		vao.Unbind();
		m_Lights[i]->GetDepthShader().Disable();
	}*/
}

void Renderer::Draw(const Object* obj) const
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
	glDrawElements(GL_TRIANGLES, obj->GetIBO()->GetCount(), GL_UNSIGNED_INT, nullptr);
	obj->GetIBO()->Unbind();
	obj->GetVAO()->Unbind();
	obj->GetShader().Disable();

	//DepthRender
	/*for (int i = 0; i < (signed int)m_Lights.size(); i++)
	{
		m_Lights[i]->GetDepthShader().Enable();
		m_Lights[i]->CalculateLightMVP();
		obj->GetVAO()->Bind();
		obj->GetIBO()->Bind();
		glDrawElements(GL_TRIANGLES, obj->GetIBO()->GetCount(), GL_UNSIGNED_INT, nullptr);
		obj->GetIBO()->Unbind();
		obj->GetVAO()->Unbind();
		m_Lights[i]->GetDepthShader().Disable();
	}*/
}

void Renderer::Clear() const
{
	glClear(GL_COLOR_BUFFER_BIT);
}