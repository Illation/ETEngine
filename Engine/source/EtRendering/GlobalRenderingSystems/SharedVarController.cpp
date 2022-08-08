#include "stdafx.h"
#include "SharedVarController.h"

#include <EtRHI/GraphicsTypes/TextureData.h>

#include <EtRendering/GraphicsTypes/Camera.h>
#include <EtRendering/SceneRendering/Gbuffer.h>


namespace et {
namespace render {


//=======================
// Shared Var Controller
//=======================


//----------------------------
// SharedVarController::d-tor
//
SharedVarController::~SharedVarController()
{
	if (m_IsInitialized)
	{
		Deinit();
	}
}

//-----------------------------
// SharedVarController::Deinit
//
void SharedVarController::Init()
{
	rhi::I_GraphicsContextApi* const api = rhi::ContextHolder::GetRenderContext();

	m_BufferLocation = api->CreateBuffer();

	api->BindBuffer(rhi::E_BufferType::Uniform, m_BufferLocation);
	api->SetBufferData(rhi::E_BufferType::Uniform, sizeof(m_Data), nullptr, rhi::E_UsageHint::Dynamic);
	api->BindBuffer(rhi::E_BufferType::Uniform, 0u);

	api->BindBufferRange(rhi::E_BufferType::Uniform, m_BufferBinding, m_BufferLocation, 0, sizeof(m_Data));
}

//-----------------------------
// SharedVarController::Deinit
//
void SharedVarController::Deinit()
{
	rhi::I_GraphicsContextApi* const api = rhi::ContextHolder::GetRenderContext();

	api->DeleteBuffer(m_BufferLocation);
}

//---------------------------------
// SharedVarController::UpdataData
//
void SharedVarController::UpdataData(Camera const& camera, Gbuffer const& gbuffer)
{
	rhi::I_GraphicsContextApi* const api = rhi::ContextHolder::GetRenderContext();

	m_Data.view = camera.GetView();
	m_Data.viewInv = camera.GetViewInv();
	m_Data.projection = camera.GetProj();
	m_Data.viewProjection = camera.GetViewProj();
	m_Data.viewProjectionInv = camera.GetViewProjInv();
	m_Data.staticViewProjection = camera.GetStatViewProj();
	m_Data.staticViewProjectionInv = camera.GetStatViewProjInv();

	core::Time* const time = core::ContextManager::GetInstance()->GetActiveContext()->time;
	m_Data.time = time->GetTime();
	m_Data.deltaTime = time->DeltaTime();

	m_Data.camPos = camera.GetPosition();
	m_Data.projectionA = camera.GetDepthProjA();
	m_Data.projectionB = camera.GetDepthProjB();

	std::vector<rhi::TextureData*> const& gbufferTex = gbuffer.GetTextures();
	m_Data.gbufferA = gbufferTex[0]->GetHandle();
	m_Data.gbufferB = gbufferTex[1]->GetHandle();
	m_Data.gbufferC = gbufferTex[2]->GetHandle();

	api->BindBuffer(rhi::E_BufferType::Uniform, m_BufferLocation);
	api->SetBufferData(rhi::E_BufferType::Uniform, sizeof(m_Data), &m_Data, rhi::E_UsageHint::Dynamic);
	api->BindBuffer(rhi::E_BufferType::Uniform, 0u);
}


} // namespace render
} // namespace et
