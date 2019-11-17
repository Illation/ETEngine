#pragma once
#include <EtCore/Helper/Context.h>

#include <EtFramework/Components/CameraComponent.h>
#include <EtFramework/SceneGraph/AbstractScene.h>


struct SceneContext : public BaseContext
{
	SceneContext() : BaseContext() {}
	virtual ~SceneContext() = default;

	CameraComponent* camera = nullptr;
	AbstractScene* scene = nullptr;
};
