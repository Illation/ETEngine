#include "stdafx.h"
#include "ModelComponent.h"

#include <EtCore/Reflection/Registration.h>
#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GraphicsTypes/Mesh.h>
#include <EtRendering/MaterialSystem/MaterialInstance.h>
#include <EtRendering/MaterialSystem/MaterialData.h>


namespace et {
namespace fw {


// reflection
//------------

RTTR_REGISTRATION
{
	rttr::registration::class_<ModelComponent>("model component");
	
	BEGIN_REGISTER_POLYMORPHIC_CLASS(ModelComponentDesc, "model comp desc")
		.property("mesh", &ModelComponentDesc::mesh)
		.property("material", &ModelComponentDesc::material)
	END_REGISTER_POLYMORPHIC_CLASS(ModelComponentDesc, I_ComponentDescriptor);
}

// component registration
//------------------------

ECS_REGISTER_COMPONENT(ModelComponent);


//=================
// Model Component 
//=================


//-----------------------
// ModelComponent::c-tor
//
// load assets from ids
//
ModelComponent::ModelComponent(T_Hash const meshId, T_Hash const materialId)
{
	m_Mesh = core::ResourceManager::Instance()->GetAssetData<render::MeshData>(meshId);

	// Load material
	m_Material = core::ResourceManager::Instance()->GetAssetData<render::Material>(materialId, false);
	if (m_Material == nullptr)
	{
		m_Material = core::ResourceManager::Instance()->GetAssetData<render::MaterialInstance>(materialId);
	}
}

//-----------------------
// ModelComponent::c-tor
//
// Construct from preloaded mesh and material
//
ModelComponent::ModelComponent(AssetPtr<render::MeshData> const mesh, I_AssetPtr const material)
	: m_Mesh(mesh)
	, m_Material(material)
{ 
	ET_ASSERT((m_Material.GetType() == typeid(render::Material)) || (m_Material.GetType() == typeid(render::MaterialInstance)));
}


//============================
// Model Component Descriptor
//============================


//------------------------------
// ModelComponentDesc::MakeData
//
// Create a model component from a descriptor
//
ModelComponent* ModelComponentDesc::MakeData()
{
	return new ModelComponent(GetHash(mesh), GetHash(material));
}


} // namespace fw
} // namespace et
