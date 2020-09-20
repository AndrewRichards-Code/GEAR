#include "GearBoxScenePlayer.h"

GearBoxScenePlayer::GearBoxScenePlayer(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.playButton, SIGNAL(clicked()), this, SLOT(PlayScene()));
	connect(ui.stopButton, SIGNAL(clicked()), this, SLOT(StopScene()));
	connect(ui.reloadScriptButton, SIGNAL(clicked()), this, SLOT(ReloadScripts()));
}

GearBoxScenePlayer::~GearBoxScenePlayer()
{
}

void GearBoxScenePlayer::RetranslateUI()
{
	ui.retranslateUi(this);
}

void GearBoxScenePlayer::Initialise(gear::Ref<gear::scene::Scene>& activeScene)
{
	m_ActiveScene = activeScene;
}

void GearBoxScenePlayer::PlayScene()
{
	m_ActiveScene->Play();
}

void GearBoxScenePlayer::StopScene()
{
	m_ActiveScene->Stop();
}

void GearBoxScenePlayer::UpdateRenderingLabels(const gear::Ref<gear::graphics::RenderSurface>& renderSurface)
{
	QString apiName = QString(renderSurface->GetGraphicsAPIVersion().c_str());
	ui.renderingAPILabel->setText(apiName);
	QString gpuName = QString(renderSurface->GetDeviceName().c_str());
	ui.gpuDeviceLabel->setText(gpuName);
}

void GearBoxScenePlayer::ReloadScripts()
{
	m_ActiveScene->UnloadNativeScriptLibrary();
	m_ActiveScene->LoadNativeScriptLibrary();
}
