#pragma once
#include "AbstractComponent.hpp"

class TextureData;

class SpriteComponent : public AbstractComponent
{
public:
	SpriteComponent( const std::string& spriteAsset, vec2 pivot = vec2( 0 ), vec4 color = vec4(1) );
	~SpriteComponent();

	vec2 GetPivot() const { return m_Pivot; }
	vec4 GetColor() const { return m_Color; }

	void SetPivot( vec2 pivot ) { m_Pivot = pivot; }
	void SetColor( vec4 color ) { m_Color = color; }
	void SetTexture( const std::string& spriteAsset );

protected:
	virtual void Initialize();
	virtual void Update() {}
	virtual void Draw();
	virtual void DrawForward() {}

private:

	TextureData* m_pTexture;
	std::string m_SpriteAsset;
	vec2 m_Pivot;
	vec4 m_Color;

	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	SpriteComponent( const SpriteComponent& obj );
	SpriteComponent& operator=( const SpriteComponent& obj );
};
