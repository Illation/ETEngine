#pragma once
#include "Triangulator.h"

#include <EtCore/Content/AssetPointer.h>
#include <EtCore/Containers/slot_map.h>

#include <Engine/Graphics/TextureData.h>


// forward
class ShaderData;
class Frustum;


namespace render {


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
	T_Hash texDiffuseId = 0u;
	T_Hash texDetail1Id = 0u;
	T_Hash texDetail2Id = 0u;

	T_Hash texHeightId = 0u;
	T_Hash texHeightDetailId = 0u;

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

	TextureData const* GetTexDiffuse() const { return m_TexDiffuse.get(); }
	TextureData const* GetTexDetail1() const { return m_TexDetail1.get(); }
	TextureData const* GetTexDetail2() const { return m_TexDetail2.get(); }
	TextureData const* GetTexHeight() const { return m_TexHeight.get(); }
	TextureData const* GetTexHeightDetail() const { return m_TexHeightDetail.get(); }

	// Data
	///////
private:

	core::T_SlotId m_Node = core::INVALID_SLOT_ID;

	float m_Radius = 0.f;
	float m_MaxHeight = 0.f;

	AssetPtr<TextureData> m_TexDiffuse;
	AssetPtr<TextureData> m_TexDetail1;
	AssetPtr<TextureData> m_TexDetail2;

	AssetPtr<TextureData> m_TexHeight;
	AssetPtr<TextureData> m_TexHeightDetail;

	Triangulator m_Triangulator;
};


} // namespace render
