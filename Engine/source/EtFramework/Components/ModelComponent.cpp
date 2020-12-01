#include "stdafx.h"
#include "ModelComponent.h"

#include <EtCore/Reflection/Registration.h>
#include <EtCore/Content/ResourceManager.h>
#include <EtRendering/MaterialSystem/MaterialInstance.h>
#include <EtRendering/MaterialSystem/MaterialData.h>


namespace et {
namespace fw {


// reflection
//------------

RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(ModelComponent, "model component")
		.property("mesh", &ModelComponent::m_Mesh)
		.property("material", &ModelComponent::m_Material)
	END_REGISTER_CLASS_POLYMORPHIC(ModelComponent, I_ComponentDescriptor);
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
// Construct from preloaded mesh and material
//
ModelComponent::ModelComponent(AssetPtr<render::MeshData> const mesh, AssetPtr<render::I_Material> const material)
	: m_Mesh(mesh)
	, m_Material(material)
{ 
	ET_ASSERT((m_Material.GetType() == rttr::type::get<render::Material>()) || (m_Material.GetType() == rttr::type::get<render::MaterialInstance>()));
}


} // namespace fw
} // namespace et
