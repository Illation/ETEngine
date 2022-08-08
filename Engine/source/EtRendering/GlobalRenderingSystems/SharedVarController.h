#pragma once
#include <EtRHI/Util/SharedVarInterface.h>
#include <EtRHI/GraphicsContext/GraphicsTypes.h>


namespace et {
namespace render {


class Camera;
class Gbuffer;


//--------------------
// SharedVarController
//
// Keeps common uniform variables up to date to avoid this responsibility for shaders
//
class SharedVarController final : public rhi::I_SharedVarController
{
	// definitions
	//---------------
	struct GlobalData
	{
		mat4 view;
		mat4 viewInv;
		mat4 projection;
		mat4 viewProjection;
		mat4 viewProjectionInv;
		mat4 staticViewProjection;
		mat4 staticViewProjectionInv;

		float time = 0.f;
		float deltaTime = 0.f;

		float projectionA = 0.f;
		float projectionB = 0.f;

		vec3 camPos;

		float _padding1;

		rhi::T_TextureHandle gbufferA;
		rhi::T_TextureHandle gbufferB;
		rhi::T_TextureHandle gbufferC;
	};

	// construct deconstruct
	//-----------------------
public:
	SharedVarController() : rhi::I_SharedVarController() {}
	~SharedVarController();

	void Init();
	void Deinit();

	// functionality
	//---------------
	void UpdataData(Camera const& camera, Gbuffer const& gbuffer);

	// accessors
	//-----------
	rhi::T_BufferLoc const GetBufferLocation() const override { return m_BufferLocation; }
	uint32 const GetBufferBinding() const override { return m_BufferBinding; }
	std::string const& GetBlockName() const override { return m_UniformBlockName; }

private:

	// Data
	///////

	bool m_IsInitialized = false;

	GlobalData m_Data;
	rhi::T_BufferLoc m_BufferLocation;

	std::string m_UniformBlockName = "SharedVars";
	uint32 m_BufferBinding = 0u;
};


} // namespace render
} // namespace et
