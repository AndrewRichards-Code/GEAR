#pragma once

#include "gear_core.h"

#include <QtWidgets/QMainWindow>
#include <QtCore/QTranslator.h>
#include <QtCore/QTimer.h>
#include "ui_GearBox.h"

class GearBox : public QMainWindow
{
    Q_OBJECT

public:
    GearBox(QWidget *parent = Q_NULLPTR);
    ~GearBox();

private:
    void changeEvent(QEvent* event) override;

private slots:
    void Render();
    void LanguageChanged(QAction* action);
    void PlayScene();
    void StopScene();
    void UpdateRenderingLabels();
    void ReloadScripts();

private:
    Ui::GearBoxClass ui;
    QTimer* timer;
    QTranslator translator;

    gear::Ref<gear::graphics::RenderSurface> m_RenderSurface;
    gear::graphics::RenderSurface::CreateInfo m_RenderSurfaceCI;

    gear::Ref<gear::scene::Scene> activeScene;
    gear::scene::Scene::CreateInfo sceneCI;

    gear::core::Timer gearTimer;
    bool windowResize = false;
    gear::Ref<gear::graphics::Renderer> renderer;

    gear::scene::Entity cameraEnitity;
};
