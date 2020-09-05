#include "gear_core_common.h"
#include "Entity.h"
#include "INativeScript.h"

using namespace gear;
using namespace scene;

Entity::Entity(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	m_Entity = m_CI.pScene->m_Registry.create();
}

Entity::~Entity()
{
}