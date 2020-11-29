#include "stdafx.h"
#include "SpawnComponent.h"

#include <EtCore/Reflection/Registration.h>
#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GraphicsTypes/Mesh.h>
#include <EtRendering/MaterialSystem/MaterialInstance.h>
#include <EtRendering/MaterialSystem/MaterialData.h>


namespace et {
namespace demo {


// reflection
//------------

RTTR_REGISTRATION
{
	rttr::registration::class_<SpawnComponent>("spawn component");

	BEGIN_REGISTER_CLASS(SpawnComponentDesc, "spawn comp desc")
		.property("mesh", &SpawnComponentDesc::mesh)
		.property("material", &SpawnComponentDesc::material)
		.property("scale", &SpawnComponentDesc::scale)
		.property("shape", &SpawnComponentDesc::shape)
		.property("mass", &SpawnComponentDesc::mass)
		.property("interval", &SpawnComponentDesc::interval)
		.property("impulse", &SpawnComponentDesc::impulse)
	END_REGISTER_CLASS_POLYMORPHIC(SpawnComponentDesc, fw::I_ComponentDescriptor);
}
DEFINE_FORCED_LINKING(SpawnComponentDesc) // force the linker to include this unit

ECS_REGISTER_COMPONENT(SpawnComponent);


//=================
// Spawn Component 
//=================


//-----------------------
// SpawnComponent::c-tor
//
// load assets from ids
//
SpawnComponent::SpawnComponent(core::HashString const meshId,
	core::HashString const materialId,
	float const s,
	btCollisionShape* const shape,
	float const shapeMass,
	float const interv,
	float const imp
)
	: collisionShape(shape)
	, mass(shapeMass)
	, scale(s)
	, interval(interv)
	, impulse(imp)
{
	mesh = core::ResourceManager::Instance()->GetAssetData<render::MeshData>(meshId);

	// Load material
	material = core::ResourceManager::Instance()->GetAssetData<render::Material>(materialId, false);
	if (material == nullptr)
	{
		material = core::ResourceManager::Instance()->GetAssetData<render::MaterialInstance>(materialId);
	}
}


//============================
// Spawn Component Descriptor
//============================


//-----------------------------
// SpawnComponentDesc:: = 
//
SpawnComponentDesc& SpawnComponentDesc::operator=(SpawnComponentDesc const& other)
{
	mesh = other.mesh;
	material = other.material;
	scale = other.scale;
	mass = other.mass;
	interval = other.interval;
	impulse = other.impulse;

	delete shape;
	shape = nullptr;
	if (other.shape != nullptr)
	{
		shape = other.shape->Clone();
	}

	return *this;
}

//-------------------------------
// SpawnComponentDesc::c-tor
//
SpawnComponentDesc::SpawnComponentDesc(SpawnComponentDesc const& other)
{
	*this = other;
}

//-------------------------------
// SpawnComponentDesc::d-tor
//
SpawnComponentDesc::~SpawnComponentDesc()
{
	delete shape;
}


//------------------------------
// SpawnComponentDesc::MakeData
//
// Create a spawn component from a descriptor
//
SpawnComponent* SpawnComponentDesc::MakeData()
{
	return new SpawnComponent(mesh, material, scale, shape->MakeBulletCollisionShape(), mass, interval, impulse);
}


} // namespace demo
} // namespace et
