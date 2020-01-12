#include "stdafx.h"
#include "ModelComponent.h"

#include <rttr/registration>

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GraphicsTypes/Mesh.h>
#include <EtRendering/MaterialSystem/MaterialInstance.h>
#include <EtRendering/MaterialSystem/MaterialData.h>


// reflection
//------------

RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<fw::ModelComponent>("model component");

	registration::class_<fw::ModelComponentDesc>("model comp desc")
		.constructor<fw::ModelComponentDesc const&>()
		.constructor<>()(rttr::detail::as_object())
		.property("mesh", &fw::ModelComponentDesc::mesh)
		.property("material", &fw::ModelComponentDesc::material);

	rttr::type::register_converter_func([](fw::ModelComponentDesc& descriptor, bool& ok) -> fw::I_ComponentDescriptor*
	{
		ok = true;
		return new fw::ModelComponentDesc(descriptor);
	});
}

// component registration
//------------------------

ECS_REGISTER_COMPONENT(fw::ModelComponent);


namespace fw {


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
	m_Mesh = ResourceManager::Instance()->GetAssetData<MeshData>(meshId);

	// Load material
	m_Material = ResourceManager::Instance()->GetAssetData<render::Material>(materialId, false);
	if (m_Material == nullptr)
	{
		m_Material = ResourceManager::Instance()->GetAssetData<render::MaterialInstance>(materialId);
	}
}

//-----------------------
// ModelComponent::c-tor
//
// Construct from preloaded mesh and material
//
ModelComponent::ModelComponent(AssetPtr<MeshData> const mesh, I_AssetPtr const material)
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
