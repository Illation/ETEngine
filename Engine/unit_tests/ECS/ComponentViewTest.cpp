#include <EtFramework/stdafx.h>
#include "TestComponentViews.h"

#include <catch2/catch.hpp>
#include <rttr/registration>

#include <thread>
#include <chrono>

#include <mainTesting.h>

// components should already have been registered in ComponentRegistryTest.cpp

framework::Archetype GenTestArchetype(size_t const count)
{
	framework::Archetype arch(framework::GenSignature<TestBComponent, TestCComponent>());

	for (size_t idx = 0u; idx < count; ++idx)
	{
		arch.AddEntity(static_cast<framework::T_EntityId>(idx), {
			framework::MakeRawComponent(TestBComponent(std::to_string(idx))),
			framework::MakeRawComponent(TestCComponent(static_cast<uint32>(idx)))
			});
	}

	return arch;
}


TEST_CASE("component view signature", "[ecs]")
{
	framework::ComponentSignature archBCSig = framework::GenSignature<TestBComponent, TestCComponent>();
	framework::ComponentSignature viewBCSig = framework::SignatureFromView<TestBCView>();
	framework::ComponentSignature viewCSig = framework::SignatureFromView<TestCView>();

	REQUIRE(viewBCSig == archBCSig);
	REQUIRE_FALSE(viewCSig == archBCSig);

	REQUIRE(archBCSig.Contains(viewCSig));
	REQUIRE_FALSE(viewCSig.Contains(archBCSig));
}


TEST_CASE("component view", "[ecs]")
{
	size_t const entityCount = 16;

	framework::Archetype arch = GenTestArchetype(entityCount);
	REQUIRE(arch.GetSize() == entityCount);

	// full iteration
	framework::ComponentRange<TestBCView> range(&arch, 0u, entityCount);

	size_t idx = 0u;
	for (TestBCView& view : range)
	{
		TestCComponent const& cComp = *(view.c);
		REQUIRE(cComp.val == static_cast<uint32>(idx));

		idx++;
	}

	REQUIRE(idx == entityCount);

	// iteration starting from an offset (8 - 15)
	size_t const halved = entityCount / 2u;
	framework::ComponentRange<TestBCView> halfRange(&arch, halved, halved);

	idx = 0u;
	for (TestBCView& view : halfRange)
	{
		REQUIRE(view.c->val == static_cast<uint32>(idx + halved));

		idx++;
	}

	REQUIRE(idx == halved);
}
