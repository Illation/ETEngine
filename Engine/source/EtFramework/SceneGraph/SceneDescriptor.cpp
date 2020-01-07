#include "stdafx.h"
#include "SceneDescriptor.h"

#include <EtCore/Reflection/Serialization.h>


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<fw::EntityDescriptor>("entity descriptor")
		.property("id", &fw::EntityDescriptor::m_Id)
		.property("components", &fw::EntityDescriptor::m_Components)
		.property("children", &fw::EntityDescriptor::m_Children);

	registration::class_<fw::SceneDescriptor>("scene descriptor")
		.property("entities", &fw::SceneDescriptor::entities)
		.property("skybox", &fw::SceneDescriptor::skybox)
		.property("starfield", &fw::SceneDescriptor::starfield)
		.property("active camera", &fw::SceneDescriptor::activeCamera)
		.property("postprocessing", &fw::SceneDescriptor::postprocessing)
		.property("audio listener", &fw::SceneDescriptor::audioListener)
		.property("gravity", &fw::SceneDescriptor::gravity);

	registration::class_<fw::SceneDescriptorAsset>("scene descriptor asset")
		.constructor<fw::SceneDescriptorAsset const&>()
		.constructor<>()(rttr::detail::as_object());
	rttr::type::register_converter_func([](fw::SceneDescriptorAsset& descriptor, bool& ok) -> I_Asset*
	{
		ok = true;
		return new fw::SceneDescriptorAsset(descriptor);
	});
}
DEFINE_FORCED_LINKING(fw::SceneDescriptorAsset) // force the asset class to be linked as it is only used in reflection


namespace fw {


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

