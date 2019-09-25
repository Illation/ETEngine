#pragma once

#include "ViewportRenderer.h"

#include <EtCore/Content/AssetPointer.h>

#include <Engine/Graphics/Shader.h>


//---------------------------------
// TestRenderer
//
// Renders a Scene to the viewport - #todo: fully merge with render pipeline
//
class TestRenderer final : public I_ViewportRenderer
{
	// construct destruct
	//--------------------
private:
	TestRenderer() : I_ViewportRenderer() {}
	virtual ~TestRenderer() = default;

	// Viewport Renderer Interface
	//-----------------------------
protected:
	void OnInit() override;
	void OnDeinit() override;
	void OnResize(ivec2 const dim) override {}
	void OnRender(T_FbLoc const targetFb) override;

	// Data
	///////
private:

	vec3 m_ClearColor;

	T_ArrayLoc m_Vao;
	T_BufferLoc m_Vbo;
	AssetPtr<ShaderData> m_Shader;
};