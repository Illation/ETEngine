#pragma once
#include <EtCore/Helper/Context.h>

#include <Engine/Components/CameraComponent.h>
#include <Engine/SceneGraph/AbstractScene.h>


struct SceneContext : public BaseContext
{
	SceneContext() : BaseContext() {}
	virtual ~SceneContext() = default;

	CameraComponent* camera = nullptr;
	AbstractScene* scene = nullptr;
};
