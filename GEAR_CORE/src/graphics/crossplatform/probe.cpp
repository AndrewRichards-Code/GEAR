#include "probe.h"

using namespace GEAR;
using namespace GRAPHICS;
using namespace CROSSPLATFORM;
using namespace ARM;

Probe::Probe(const Vec3& position, int width, int height, int multisample, OPENGL::Texture::ImageFormat format)
	:m_Position(position), m_Width(width), m_Height(height), m_Multisample(multisample), m_Format(format)
{
	m_FrameBuffer = std::make_shared<OPENGL::FrameBuffer>(m_Width, m_Height, true, m_Multisample, m_Format);
	m_Camera = std::make_unique<Camera>(GEAR_CAMERA_PERSPECTIVE, m_Position, Vec3(0, 0, -1), Vec3(0, 1, 0));
}
Probe::~Probe()
{

}

std::shared_ptr<OPENGL::Texture> Probe::ConstructCubemap()
{
	return m_Cubemap;
}

void Probe::Resolve()
{
	m_FrameBuffer->Resolve();
	m_IsResolved = true;
}

void Probe::Render(std::deque<Object*> renderQueue, int windowWidth, int windowHeight)
{
	glViewport(0, 0, m_Width, m_Height);

	m_FrameBuffer->Bind();
	for (int i = 0; i < 6; i++)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);;
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		m_FrameBuffer->UseColourTextureAttachment(i);
		m_Camera->DefineProjection(DegToRad(90), (static_cast<float>(m_Width) / (static_cast<float>(m_Height))), 0.01f, 1500.0f, true, true);
		if(i==2||i==3)
			m_Camera->DefineProjection(DegToRad(90), (static_cast<float>(m_Width) / (static_cast<float>(m_Height))), 0.01f, 1500.0f, false, false);

		m_Camera->DefineView(m_ViewMatrices[i]);
		m_Camera->UpdateCameraPosition();
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
			obj->GetShader().Enable();
			obj->GetVAO()->Bind();
			obj->GetIBO()->Bind();
			glDrawElements(GL_TRIANGLES, obj->GetIBO()->GetCount(), GL_UNSIGNED_INT, nullptr);

		}
		if (!m_IsResolved)
			Resolve();

		m_Cubemap->BindCubeMap();
		glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
		glCopyTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, (unsigned int)OPENGL::Texture::ToBaseFormat(m_Format), 0, 0, m_Width, m_Height, 0);
		m_Cubemap->UnbindCubeMap();
	}
	renderQueue.back()->GetIBO()->Unbind();
	renderQueue.back()->GetVAO()->Unbind();
	renderQueue.back()->GetShader().Disable();

	m_FrameBuffer->Unbind();
	glViewport(0, 0, windowWidth, windowHeight);
}
