#pragma once
#include <EtCore/Content/AssetPointer.h>


namespace et {
namespace render {


class ShaderData;


class ScreenSpaceReflections
{
public:
	ScreenSpaceReflections() = default;
	virtual ~ScreenSpaceReflections();
	void Initialize();

	void EnableInput();
	T_FbLoc GetTargetFBO() { return m_CollectFBO; }

	void Draw();
private:

	AssetPtr<ShaderData> m_pShader;

	T_FbLoc m_CollectFBO;
	TextureData* m_CollectTex = nullptr;
	T_RbLoc m_CollectRBO;
};


} // namespace render
} // namespace et
