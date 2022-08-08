#pragma once
#include "Triangulator.h"

#include <EtCore/Content/AssetPointer.h>
#include <EtCore/Containers/slot_map.h>

#include <EtRHI/GraphicsTypes/TextureData.h>


namespace et {
namespace render {


// forward
class Frustum;
class Triangulator;
class Patch;
class Atmosphere;


//-----------------------------
// PlanetParams
//
// Descriptor for a planet
//
struct PlanetParams
{
	PlanetParams() = default;

	core::HashString texDiffuseId;
	core::HashString texDetail1Id;
	core::HashString texDetail2Id;

	core::HashString texHeightId;
	core::HashString texHeightDetailId;

	float radius = 0.f;
	float height = 0.f;
};

//-----------------------------
// Planet
//
// Rendering data for a planet
//
class Planet final
{
	// construct destruct
	//--------------------
public:
	Planet() = default;
	~Planet() = default;

	void Init(PlanetParams const& params, core::T_SlotId const nodeId);

	// Accessors
	//-----------
	core::T_SlotId GetNodeId() const { return m_Node; }
	float GetRadius() const { return m_Radius; }
	float GetMaxHeight() const { return m_MaxHeight; }

	Triangulator& GetTriangulator() { return m_Triangulator; }
	Triangulator const& GetTriangulator() const { return m_Triangulator; }

	rhi::TextureData const* GetTexDiffuse() const { return m_TexDiffuse.get(); }
	rhi::TextureData const* GetTexDetail1() const { return m_TexDetail1.get(); }
	rhi::TextureData const* GetTexDetail2() const { return m_TexDetail2.get(); }
	rhi::TextureData const* GetTexHeight() const { return m_TexHeight.get(); }
	rhi::TextureData const* GetTexHeightDetail() const { return m_TexHeightDetail.get(); }

	// Data
	///////
private:

	core::T_SlotId m_Node = core::INVALID_SLOT_ID;

	float m_Radius = 0.f;
	float m_MaxHeight = 0.f;

	AssetPtr<rhi::TextureData> m_TexDiffuse;
	AssetPtr<rhi::TextureData> m_TexDetail1;
	AssetPtr<rhi::TextureData> m_TexDetail2;

	AssetPtr<rhi::TextureData> m_TexHeight;
	AssetPtr<rhi::TextureData> m_TexHeightDetail;

	Triangulator m_Triangulator;
};


} // namespace render
} // namespace et
