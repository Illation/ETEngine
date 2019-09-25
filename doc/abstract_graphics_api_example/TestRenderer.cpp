#include "stdafx.h"
#include "TestRenderer.h"

#include <EtCore/Content/ResourceManager.h>


//---------------------------------
// TestRenderer::OnInit
//
void TestRenderer::OnInit()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	m_ClearColor = vec3(200.f / 255.f, 114.f / 255.f, 200.f / 255.f)*0.2f;

	m_Shader = ResourceManager::Instance()->GetAssetData<ShaderData>("DebugRenderer.glsl"_hash);

	m_Vao = api->CreateVertexArray();
	m_Vbo = api->CreateBuffer();
	api->BindVertexArray(m_Vao);
	api->BindBuffer(E_BufferType::Vertex, m_Vbo);

	struct TestVertex
	{
		TestVertex(vec3 const& p, vec4 const& c) : pos(p), color(c) {}

		vec3 pos;
		vec4 color;
	};

	TestVertex vertices[] = {
		TestVertex(vec3(-0.5f, -0.5f, 0.0f), vec4(1.f, 0.f, 0.f, 1.f)),// left  
		TestVertex(vec3(0.5f, -0.5f, 0.0f), vec4(1.f)),// right 
		TestVertex(vec3(0.0f, 0.5f, 0.0f), vec4(1.f))  // top   
	};

	api->SetBufferData(E_BufferType::Vertex, sizeof(vertices), vertices, E_UsageHint::Static);

	api->DefineVertexAttributePointer(0, 3, E_DataType::Float, false, sizeof(TestVertex), offsetof(TestVertex, pos));
	api->SetVertexAttributeArrayEnabled(0, true);
	api->DefineVertexAttributePointer(1, 4, E_DataType::Float, false, sizeof(TestVertex), offsetof(TestVertex, color));
	api->SetVertexAttributeArrayEnabled(1, true);

	api->BindBuffer(E_BufferType::Vertex, 0);
	api->BindVertexArray(0);
}

//---------------------------------
// TestRenderer::OnDeinit
//
void TestRenderer::OnDeinit()
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->DeleteVertexArray(m_Vao);
	api->DeleteBuffer(m_Vbo);

	m_Shader = nullptr;
}

//---------------------------------
// TestRenderer::OnRender
//
void TestRenderer::OnRender(T_FbLoc const targetFb)
{
	Config::Settings::Window const& windowSettings = Config::GetInstance()->GetWindow();
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	api->BindFramebuffer(targetFb);

	api->SetViewport(ivec2(0), windowSettings.Dimensions);

	api->SetClearColor(vec4(m_ClearColor, 1.f));
	api->Clear(E_ClearFlag::Color | E_ClearFlag::Depth);

	api->SetShader(m_Shader.get());
	m_Shader->Upload("uViewProj"_hash, mat4());

	api->SetBlendEnabled(false);

	api->BindVertexArray(m_Vao);
	api->DrawArrays(E_DrawMode::Triangles, 0, 3);
}
