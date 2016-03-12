#include "Entity.hpp"
#include "AbstractScene.hpp"
Entity::Entity()
{
}

Entity::~Entity()
{
}

void Entity::RootInitialize()
{
	if (m_IsInitialized)
		return;
	Initialize();
	m_IsInitialized = true;
}
void Entity::RootStart()
{
	Start();
}
void Entity::RootDraw()
{
	Draw();
}
void Entity::RootUpdate()
{
	Update();
}