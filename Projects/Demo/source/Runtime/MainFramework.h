#pragma once
#include "FreeCamera.h"

#include <EtCore/Content/AssetPointer.h>

#include <EtRuntime/AbstractFramework.h>


namespace et { namespace render {
	class SpriteFont;
} }


namespace et {
namespace demo {


//--------------------------
// MainFramework
//
// User facing wrapper around the engine
//
class MainFramework final : public rt::AbstractFramework
{
	// construct destruct
	//--------------------
public:
	MainFramework() : AbstractFramework() {} // initializes the engine
	~MainFramework() = default;

	// framework interface
	//---------------------
private:
	void OnSystemInit() override;
	void OnInit() override;
	void OnTick() override;

	// Data
	///////

	size_t m_CurrentScene = 0u;
	std::vector<T_Hash> m_Scenes;

	AssetPtr<render::SpriteFont> m_DebugFont;
	bool m_DrawDebugInfo = true;
	bool m_DrawFontAtlas = false;
};


} // namespace demo
} // namespace et
