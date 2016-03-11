#pragma once
#include "../Helper/Singleton.h"
#include "../Base/Time.hpp"
#include "../Graphics/Camera.hpp"

struct Context : public Singleton<Context>
{
public:
	Context():pTime(new Time())
	{

	}
	~Context() 
	{ 
		delete pTime;
		pTime = nullptr;
	}
	Time* pTime;
	Camera* pCamera;
};

