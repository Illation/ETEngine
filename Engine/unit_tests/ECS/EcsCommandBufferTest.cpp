#include <EtFramework/stdafx.h>
#include "EcsTestUtilities.h"

#include <catch2/catch.hpp>
#include <rttr/registration>

#include <thread>
#include <chrono>

#include <mainTesting.h>

#include <EtFramework/ECS/EcsController.h>


TEST_CASE("command buffer reparent", "[ecs]")
{
	fw::EcsController ecs;

	// generate entities
	fw::T_EntityId ent0 = ecs.AddEntity(TestBComponent(""));
	fw::T_EntityId ent1 = ecs.AddEntity(TestCComponent(ent0));

	class TestCReparentSystem final : public fw::System<TestCReparentSystem, TestCView>
	{
	public:
		TestCReparentSystem() = default;
		// base class constructors don't do anything so are not needed unless we declare dependencies or init lookup variables

		void Process(fw::ComponentRange<TestCView>& range)
		{
			fw::EcsCommandBuffer& cb = GetCommandBuffer();

			for (TestCView& view : range)
			{
				cb.ReparentEntity(view.GetCurrentEntity(), view.c->val);
			}
		}
	};

	ecs.RegisterSystem<TestCReparentSystem>();

	ecs.Process();

	REQUIRE(ecs.GetEntityCount() == 2u);
	REQUIRE(ecs.GetParent(ent1) == ent0);
	REQUIRE(ecs.GetChildren(ent0).size() == 1u);
	REQUIRE(ecs.GetChildren(ent0)[0] == ent1);
}

TEST_CASE("command buffer add", "[ecs]")
{
	fw::EcsController ecs;

	// generate entities
	fw::T_EntityId ent0 = ecs.AddEntity(TestCComponent(fw::INVALID_ENTITY_ID));
	fw::T_EntityId ent1 = ecs.AddEntity(TestCComponent(fw::INVALID_ENTITY_ID));


	class TestAddSystem final : public fw::System<TestAddSystem, TestCWriteView>
	{
	public:
		TestAddSystem() = default;
		// base class constructors don't do anything so are not needed unless we declare dependencies or init lookup variables

		void Process(fw::ComponentRange<TestCWriteView>& range)
		{
			fw::EcsCommandBuffer& cb = GetCommandBuffer();

			for (TestCWriteView& view : range)
			{
				view.c->val = cb.AddEntity();

				cb.AddComponents(view.c->val, TestAComponent(), TestBComponent(), TestCComponent());
			}
		}
	};

	ecs.RegisterSystem<TestAddSystem>();

	ecs.Process();

	REQUIRE(ecs.GetEntityCount() == 4u);

	fw::T_EntityId addEnt = ecs.GetComponent<TestCComponent>(ent0).val;
	REQUIRE(addEnt != fw::INVALID_ENTITY_ID);

	REQUIRE(ecs.HasComponent<TestAComponent>(addEnt));
	REQUIRE(ecs.HasComponent<TestBComponent>(addEnt));
	REQUIRE(ecs.HasComponent<TestCComponent>(addEnt));

	fw::T_EntityId addEnt2 = ecs.GetComponent<TestCComponent>(ent1).val;
	REQUIRE(addEnt2 != fw::INVALID_ENTITY_ID);

	REQUIRE(ecs.HasComponent<TestAComponent>(addEnt2));
	REQUIRE(ecs.HasComponent<TestBComponent>(addEnt2));
	REQUIRE(ecs.HasComponent<TestCComponent>(addEnt2));
}


