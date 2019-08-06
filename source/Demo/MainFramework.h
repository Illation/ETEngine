#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <Engine/Base/AbstractFramework.h>


class StubData;


class MainFramework : public AbstractFramework
{
public:
	MainFramework();
	~MainFramework();

private:
	void AddScenes();
	void OnTick() override;

	// Data
	///////

	AssetPtr<StubData> m_PostDefTxt;
	AssetPtr<StubData> m_LoremTxt;
};

