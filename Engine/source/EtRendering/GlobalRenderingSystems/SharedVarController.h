#pragma once
#include <EtRendering/GraphicsContext/GraphicsTypes.h>


class Camera;
class Gbuffer;


namespace render {


//--------------------
// SharedVarController
//
// Keeps common uniform variables up to date to avoid this responsibility for shaders
//
class SharedVarController final
{
	// definititions
	//---------------
	struct GlobalData
	{
		mat4 view;
		mat4 viewInv;
		mat4 projection;
		mat4 viewProjection;
		mat4 viewProjectionInv;

		float time = 0.f;
		float deltaTime = 0.f;

		vec3 camPos;
		float _padding;

		float projectionA = 0.f;
		float projectionB = 0.f;

		// samplers
		int32 gbufferSamplerA = -1; // depth

		int32 gbufferSamplerB = -1;
		int32 gbufferSamplerC = -1;

		// maybe in the future last frame
	};

	// construct deconstruct
	//-----------------------
public:
	SharedVarController() = default;
	~SharedVarController();

	void Init();
	void Deinit();

	// functionality
	//---------------
	void UpdataData(Camera const& camera, Gbuffer const& gbuffer);

	// accessors
	//-----------
	T_BufferLoc const GetBufferLocation() const { return m_BufferLocation; }
	uint32 const GetBufferBinding() const { return m_BufferBinding; }

private:

	// Data
	///////

	bool m_IsInitialized = false;

	GlobalData m_Data;
	T_BufferLoc m_BufferLocation;
	uint32 m_BufferBinding = 0u;
};


} // namespace render
