#include "stdafx.h"
#include "SharedVarController.h"

#include <EtRHI/GraphicsTypes/TextureData.h>

#include <EtRendering/GraphicsTypes/Camera.h>


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
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	m_BufferLocation = device->CreateBuffer();

	device->BindBuffer(rhi::E_BufferType::Uniform, m_BufferLocation);
	device->SetBufferData(rhi::E_BufferType::Uniform, sizeof(m_Data), nullptr, rhi::E_UsageHint::Dynamic);
	device->BindBuffer(rhi::E_BufferType::Uniform, 0u);

	device->BindBufferRange(rhi::E_BufferType::Uniform, m_BufferBinding, m_BufferLocation, 0, sizeof(m_Data));
}

//-----------------------------
// SharedVarController::Deinit
//
void SharedVarController::Deinit()
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

	device->DeleteBuffer(m_BufferLocation);
}

//---------------------------------
// SharedVarController::UpdataData
//
void SharedVarController::UpdataData(Camera const& camera)
{
	rhi::I_RenderDevice* const device = rhi::ContextHolder::GetRenderDevice();

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

	device->BindBuffer(rhi::E_BufferType::Uniform, m_BufferLocation);
	device->SetBufferData(rhi::E_BufferType::Uniform, sizeof(m_Data), &m_Data, rhi::E_UsageHint::Dynamic);
	device->BindBuffer(rhi::E_BufferType::Uniform, 0u);
}


} // namespace render
} // namespace et
