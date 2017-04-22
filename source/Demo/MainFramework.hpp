#pragma once
#include "../Engine/Base/AbstractFramework.hpp"

class MainFramework : public AbstractFramework
{
public:
	MainFramework();
	~MainFramework();

private:
	void Initialize();
	void Update();
};

