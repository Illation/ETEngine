#pragma once
#include "../Helper/Singleton.hpp"
#include "../Base/Time.hpp"
#include "../Components/CameraComponent.hpp"
#include "../SceneGraph/AbstractScene.hpp"

class ContextObjects
{
public:
	ContextObjects()
	{}
	 ~ContextObjects()
	{
	}
	Time* pTime;
	CameraComponent* pCamera;
	AbstractScene* pScene;
};

struct Context : public Singleton<Context>, public ContextObjects
{
public:
	Context():ContextObjects()
	{
	}
	~Context() 
	{ 
	}
	void SetContext(ContextObjects* pContext)
	{
		pTime = pContext->pTime;
		pCamera = pContext->pCamera;
		pScene = pContext->pScene;
	}
};

