#pragma once
#include <EtCore/Helper/Context.h>

#include <Engine/Components/CameraComponent.h>
#include <Engine/SceneGraph/AbstractScene.h>


struct SceneContext : public BaseContext
{
	SceneContext() : BaseContext() {}
	virtual ~SceneContext()
	{
		SafeDelete(camera);
		SafeDelete(scene);
	}

	CameraComponent* camera = nullptr;
	AbstractScene* scene = nullptr;
};
