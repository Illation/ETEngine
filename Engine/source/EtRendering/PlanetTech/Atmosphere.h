#pragma once
#include "AtmosphereSettings.h"

#include <EtCore/Content/AssetPointer.h>
#include <EtCore/Containers/slot_map.h>

#include <EtRendering/GraphicsTypes/TextureData.h>


class AtmoPreCompute;


namespace render {


//-----------------------------
// AtmosphereInstance
//
// Draw parameters for an atmosphere within a render scene
//
struct AtmosphereInstance
{
	T_Hash atmosphereId = 0u;

	core::T_SlotId nodeId = core::INVALID_SLOT_ID;
	core::T_SlotId lightId = core::INVALID_SLOT_ID; // for sun direction, color

	float height = 0.f;
	float groundRadius = 0.f;
};


//-----------------------------
// Atmosphere
//
// Rendering data for an atmosphere, the result of a precomputation
//
class Atmosphere
{
public:
	Atmosphere() = default;
	~Atmosphere();

	void Initialize(T_Hash const parameterAssetId);

	T_Hash GetId() const { return m_Id; }

	void Draw(vec3 const& position, float const height, float const groundRadius, vec3 const& sunDir) const;

private:
	friend class AtmospherePrecompute;

	void GetUniforms();

	T_Hash m_Id = 0u;

	AtmosphereParameters m_Params;
	dvec3 m_SkyColor;
	dvec3 m_SunColor;

	//textures for precomputed data
	TextureData* m_TexTransmittance = nullptr;
	TextureData* m_TexIrradiance = nullptr;
	TextureData* m_TexInscatter = nullptr;

	AssetPtr<ShaderData> m_pShader;
};


} // namespace render
