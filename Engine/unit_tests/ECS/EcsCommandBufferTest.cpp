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

TEST_CASE("command buffer add / duplicate / remove", "[ecs]")
{
	fw::EcsController ecs;

	// generate entities
	fw::T_EntityId ent0 = ecs.AddEntity(TestCComponent(fw::INVALID_ENTITY_ID));
	REQUIRE(ent0 == 0u);

	class TestDuplicateSystem final : public fw::System<TestDuplicateSystem, TestCView>
	{
	public:
		TestDuplicateSystem() = default;
		// base class constructors don't do anything so are not needed unless we declare dependencies or init lookup variables

		void Process(fw::ComponentRange<TestCView>& range)
		{
			fw::EcsCommandBuffer& cb = GetCommandBuffer();

			for (TestCView& view : range)
			{
				cb.DuplicateEntity(view.GetCurrentEntity());
			}
		}
	};

	ecs.RegisterSystem<TestDuplicateSystem>();

	ecs.Process();

	REQUIRE(ecs.GetEntityCount() == 2u);
	REQUIRE(ecs.HasComponent<TestCComponent>(1u));
	REQUIRE(ecs.GetComponent<TestCComponent>(1u).val == fw::INVALID_ENTITY_ID);


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

	ecs.UnregisterSystem<TestDuplicateSystem>();
	ecs.RegisterSystem<TestAddSystem>();

	ecs.Process();

	REQUIRE(ecs.GetEntityCount() == 4u);

	fw::T_EntityId addEnt = ecs.GetComponent<TestCComponent>(ent0).val;
	REQUIRE(addEnt != fw::INVALID_ENTITY_ID);

	REQUIRE(ecs.HasComponent<TestAComponent>(addEnt));
	REQUIRE(ecs.HasComponent<TestBComponent>(addEnt));
	REQUIRE(ecs.HasComponent<TestCComponent>(addEnt));

	fw::T_EntityId addEnt2 = ecs.GetComponent<TestCComponent>(1u).val;
	REQUIRE(addEnt2 != fw::INVALID_ENTITY_ID);

	REQUIRE(ecs.HasComponent<TestAComponent>(addEnt2));
	REQUIRE(ecs.HasComponent<TestBComponent>(addEnt2));
	REQUIRE(ecs.HasComponent<TestCComponent>(addEnt2));

	class TestRemoveASystem final : public fw::System<TestRemoveASystem, TestBCView>
	{
	public:
		TestRemoveASystem() = default;
		// base class constructors don't do anything so are not needed unless we declare dependencies or init lookup variables

		void Process(fw::ComponentRange<TestBCView>& range)
		{
			fw::EcsCommandBuffer& cb = GetCommandBuffer();

			for (TestBCView& view : range)
			{
				cb.RemoveComponents<TestAComponent>(view.GetCurrentEntity());
			}
		}
	};

	ecs.UnregisterSystem<TestAddSystem>();
	ecs.RegisterSystem<TestRemoveASystem>();

	ecs.Process();
	
	REQUIRE_FALSE(ecs.HasComponent<TestAComponent>(addEnt));
	REQUIRE(ecs.HasComponent<TestBComponent>(addEnt));
	REQUIRE(ecs.HasComponent<TestCComponent>(addEnt));

	REQUIRE_FALSE(ecs.HasComponent<TestAComponent>(addEnt2));
	REQUIRE(ecs.HasComponent<TestBComponent>(addEnt2));
	REQUIRE(ecs.HasComponent<TestCComponent>(addEnt2));

	class TestRemoveSystem final : public fw::System<TestRemoveSystem, TestBCView>
	{
	public:
		TestRemoveSystem() = default;
		// base class constructors don't do anything so are not needed unless we declare dependencies or init lookup variables

		void Process(fw::ComponentRange<TestBCView>& range)
		{
			fw::EcsCommandBuffer& cb = GetCommandBuffer();

			for (TestBCView& view : range)
			{
				cb.RemoveEntity(view.GetCurrentEntity());
			}
		}
	};

	ecs.UnregisterSystem<TestRemoveASystem>();
	ecs.RegisterSystem<TestRemoveSystem>();

	ecs.Process();

	REQUIRE(ecs.GetEntityCount() == 2u);
}


