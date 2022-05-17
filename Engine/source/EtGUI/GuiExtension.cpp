#include "stdafx.h"
#include "GuiExtension.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GraphicsTypes/TextureData.h>


namespace et {
namespace gui {


//===============
// GUI Extension
//===============


core::HashString const GuiExtension::s_ExtensionId("GuiExtension");


//-------------------------
// GuiExtension::AddSprite
//
core::T_SlotId GuiExtension::AddSprite(core::HashString const textureId, render::T_NodeId const node, vec2 const pivot, vec4 const& color)
{
	auto sprite = m_Sprites.insert(Sprite());

	sprite.first->node = node;
	sprite.first->pivot = pivot;
	sprite.first->color = color;
	sprite.first->texture = core::ResourceManager::Instance()->GetAssetData<render::TextureData>(textureId);

	return sprite.second;
}

//-----------------------------------
// GuiExtension::UpdateSpriteTexture
//
void GuiExtension::UpdateSpriteTexture(core::T_SlotId const spriteId, core::HashString const textureId)
{
	m_Sprites[spriteId].texture = core::ResourceManager::Instance()->GetAssetData<render::TextureData>(textureId);
}

//---------------------------------
// GuiExtension::UpdateSpritePivot
//
void GuiExtension::UpdateSpritePivot(core::T_SlotId const spriteId, vec2 const pivot)
{
	m_Sprites[spriteId].pivot = pivot;
}

//---------------------------------
// GuiExtension::UpdateSpriteColor
//
void GuiExtension::UpdateSpriteColor(core::T_SlotId const spriteId, vec4 const& color)
{
	m_Sprites[spriteId].color = color;
}

//----------------------------
// GuiExtension::RemoveSprite
//
void GuiExtension::RemoveSprite(core::T_SlotId const spriteId)
{
	m_Sprites.erase(spriteId);
}


} // namespace gui
} // namespace et

