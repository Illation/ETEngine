#include "stdafx.h"
#include "SceneDescriptor.h"

#include <EtCore/Reflection/Serialization.h>


namespace et {
namespace fw {


// reflection
RTTR_REGISTRATION
{
	rttr::registration::class_<EntityDescriptor>("entity descriptor")
		.property("id", &EntityDescriptor::m_Id)
		.property("components", &EntityDescriptor::m_Components)
		.property("children", &EntityDescriptor::m_Children);

	rttr::registration::class_<SceneDescriptor>("scene descriptor")
		.property("entities", &SceneDescriptor::entities)
		.property("skybox", &SceneDescriptor::skybox)
		.property("starfield", &SceneDescriptor::starfield)
		.property("active camera", &SceneDescriptor::activeCamera)
		.property("postprocessing", &SceneDescriptor::postprocessing)
		.property("audio listener", &SceneDescriptor::audioListener)
		.property("gravity", &SceneDescriptor::gravity);

	BEGIN_REGISTER_POLYMORPHIC_CLASS(SceneDescriptorAsset, "scene descriptor asset")
	END_REGISTER_POLYMORPHIC_CLASS(SceneDescriptorAsset, I_Asset);
}
DEFINE_FORCED_LINKING(SceneDescriptorAsset) // force the asset class to be linked as it is only used in reflection


//===================
// Entity Descriptor
//===================


//-------------------------
// EntityDescriptor::c-tor
//
// Default constructor creates a new assigned ID
//
EntityDescriptor::EntityDescriptor() 
	: m_AssignedId(new T_EntityId(m_Id))
{

}

//-------------------------
// EntityDescriptor::c-tor
//
// Ensure copy contructor also creates a new assigned ID
//
EntityDescriptor::EntityDescriptor(EntityDescriptor const& other) 
{
	*this = other;
}

//----------------------
// EntityDescriptor:: =
//
// Ensure copy contructor also creates a new assigned ID
//
EntityDescriptor& EntityDescriptor::operator=(EntityDescriptor const& other)
{
	m_Id = other.GetId();
	m_AssignedId = new T_EntityId(other.GetAssignedId());
	m_Components = other.GetComponents();
	m_Children = other.GetChildren();

	return *this;
}

//-------------------------
// EntityDescriptor::d-tor
//
EntityDescriptor::~EntityDescriptor()
{
	delete m_AssignedId;
}

//========================
// Scene Descriptor Asset
//========================


//---------------------------------------
// SceneDescriptorAsset::LoadFromMemory
//
// Deserialize a scene descriptor
//
bool SceneDescriptorAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	m_Data = new SceneDescriptor();

	// convert that data to a string and deserialize it as json
	if (!serialization::DeserializeFromJsonString(FileUtil::AsText(data), *m_Data))
	{
		LOG("SceneDescriptorAsset::LoadFromMemory > Failed to deserialize descriptor!", LogLevel::Warning);
		delete m_Data;
		m_Data = nullptr;
		return false;
	}

	return true;
}


} // namespace fw
} // namespace et
