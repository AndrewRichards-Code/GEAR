#pragma once

#include "Scene/Scene.h"

#include <QtWidgets/QWidget.h>
#include <QtWidgets/QTreeWidget.h>
#include "ui_GearBoxSceneHierarchy.h"

class GearBoxSceneHierarchy : public QWidget
{
	Q_OBJECT

public:
	GearBoxSceneHierarchy(QWidget *parent = Q_NULLPTR);
	~GearBoxSceneHierarchy();

	void RetranslateUI();

	void Initialise(gear::Ref<gear::scene::Scene>& activeScene);
	void Update();

private:
	Ui::GearBoxSceneHierarchy ui;

	gear::Ref<gear::scene::Scene> m_ActiveScene;
};
