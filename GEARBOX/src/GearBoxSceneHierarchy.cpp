#include "GearBoxSceneHierarchy.h"

using namespace gear;
using namespace scene;

GearBoxSceneHierarchy::GearBoxSceneHierarchy(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

GearBoxSceneHierarchy::~GearBoxSceneHierarchy()
{
}

void GearBoxSceneHierarchy::Initialise(gear::Ref<Scene>& activeScene)
{
	m_ActiveScene = activeScene;
}

void GearBoxSceneHierarchy::RetranslateUI()
{
	ui.retranslateUi(this);
}

void GearBoxSceneHierarchy::Update()
{
	auto& vNameComponents = m_ActiveScene->GetRegistry().view<NameComponent>();
	for (auto& entity : vNameComponents)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem(ui.sceneHierarchy);
		item->setText(0, QString(vNameComponents.get<NameComponent>(entity).name.c_str()));
		ui.sceneHierarchy->addTopLevelItem(item);
	}
}
