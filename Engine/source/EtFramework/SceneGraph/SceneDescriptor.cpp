#include "stdafx.h"
#include "SceneDescriptor.h"

#include <EtCore/Reflection/Serialization.h>


//========================
// Scene Descriptor Asset
//========================


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<framework::EntityDescriptor>("entity descriptor")
		.property("id", &framework::EntityDescriptor::m_Id)
		.property("components", &framework::EntityDescriptor::m_Components)
		.property("children", &framework::EntityDescriptor::m_Children);

	registration::class_<framework::SceneDescriptor>("scene descriptor")
		.property("entities", &framework::SceneDescriptor::entities)
		.property("skybox", &framework::SceneDescriptor::skybox)
		.property("starfield", &framework::SceneDescriptor::starfield)
		.property("active camera", &framework::SceneDescriptor::activeCamera)
		.property("postprocessing", &framework::SceneDescriptor::postprocessing)
		.property("audio listener", &framework::SceneDescriptor::audioListener)
		.property("gravity", &framework::SceneDescriptor::gravity);

	registration::class_<framework::SceneDescriptorAsset>("scene descriptor asset")
		.constructor<framework::SceneDescriptorAsset const&>()
		.constructor<>()(rttr::detail::as_object());
	rttr::type::register_converter_func([](framework::SceneDescriptorAsset& descriptor, bool& ok) -> I_Asset*
	{
		ok = true;
		return new framework::SceneDescriptorAsset(descriptor);
	});
}
DEFINE_FORCED_LINKING(framework::SceneDescriptorAsset) // force the asset class to be linked as it is only used in reflection


namespace framework {


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


} // namespace framework

