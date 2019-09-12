#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <Engine/SceneGraph/AbstractScene.h>


class FrameBuffer;
class TexPBRMaterial;
class ParamPBRMaterial;
class EmissiveMaterial;
class PointLight;
class Entity;
class Skybox;
class DirectionalLight;
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
	void DrawForward();
	void PostDraw();
private:
	TexPBRMaterial* m_pMat = nullptr;
	ParamPBRMaterial* m_pStandMat = nullptr;
	ParamPBRMaterial* m_pEnvMat = nullptr;
	Entity* m_pLigEntity = nullptr;
	DirectionalLight* m_pLight = nullptr;

	AssetPtr<SpriteFont> m_pDebugFont;
};

