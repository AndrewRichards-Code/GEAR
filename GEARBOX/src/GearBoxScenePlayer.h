#pragma once

#include "Scene/Scene.h"
#include "Graphics/RenderSurface.h"

#include <QtWidgets/QWidget.h>
#include "ui_GearBoxScenePlayer.h"

class GearBoxScenePlayer : public QWidget
{
	Q_OBJECT

public:
	GearBoxScenePlayer(QWidget *parent = Q_NULLPTR);
	~GearBoxScenePlayer();

	void RetranslateUI();

	void Initialise(gear::Ref<gear::scene::Scene>& activeScene);
	void UpdateRenderingLabels(const gear::Ref<gear::graphics::RenderSurface>& renderSurface);

private slots:
	void PlayScene();
	void StopScene();
	void ReloadScripts();

private:
	Ui::GearBoxScenePlayer ui;

	gear::Ref<gear::scene::Scene> m_ActiveScene;
};
