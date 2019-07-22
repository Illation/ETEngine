#pragma once
#include <Engine/Base/AbstractFramework.h>


class MainFramework : public AbstractFramework
{
public:
	MainFramework();
	~MainFramework();

private:
	void AddScenes();
	void OnTick() override;
};

