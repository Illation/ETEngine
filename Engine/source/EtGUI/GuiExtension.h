#pragma once
#include <EtCore/Content/AssetPointer.h>
#include <EtCore/Containers/slot_map.h>

#include <EtRendering/Extensions/SceneExtension.h>
#include <EtRendering/SceneStructure/NodeIdFwd.h>

#include <EtGUI/Context/ContextContainer.h>


namespace et { namespace rendering {
	class TextureData;
} }


namespace et {
namespace gui {


//---------------------------------
// GuiExtension
//
// Scene render data for UI
//
class GuiExtension final : public render::I_SceneExtension
{
public:
	static core::HashString const s_ExtensionId;

	//----------------------
	// Sprite
	//
	// Data required to draw a sprite in a scene
	//
	struct Sprite
	{
		core::T_SlotId node = core::INVALID_SLOT_ID;
		vec2 pivot;

		vec4 color;
		AssetPtr<render::TextureData> texture;
	};

	// construct destruct
	//--------------------
	GuiExtension() : I_SceneExtension() {}
	~GuiExtension() = default;

	// I_SceneExtension interface
	//----------------------------
	core::HashString GetId() const override { return s_ExtensionId; }

	// functionality
	//---------------
	core::T_SlotId AddSprite(core::HashString const textureId, render::T_NodeId const node, vec2 const pivot, vec4 const& color);
	void UpdateSpriteTexture(core::T_SlotId const spriteId, core::HashString const textureId);
	void UpdateSpritePivot(core::T_SlotId const spriteId, vec2 const pivot);
	void UpdateSpriteColor(core::T_SlotId const spriteId, vec4 const& color);
	void RemoveSprite(core::T_SlotId const spriteId);
	
	// accessors
	//-----------
	core::slot_map<Sprite> const& GetSprites() const { return m_Sprites; }

	ContextContainer& GetContextContainer() { return m_ContextContainer; }
	ContextContainer const& GetContextContainer() const { return m_ContextContainer; }


	// Data
	///////

private:
	core::slot_map<Sprite> m_Sprites;
	ContextContainer m_ContextContainer;
};


} // namespace gui
} // namespace et
