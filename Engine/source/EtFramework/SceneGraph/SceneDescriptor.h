#pragma once
#include "ComponentDescriptor.h"
#include "EntityLink.h"

#include <rttr/type>
#include <rttr/registration_friend.h>

#include <EtCore/Content/Asset.h>
#include <EtCore/Helper/LinkerUtils.h>

#include <EtRendering/GraphicsTypes/PostProcessingSettings.h>


namespace fw {


//-----------------------
// EntityDescriptor
//
// Contains all component descriptions for an entity
//
class EntityDescriptor final
{
	// definitions
	//-------------
	RTTR_ENABLE()
	RTTR_REGISTRATION_FRIEND

	// construct destruct
	//--------------------
public:
	EntityDescriptor();
	EntityDescriptor(EntityDescriptor const& other);
	EntityDescriptor& operator=(EntityDescriptor const& other);
	~EntityDescriptor();

	// modifiers
	//-----------
	void SetAssignedId(T_EntityId const val) const { *m_AssignedId = val; }

	// accessors
	//-----------
	T_EntityId GetId() const { return m_Id; }
	T_EntityId GetAssignedId() const { return *m_AssignedId; }
	std::vector<I_ComponentDescriptor*> const& GetComponents() const { return m_Components; }
	std::vector<EntityDescriptor> const& GetChildren() const { return m_Children; }

	// Data
	///////

private:
	T_EntityId m_Id = INVALID_ENTITY_ID;
	T_EntityId* m_AssignedId = nullptr; // use pointer in order to allow const modification
	std::vector<I_ComponentDescriptor*> m_Components;
	std::vector<EntityDescriptor> m_Children;
};


//-----------------------
// EntityDescriptor
//
// Data to serialize / deserialize a scene with
//
struct SceneDescriptor final
{
	// definitions
	//-------------
	RTTR_ENABLE()

	// Data
	///////

public:
	// entities
	std::vector<EntityDescriptor> entities;

	// graphical parameters
	std::string skybox;
	std::string starfield;
	EntityLink activeCamera;
	PostProcessingSettings postprocessing;

	// audio parameters
	EntityLink audioListener;

	// physics parameters
	vec3 gravity;
};


//---------------------------------
// SceneDescriptorAsset
//
// Asset data to load a scene
//
class SceneDescriptorAsset final : public Asset<SceneDescriptor, false>
{
	// definitions
	//-------------
	DECLARE_FORCED_LINKING()
	RTTR_ENABLE(Asset<SceneDescriptor, false>)

	// Construct destruct
	//---------------------
public:
	SceneDescriptorAsset() : Asset<SceneDescriptor, false>() {}
	virtual ~SceneDescriptorAsset() = default;

	// Asset interface
	//---------------------
	bool LoadFromMemory(std::vector<uint8> const& data) override;
};


} // namespace fw

