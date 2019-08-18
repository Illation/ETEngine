#pragma once
#include "AbstractComponent.h"

#include <EtCore/Content/AssetPointer.h>


class TextureData;


class SpriteComponent final : public AbstractComponent
{
public:
	SpriteComponent(T_Hash const spriteAsset, vec2 const& pivot = vec2(0), vec4 const& color = vec4(1));
	virtual ~SpriteComponent() = default;

	vec2 GetPivot() const { return m_Pivot; }
	vec4 GetColor() const { return m_Color; }

	void SetPivot(vec2 const& pivot) { m_Pivot = pivot; }
	void SetColor(vec4 const& color) { m_Color = color; }
	void SetTexture(T_Hash const spriteAsset);

protected:
	void Initialize() override;
	void Update() override {}
	void Draw() override;
	void DrawForward() override {}

private:

	AssetPtr<TextureData> m_Texture;
	T_Hash m_SpriteAsset;
	vec2 m_Pivot;
	vec4 m_Color;

	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	SpriteComponent( const SpriteComponent& obj );
	SpriteComponent& operator=( const SpriteComponent& obj );
};
