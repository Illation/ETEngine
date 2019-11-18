#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <EtFramework/SceneGraph/AbstractScene.h>


class TexPBRMaterial;
class ParamPBRMaterial;
class LightComponent;
class SpriteFont;


class ShadingTestScene : public AbstractScene
{
public:
	ShadingTestScene();
	~ShadingTestScene();
private:
	void Initialize();
	void Update();
	void Draw();
	void DrawForward() {}
	void PostDraw() {}
private:
	TexPBRMaterial* m_pMat = nullptr;
	ParamPBRMaterial* m_pStandMat = nullptr;
	ParamPBRMaterial* m_pEnvMat = nullptr;
	LightComponent* m_Light = nullptr;

	AssetPtr<SpriteFont> m_pDebugFont;
};

