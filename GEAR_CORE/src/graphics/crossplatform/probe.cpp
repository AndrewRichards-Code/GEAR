#include "probe.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace CROSSPLATFORM;
using namespace mars;

//--------OmniProbe--------//
OmniProbe::OmniProbe(const Vec3& position, int size, int multisample, OPENGL::Texture::ImageFormat format)
	:m_Position(position), m_Size(size), m_Multisample(multisample), m_Format(format)
{
	m_FrameBuffer = std::make_shared<OPENGL::FrameBuffer>(m_Size, m_Size, m_Multisample, true, m_Format);
	m_Camera = std::make_shared<Camera>(GEAR_CAMERA_PERSPECTIVE, m_Position, Vec3(0, 0, -1), Vec3(0, 1, 0));
}
OmniProbe::~OmniProbe()
{

}

void OmniProbe::Resolve()
{
	m_FrameBuffer->Resolve();
}

void OmniProbe::Render(const std::deque<Object*>& renderQueue, int windowWidth, int windowHeight, const OPENGL::Shader* overrideShader)
{
	glViewport(0, 0, m_Size, m_Size);
	m_FrameBuffer->Bind();

	for (int i = 0; i < 6; i++)
	{
		m_FrameBuffer->DrawToColourTextureAttachment(i);
		m_FrameBuffer->Clear();
		m_Camera->DefineProjection(DegToRad(90), 1.0f, 0.01f, 1500.0f, true, true);
		if(i==2||i==3)
			m_Camera->DefineProjection(DegToRad(90), 1.0f, 0.01f, 1500.0f, false, false);

		m_Camera->UpdateCameraPosition();
		m_Camera->DefineView(m_ViewMatrices[i]);
		for (Object* obj : renderQueue)
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
			overrideShader ? overrideShader->Enable() : obj->GetShader().Enable();
			obj->GetVAO()->Bind();
			obj->GetIBO()->Bind();
			glDrawElements(GL_TRIANGLES, obj->GetIBO()->GetCount(), GL_UNSIGNED_INT, nullptr);

		}
		/*r.OpenMapBuffer();
		for (Object* obj : renderQueue)
			r.Submit(obj);
		r.CloseMapBuffer();
		overrideShader ? r.Flush(overrideShader) : r.Flush();*/
	}
	renderQueue.back()->GetIBO()->Unbind();
	renderQueue.back()->GetVAO()->Unbind();
	overrideShader ? overrideShader->Disable() : renderQueue.back()->GetShader().Disable();
	
	m_FrameBuffer->Unbind();
	glViewport(0, 0, windowWidth, windowHeight);
		
	Resolve();

	m_FrameBuffer->BindResolved();
	for (int i = 0; i < 6; i++)
	{
		m_Cubemap->BindCubeMap();
		glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
		glCopyTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, (unsigned int)OPENGL::Texture::ToBaseFormat(m_Format), 0, 0, m_Size, m_Size, 0);
		m_Cubemap->UnbindCubeMap();
	}
	m_FrameBuffer->UnbindResolved();
}

void OmniProbe::UpdatePosition(const Vec3& position)
{
	m_Camera->m_Position = position;
}

//--------UniProbe--------//
UniProbe::UniProbe(const Vec3& position, const mars::Vec3& direction, int size, int projectionType, int multisample, OPENGL::Texture::ImageFormat format)
	:m_Position(position), m_Direction(direction), m_Size(size), m_Multisample(multisample), m_Format(format)
{
	m_FrameBuffer = std::make_shared<OPENGL::FrameBuffer>(m_Size, m_Size, m_Multisample, false, m_Format);
	m_Camera = std::make_shared<Camera>(projectionType, m_Position, m_Direction, Vec3(0, 1, 0));
}
UniProbe::~UniProbe()
{

}

void UniProbe::Resolve()
{
	m_FrameBuffer->Resolve();
}

void UniProbe::Render(const std::deque<Object*>& renderQueue, int windowWidth, int windowHeight, const OPENGL::Shader* overrideShader)
{
	glViewport(0, 0, m_Size, m_Size);
	m_FrameBuffer->Bind();

	m_FrameBuffer->DrawToColourTextureAttachment();
	m_FrameBuffer->Clear();
	m_Camera->DefineProjection(DegToRad(90), 1.0f, 0.01f, 1500.0f, true, true);

	m_Camera->UpdateCameraPosition();
	m_Camera->DefineView();
	for (Object* obj : renderQueue)
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
		overrideShader ? overrideShader->Enable() : obj->GetShader().Enable();
		obj->GetVAO()->Bind();
		obj->GetIBO()->Bind();
		glDrawElements(GL_TRIANGLES, obj->GetIBO()->GetCount(), GL_UNSIGNED_INT, nullptr);

	}
	/*r.OpenMapBuffer();
	for (Object* obj : renderQueue)
		r.Submit(obj);
	r.CloseMapBuffer();
	overrideShader ? r.Flush(overrideShader) : r.Flush();*/
	renderQueue.back()->GetIBO()->Unbind();
	renderQueue.back()->GetVAO()->Unbind();
	overrideShader ? overrideShader->Disable() : renderQueue.back()->GetShader().Disable();

	m_FrameBuffer->Unbind();
	glViewport(0, 0, windowWidth, windowHeight);

	Resolve();

	m_FrameBuffer->BindResolved();
	m_Texture->Bind();
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, (unsigned int)OPENGL::Texture::ToBaseFormat(m_Format), 0, 0, m_Size, m_Size, 0);
	m_Texture->Unbind();
	m_FrameBuffer->UnbindResolved();
}

void UniProbe::UpdatePosition(const Vec3& position)
{
	m_Camera->m_Position = position;
}