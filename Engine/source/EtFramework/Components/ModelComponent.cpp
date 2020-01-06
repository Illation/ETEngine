#include "stdafx.h"
#include "ModelComponent.h"

#include "TransformComponent.h"

#include <rttr/registration>

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GraphicsTypes/Mesh.h>
#include <EtRendering/MaterialSystem/MaterialInstance.h>
#include <EtRendering/MaterialSystem/MaterialData.h>

#include <EtFramework/SceneGraph/Entity.h>
#include <EtFramework/SceneGraph/SceneManager.h>


// deprecated
//------------


//=================
// Model Component
//=================


//---------------------------------
// ModelComponent::c-tor
//
// default constructor sets the asset ID
//
ModelComponent::ModelComponent(T_Hash const meshId, T_Hash const materialId)
	: m_MeshId(meshId)
	, m_MaterialId(materialId)
{ }

//---------------------------------
// ModelComponent::Init
//
// Loads the mesh data
//
void ModelComponent::Init()
{
	// Load mesh
	m_Mesh = ResourceManager::Instance()->GetAssetData<MeshData>(m_MeshId);

	// Load material
	m_Material = ResourceManager::Instance()->GetAssetData<render::Material>(m_MaterialId, false);
	if (m_Material == nullptr)
	{
		m_Material = ResourceManager::Instance()->GetAssetData<render::MaterialInstance>(m_MaterialId);
	}

	render::I_Material const* const mat = m_Material.get_as<render::I_Material>();

	// register
	m_InstanceId = SceneManager::GetInstance()->GetRenderScene().AddInstance(mat, m_Mesh, GetTransform()->GetNodeId());
}

//---------------------------------
// ModelComponent::Deinit
//
void ModelComponent::Deinit()
{
	SceneManager::GetInstance()->GetRenderScene().RemoveInstance(m_InstanceId);
}


// reflection
//------------

RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<framework::ModelComponent>("model component");

	registration::class_<framework::ModelComponentDesc>("model comp desc")
		.constructor<framework::ModelComponentDesc const&>()
		.constructor<>()(rttr::detail::as_object())
		.property("mesh", &framework::ModelComponentDesc::mesh)
		.property("material", &framework::ModelComponentDesc::material);

	rttr::type::register_converter_func([](framework::ModelComponentDesc& descriptor, bool& ok) -> framework::I_ComponentDescriptor*
	{
		ok = true;
		return new framework::ModelComponentDesc(descriptor);
	});
}

// component registration
//------------------------

ECS_REGISTER_COMPONENT(framework::ModelComponent);


namespace framework {


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


} // namespace framework
