#include <EtFramework/stdafx.h>
#include "EcsTestUtilities.h"

#include <catch2/catch.hpp>
#include <rttr/registration>

#include <thread>
#include <chrono>

#include <mainTesting.h>


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

	systemBC.RootProcess(nullptr, &arch, 0u, entityCount);
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
	sys.RootProcess(nullptr, &arch, 0u, overwriteEnd);

	// validate that the system changed the components
	struct COverwriteReadOnlyView final : public framework::ComponentView
	{
		COverwriteReadOnlyView() : framework::ComponentView()
		{
			Declare(o);
			Declare(c);
		}

		ReadAccess<TestOverwriteComp> o;
		ReadAccess<TestCComponent> c;
	};

	framework::ComponentRange<COverwriteReadOnlyView> range(nullptr, &arch, 0u, entityCount);

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
