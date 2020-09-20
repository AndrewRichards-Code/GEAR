#pragma once

#include "gear_core.h"

#include <QtWidgets/QMainWindow.h>
#include <QtCore/QTranslator.h>
#include <QtCore/QTimer.h>
#include "ui_GearBox.h"

class GearBox : public QMainWindow
{
    Q_OBJECT

public:
    GearBox(QWidget *parent = Q_NULLPTR);
    ~GearBox();

    void RetranslateUI();

private:
    void changeEvent(QEvent* event) override;

private slots:
    void Render();
    void LanguageChanged(QAction* action);

private:
    Ui::GearBoxClass ui;
    QTimer* timer;
    QTranslator translator;

    gear::Ref<gear::graphics::RenderSurface> m_RenderSurface;
    gear::graphics::RenderSurface::CreateInfo m_RenderSurfaceCI;

    gear::Ref<gear::scene::Scene> m_ActiveScene;
    gear::scene::Scene::CreateInfo m_ActiveSceneCI;

    gear::core::Timer m_GearTimer;
    bool windowResize = false;
    gear::Ref<gear::graphics::Renderer> m_Renderer;

    gear::scene::Entity cameraEnitity;
};
