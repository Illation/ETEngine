#include "stdafx.hpp"
#include "EditorRenderer.hpp"
#include "Editor.hpp"
#include "SpriteRenderer.hpp"
#include "TextRenderer.hpp"
#include "PrimitiveRenderer.hpp"
#include "TextureData.hpp"
#include "ShaderData.hpp"
#include "UI/UIViewport.h"

#ifdef EDITOR

EditorRenderer::EditorRenderer()
{

}

EditorRenderer::~EditorRenderer()
{
}

void EditorRenderer::Initialize()
{
}

void EditorRenderer::Draw()
{
	STATE->BindFramebuffer(0);

	//temporary until ui system is in place
	SpriteRenderer::GetInstance()->Draw(nullptr, vec2(0), vec4(0.2f), vec2(0), vec2(200), 0, 1, SpriteScalingMode::TEXTURE);

	DrawRenderers();

	Editor::GetInstance()->GetViewport()->Draw(0);
}

void EditorRenderer::DrawRenderers()
{
	SpriteRenderer::GetInstance()->Draw();
	TextRenderer::GetInstance()->Draw();
}

#endif
