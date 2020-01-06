#include <EtFramework/stdafx.h>
#include "EcsTestUtilities.h"

#include <catch2/catch.hpp>
#include <rttr/registration>


// reflection
//------------

RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<TestAComponent>("test A component")
		.property("x", &TestAComponent::x)
		.property("y", &TestAComponent::y);

	registration::class_<TestBComponent>("test B component")
		.property("name", &TestBComponent::name);

	registration::class_<TestCComponent>("test C component")
		.property("value", &TestCComponent::val);

	registration::class_<TestOverwriteComp>("test overwrite component")
		.property("overwritten", &TestOverwriteComp::overwritten);

	registration::class_<TestRefCountComp>("test ref count component")
		.property("pointer", &TestRefCountComp::ptr);
}

// component registration
//------------------------

ECS_REGISTER_COMPONENT(TestAComponent);
ECS_REGISTER_COMPONENT(TestBComponent);
ECS_REGISTER_COMPONENT(TestCComponent);
ECS_REGISTER_COMPONENT(TestOverwriteComp);
ECS_REGISTER_COMPONENT(TestRefCountComp);

// utility
//---------

fw::Archetype GenTestArchetype(size_t const count)
{
	fw::Archetype arch(fw::GenSignature<TestBComponent, TestCComponent>());

	for (size_t idx = 0u; idx < count; ++idx)
	{
		arch.AddEntity(static_cast<fw::T_EntityId>(idx), {
			fw::MakeRawComponent(TestBComponent(std::to_string(idx))),
			fw::MakeRawComponent(TestCComponent(static_cast<uint32>(idx)))
			});
	}

	return arch;
}

// systems
//---------

void TestBCSystem::Process(fw::ComponentRange<TestBCView>& range) const
{
	size_t idx = 0u;

	for (TestBCView& view : range)
	{
		REQUIRE(view.b->name == std::to_string(idx));
		REQUIRE(view.c->val == static_cast<uint32>(idx));

		idx++;
	}
}


void TestOverwriteSystem::Process(fw::ComponentRange<TestOverwriteSystemView>& range) const
{
	for (TestOverwriteSystemView& view : range)
	{
		if (view.c->val >= m_OverwriteMin)
		{
			view.overwrite->overwritten = true;
		}
	}
}
