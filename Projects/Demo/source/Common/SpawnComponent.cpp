#include "stdafx.h"
#include "SpawnComponent.h"

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

	registration::class_<demo::SpawnComponent>("spawn component");

	registration::class_<demo::SpawnComponentDesc>("spawn comp desc")
		.constructor<demo::SpawnComponentDesc const&>()
		.constructor<>()(rttr::detail::as_object())
		.property("mesh", &demo::SpawnComponentDesc::mesh)
		.property("material", &demo::SpawnComponentDesc::material)
		.property("scale", &demo::SpawnComponentDesc::scale)
		.property("shape", &demo::SpawnComponentDesc::shape)
		.property("mass", &demo::SpawnComponentDesc::mass)
		.property("interval", &demo::SpawnComponentDesc::interval)
		.property("impulse", &demo::SpawnComponentDesc::impulse);

	rttr::type::register_converter_func([](demo::SpawnComponentDesc& descriptor, bool& ok) -> fw::I_ComponentDescriptor*
	{
		ok = true;
		return new demo::SpawnComponentDesc(descriptor);
	});
}

DEFINE_FORCED_LINKING(demo::SpawnComponentDesc) // force the linker to include this unit

// component registration
//------------------------

ECS_REGISTER_COMPONENT(demo::SpawnComponent);



namespace demo {


//=================
// Spawn Component 
//=================


//-----------------------
// SpawnComponent::c-tor
//
// load assets from ids
//
SpawnComponent::SpawnComponent(T_Hash const meshId,
	T_Hash const materialId,
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
	mesh = ResourceManager::Instance()->GetAssetData<MeshData>(meshId);

	// Load material
	material = ResourceManager::Instance()->GetAssetData<render::Material>(materialId, false);
	if (material == nullptr)
	{
		material = ResourceManager::Instance()->GetAssetData<render::MaterialInstance>(materialId);
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
	return new SpawnComponent(GetHash(mesh), GetHash(material), scale, shape->MakeBulletCollisionShape(), mass, interval, impulse);
}


} // namespace demo
