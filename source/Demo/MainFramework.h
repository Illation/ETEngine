#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <Engine/Base/AbstractFramework.h>


class ShaderData;
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

	AssetPtr<ShaderData> m_PostDefShader;
	AssetPtr<StubData> m_LoremTxt;
};

