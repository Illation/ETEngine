#include <EtFramework/stdafx.h>
#include "TestComponentViews.h"

#include <catch2/catch.hpp>
#include <rttr/registration>

#include <thread>
#include <chrono>

#include <mainTesting.h>

#include <EtFramework/ECS/System.h>

// components should already have been registered in ComponentRegistryTest.cpp


class TestBCSystem final : public framework::System<TestBCSystem, TestBCView>
{
public:
	TestBCSystem() = default;
	// base class constructors don't do anything so are not needed unless we declare dependencies or init lookup variables

	void Process(framework::ComponentRange<TestBCView>& range) const override
	{
		size_t idx = 0u;

		for (TestBCView& view : range)
		{
			REQUIRE(view.b->name == std::to_string(idx));
			REQUIRE(view.c->val == static_cast<uint32>(idx));

			idx++;
		}
	}
};

//======================
// overwrite component
//======================

struct TestOverwriteComp final
{
	ECS_DECLARE_COMPONENT
public:

	TestOverwriteComp() = default;

	bool overwritten = false;
};

RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<TestOverwriteComp>("test overwrite component")
		.property("overwritten", &TestOverwriteComp::overwritten);
}

ECS_REGISTER_COMPONENT(TestOverwriteComp);

//======================
// overwrite system
//======================

struct TestOverwriteSystemView final : public framework::ComponentView
{
	WriteAccess<TestOverwriteComp> overwrite;
	ReadAccess<TestCComponent> c;

	void Register() override
	{
		Declare(overwrite);
		Declare(c);
	}
};

class TestOverwriteSystem final : public framework::System<TestOverwriteSystem, TestOverwriteSystemView>
{
public:
	TestOverwriteSystem(size_t const min) 
		: m_OverwriteMin(min)
	{
		DeclareDependents<TestBCSystem>();
	}

	void Process(framework::ComponentRange<TestOverwriteSystemView>& range) const override
	{
		for (TestOverwriteSystemView& view : range)
		{
			if (view.c->val >= m_OverwriteMin)
			{
				view.overwrite->overwritten = true;
			}
		}
	}

private:
	size_t const m_OverwriteMin = 0u;
};

//======================
// tests
//======================

TEST_CASE("system dependency", "[ecs]")
{
	TestOverwriteSystem sys(0u);
	framework::T_DependencyList const& dependencies = sys.GetDependencies();
	framework::T_DependencyList const& dependents = sys.GetDependents();

	REQUIRE(dependencies.size() == 0u);
	REQUIRE(dependents.size() == 1u);
	REQUIRE(dependents[0] == rttr::type::get<TestBCSystem>().get_id());
}

TEST_CASE("system iterate", "[ecs]")
{
	size_t const entityCount = 16;

	framework::Archetype arch = GenTestArchetype(entityCount);
	REQUIRE(arch.GetSize() == entityCount);

	TestBCSystem systemBC;
	REQUIRE(systemBC.GetSignature() == framework::GenSignature<TestCComponent, TestBComponent>());

	REQUIRE(arch.GetSignature().Contains(systemBC.GetSignature()));

	systemBC.RootProcess(&arch, 0u, entityCount);
}

TEST_CASE("system overwrite", "[ecs]")
{
	size_t const entityCount = 16u;
	size_t const overwriteMin = 4u;
	size_t const overwriteEnd = 12u;

	// generate entities
	framework::Archetype arch(framework::GenSignature<TestOverwriteComp, TestCComponent>());

	for (uint32 idx = 0u; idx < static_cast<uint32>(entityCount); ++idx)
	{
		framework::T_EntityId const entity = static_cast<framework::T_EntityId>(idx);
		arch.AddEntity(entity, { framework::MakeRawComponent(TestOverwriteComp()), framework::MakeRawComponent(TestCComponent(idx)) });
	}

	// create and run the overwrite system
	TestOverwriteSystem sys(overwriteMin);
	sys.RootProcess(&arch, 0u, overwriteEnd);

	// validate that the system changed the components
	struct COverwriteReadOnlyView final : public framework::ComponentView
	{
		ReadAccess<TestOverwriteComp> o;
		ReadAccess<TestCComponent> c;

		void Register() override
		{
			Declare(o);
			Declare(c);
		}
	};

	framework::ComponentRange<COverwriteReadOnlyView> range(&arch, 0u, entityCount);

	size_t idx = 0u;
	for (COverwriteReadOnlyView& view : range)
	{
		if (idx < overwriteEnd)
		{
			if (view.c->val >= overwriteMin)
			{
				REQUIRE(view.o->overwritten);
			}
			else
			{
				REQUIRE_FALSE(view.o->overwritten);
			}
		}
		else
		{
			REQUIRE_FALSE(view.o->overwritten);
		}

		idx++;
	}
}
