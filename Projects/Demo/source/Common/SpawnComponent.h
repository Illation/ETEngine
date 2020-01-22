#pragma once
#include <EtCore/Content/AssetPointer.h>
#include <EtCore/Containers/slot_map.h>

#include <EtFramework/SceneGraph/ComponentDescriptor.h>
#include <EtFramework/Physics/CollisionShape.h>


class btCollisionShape;
namespace et { namespace render {
	class MeshData;
} }


namespace et {
namespace demo {


//---------------------------------
// SpawnComponent
//
// Component containing data to spawn entities with model and rigid body components
//
struct SpawnComponent final
{
	// definitions
	//-------------
	ECS_DECLARE_COMPONENT

	// construct destruct
	//--------------------
public:
	SpawnComponent(T_Hash const meshId, 
		T_Hash const materialId, 
		float const s, 
		btCollisionShape* const shape, 
		float const shapeMass, 
		float const interv, 
		float const imp);
	~SpawnComponent() = default;

	float interval = 0.f;
	float cooldown = 0.f;
	float impulse = 0.f;

	// hold the assets so that they are loaded in already when spawning
	AssetPtr<render::MeshData> mesh;
	I_AssetPtr material;
	float scale = 1.f;

	btCollisionShape* collisionShape = nullptr;
	float mass = 1.f;
};


//---------------------------------
// SpawnComponentDesc
//
// Descriptor for serialization and deserialization of spawn components
//
class SpawnComponentDesc final : public fw::ComponentDescriptor<SpawnComponent>
{
	// definitions
	//-------------
	RTTR_ENABLE(ComponentDescriptor<SpawnComponent>)
	DECLARE_FORCED_LINKING()

	// construct destruct
	//--------------------
public:
	SpawnComponentDesc() : ComponentDescriptor<SpawnComponent>() {}

	SpawnComponentDesc& operator=(SpawnComponentDesc const& other);
	SpawnComponentDesc(SpawnComponentDesc const& other);
	~SpawnComponentDesc();

	// ComponentDescriptor interface
	//-------------------------------
	SpawnComponent* MakeData() override;

	// Data
	///////

	std::string mesh;
	std::string material;
	float scale = 1.f;

	fw::CollisionShape* shape = nullptr;
	float mass = 1.f;

	float interval = 1.f;
	float impulse = 0.f;
};


} // namespace demo
} // namespace et
