#include <EtFramework/stdafx.h>
#include "EcsTestUtilities.h"

#include <catch2/catch.hpp>
#include <rttr/registration>

#include <thread>
#include <chrono>

#include <mainTesting.h>


TEST_CASE("component view signature", "[ecs]")
{
	fw::ComponentSignature archBCSig = fw::GenSignature<TestBComponent, TestCComponent>();
	fw::ComponentSignature viewBCSig = fw::SignatureFromView<TestBCView>();
	fw::ComponentSignature viewCSig = fw::SignatureFromView<TestCView>();

	REQUIRE(viewBCSig == archBCSig);
	REQUIRE_FALSE(viewCSig == archBCSig);

	REQUIRE(archBCSig.Contains(viewCSig));
	REQUIRE_FALSE(viewCSig.Contains(archBCSig));
}


TEST_CASE("component view", "[ecs]")
{
	size_t const entityCount = 16;

	fw::Archetype arch = GenTestArchetype(entityCount);
	REQUIRE(arch.GetSize() == entityCount);

	// full iteration
	fw::ComponentRange<TestBCView> range(nullptr, &arch, 0u, entityCount);

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
	fw::ComponentRange<TestBCView> halfRange(nullptr, &arch, halved, halved);

	idx = 0u;
	for (TestBCView& view : halfRange)
	{
		REQUIRE(view.c->val == static_cast<uint32>(idx + halved));

		idx++;
	}

	REQUIRE(idx == halved);
}
