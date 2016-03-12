#pragma once
#include "../Helper/Singleton.h"
#include "../Base/Time.hpp"
#include "../Graphics/Camera.hpp"

class ContextObjects
{
public:
	ContextObjects()
	{}
	 ~ContextObjects()
	{
	}
	Time* pTime;
	Camera* pCamera;
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
	}
};

