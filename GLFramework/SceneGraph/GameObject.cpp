#include "GameObject.hpp"
GameObject::GameObject()
{
}

GameObject::~GameObject()
{
}

void GameObject::RootInitialize()
{
	Initialize();
}
void GameObject::RootStart()
{
	Start();
}
void GameObject::RootDraw()
{
	Draw();
}
void GameObject::RootUpdate()
{
	Update();
}