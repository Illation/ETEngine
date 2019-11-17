#pragma once
#include "SceneExtension.h"

#include <EtCore/Content/AssetPointer.h>
#include <EtCore/Containers/slot_map.h>

#include <EtRendering/SceneStructure/MaterialCollection.h>


class MeshData;


//---------------------------------
// OutlineExtension
//
// Additional scene data to draw outlines with
//
class OutlineExtension final : public render::I_SceneExtension
{
public:
	//---------------------------------
	// OutlineExtension::OutlineList
	//
	// Lists that we can draw as a single unit without updating shader data (apart from model matricies)
	//
	struct OutlineList
	{
		vec4 color;
		std::vector<render::MaterialCollection::Mesh> meshes;
	};

	typedef std::vector<OutlineList> T_OutlineLists;

	// construct destruct
	//--------------------
	OutlineExtension() : I_SceneExtension() {}
	~OutlineExtension() = default;

	// I_SceneExtension interface
	//----------------------------
	T_Hash GetId() const override { return "OutlineExtension"_hash; }

	// functionality
	//---------------
	void SetColor(vec4 const& col) { m_Color = col; }
	void AddMesh(AssetPtr<MeshData> const mesh, render::T_NodeId const node);
	void Clear() { m_Lists.clear(); }

	// accessors
	//-----------
	T_OutlineLists const& GetOutlineLists() const { return m_Lists; }

	// Data
	///////
private:

	vec4 m_Color;
	T_OutlineLists m_Lists;
};

