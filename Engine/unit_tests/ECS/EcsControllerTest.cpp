#include <EtFramework/stdafx.h>
#include "EcsTestUtilities.h"

#include <catch2/catch.hpp>
#include <rttr/registration>

#include <thread>
#include <chrono>

#include <mainTesting.h>

#include <EtFramework/ECS/EcsController.h>


TEST_CASE("controller entity and component creation", "[ecs]")
{
	framework::EcsController ecs;
	REQUIRE(ecs.GetEntityCount() == 0u);

	framework::T_EntityId const ent0 = ecs.AddEntity();
	framework::T_EntityId const ent1 = ecs.AddEntity(TestAComponent(), TestBComponent("wowzer"), TestCComponent(9001));
	REQUIRE(ecs.GetEntityCount() == 2u);

	REQUIRE(ent0 == 0u);
	REQUIRE(ent1 == 1u);

	REQUIRE(ecs.GetComponentTypes(ent0).size() == 0u);
	REQUIRE(ecs.GetComponentTypes(ent1).size() == 3u);

	REQUIRE_FALSE(ecs.HasComponent<TestAComponent>(ent0));
	REQUIRE(ecs.HasComponent<TestAComponent>(ent1));
	REQUIRE(ecs.HasComponent<TestBComponent>(ent1));
	REQUIRE(ecs.HasComponent<TestCComponent>(ent1));

	TestAComponent addAComp;
	addAComp.x = 1;
	ecs.AddComponents(ent0, addAComp);

	REQUIRE(ecs.GetComponentTypes(ent0).size() == 1u);
	REQUIRE(ecs.HasComponent<TestAComponent>(ent0));
	REQUIRE(ecs.GetComponent<TestAComponent>(ent0).x == 1);

	ecs.GetComponent<TestAComponent>(ent0).x = 2;
	REQUIRE(ecs.GetComponent<TestAComponent>(ent0).x == 2);

	ecs.RemoveComponents<TestAComponent, TestBComponent>(ent1);
	REQUIRE(ecs.GetComponentTypes(ent1).size() == 1u);
	REQUIRE_FALSE(ecs.HasComponent<TestAComponent>(ent1));
	REQUIRE_FALSE(ecs.HasComponent<TestBComponent>(ent1));
	REQUIRE(ecs.HasComponent<TestCComponent>(ent1));

	ecs.RemoveEntity(ent0);
	REQUIRE(ecs.GetEntityCount() == 1u);
	REQUIRE(ecs.GetComponentTypes(ent1).size() == 1u);
	REQUIRE(ecs.HasComponent<TestCComponent>(ent1));

	framework::T_EntityId const ent2 = ecs.AddEntity(TestAComponent());
	framework::T_EntityId const ent3 = ecs.AddEntity(TestAComponent());

	REQUIRE(ecs.GetEntityCount() == 3u);

	REQUIRE(ent2 == 0u);
	REQUIRE(ent3 == 2u);

	REQUIRE(ecs.GetComponentTypes(ent2).size() == 1u);
	REQUIRE(ecs.HasComponent<TestAComponent>(ent2));
	REQUIRE(ecs.GetComponentTypes(ent3).size() == 1u);
	REQUIRE(ecs.HasComponent<TestAComponent>(ent3));

	ecs.RemoveAllEntities();
	REQUIRE(ecs.GetEntityCount() == 0u);
}


TEST_CASE("controller entity hierachy", "[ecs]")
{
	framework::EcsController ecs;

	// create initial hierachy
	framework::T_EntityId const ent0 = ecs.AddEntity(TestAComponent());
	REQUIRE(ecs.GetChildren(ent0).size() == 0u);

	framework::T_EntityId const ent1 = ecs.AddEntityChild(ent0, TestAComponent(), TestBComponent("wowzer"), TestCComponent(9001));
	framework::T_EntityId const ent2 = ecs.AddEntityChild(ent0);
	framework::T_EntityId const ent3 = ecs.AddEntityChild(ent0, TestCComponent(3u), TestBComponent("3"));

	REQUIRE(ecs.GetEntityCount() == 4u);

	REQUIRE(ecs.GetComponentTypes(ent0).size() == 1u);
	REQUIRE(ecs.GetComponentTypes(ent1).size() == 3u);
	REQUIRE(ecs.GetComponentTypes(ent2).size() == 0u);
	REQUIRE(ecs.GetComponentTypes(ent3).size() == 2u);

	REQUIRE(ecs.GetChildren(ent0).size() == 3u);
	REQUIRE(ecs.GetChildren(ent0)[0] == ent1);
	REQUIRE(ecs.GetChildren(ent0)[1] == ent2);
	REQUIRE(ecs.GetChildren(ent0)[2] == ent3);
	REQUIRE_FALSE(ecs.HasParent(ent0));
	REQUIRE(ecs.GetParent(ent0) == framework::INVALID_ENTITY_ID);

	// reparent to lower level
	// 3 to 2
	REQUIRE(ecs.GetParent(ent2) == ent0);
	REQUIRE(ecs.GetChildren(ent2).size() == 0u);
	REQUIRE(ecs.GetParent(ent3) == ent0);

	ecs.ReparentEntity(ent3, ent2);
	REQUIRE(ecs.GetParent(ent3) == ent2);
	REQUIRE(ecs.GetChildren(ent2).size() == 1u);
	REQUIRE(ecs.GetChildren(ent2)[0] == ent3);
	REQUIRE(ecs.GetChildren(ent0).size() == 2u);
	REQUIRE(ecs.GetEntityCount() == 4u);

	// 2 to 1
	REQUIRE(ecs.GetParent(ent2) == ent0);
	REQUIRE(ecs.GetChildren(ent1).size() == 0u);

	ecs.ReparentEntity(ent2, ent1);
	REQUIRE(ecs.GetParent(ent2) == ent1);
	REQUIRE(ecs.GetChildren(ent1).size() == 1u);
	REQUIRE(ecs.GetChildren(ent1)[0] == ent2);
	REQUIRE(ecs.GetChildren(ent0).size() == 1u);
	REQUIRE(ecs.GetEntityCount() == 4u);

	// unparent 1
	ecs.ReparentEntity(ent1, framework::INVALID_ENTITY_ID);
	REQUIRE(ecs.GetParent(ent1) == framework::INVALID_ENTITY_ID);
	REQUIRE(ecs.GetChildren(ent0).size() == 0u);
	REQUIRE(ecs.GetEntityCount() == 4u);

	// remove entity 1, and its children implicitly (2 and 3)
	ecs.RemoveEntity(ent1);
	REQUIRE(ecs.GetEntityCount() == 1u);
}


TEST_CASE("controller system overwrite", "[ecs]")
{
	framework::EcsController ecs;
	// generate entities
	// first 10 don't have a C component so shouldn't be processed by the overwrite system
	for (uint32 idx = 0u; idx < static_cast<uint32>(10u); ++idx)
	{
		ecs.AddEntity(TestOverwriteComp());
	}

	// these will be processed by the overwrite system
	for (uint32 idx = 0u; idx < static_cast<uint32>(16u); ++idx)
	{
		ecs.AddEntity(TestOverwriteComp(), TestCComponent(idx));
	}

	ecs.RegisterSystem<TestOverwriteSystem>(4u);

	ecs.Process();

	REQUIRE(ecs.GetEntityCount() == 26u);

	// these entities don't have a C component and should remain untouched
	REQUIRE_FALSE(ecs.GetComponent<TestOverwriteComp>(0u).overwritten);
	REQUIRE_FALSE(ecs.GetComponent<TestOverwriteComp>(1u).overwritten);
	REQUIRE_FALSE(ecs.GetComponent<TestOverwriteComp>(2u).overwritten);
	REQUIRE_FALSE(ecs.GetComponent<TestOverwriteComp>(3u).overwritten);
	REQUIRE_FALSE(ecs.GetComponent<TestOverwriteComp>(4u).overwritten);
	REQUIRE_FALSE(ecs.GetComponent<TestOverwriteComp>(5u).overwritten);
	REQUIRE_FALSE(ecs.GetComponent<TestOverwriteComp>(6u).overwritten);
	REQUIRE_FALSE(ecs.GetComponent<TestOverwriteComp>(7u).overwritten);
	REQUIRE_FALSE(ecs.GetComponent<TestOverwriteComp>(8u).overwritten);
	REQUIRE_FALSE(ecs.GetComponent<TestOverwriteComp>(9u).overwritten);

	// these have a C component, but the value is too low for an overwrite to occur
	REQUIRE_FALSE(ecs.GetComponent<TestOverwriteComp>(10u).overwritten);
	REQUIRE_FALSE(ecs.GetComponent<TestOverwriteComp>(11u).overwritten);
	REQUIRE_FALSE(ecs.GetComponent<TestOverwriteComp>(12u).overwritten);
	REQUIRE_FALSE(ecs.GetComponent<TestOverwriteComp>(13u).overwritten);

	// these entities have all required components and C is high enough for the overwrite to occur
	REQUIRE(ecs.GetComponent<TestOverwriteComp>(14u).overwritten);
	REQUIRE(ecs.GetComponent<TestOverwriteComp>(15u).overwritten);
	REQUIRE(ecs.GetComponent<TestOverwriteComp>(16u).overwritten);
	REQUIRE(ecs.GetComponent<TestOverwriteComp>(17u).overwritten);
	REQUIRE(ecs.GetComponent<TestOverwriteComp>(18u).overwritten);
	REQUIRE(ecs.GetComponent<TestOverwriteComp>(19u).overwritten);
	REQUIRE(ecs.GetComponent<TestOverwriteComp>(20u).overwritten);
	REQUIRE(ecs.GetComponent<TestOverwriteComp>(21u).overwritten);
	REQUIRE(ecs.GetComponent<TestOverwriteComp>(22u).overwritten);
	REQUIRE(ecs.GetComponent<TestOverwriteComp>(23u).overwritten);
	REQUIRE(ecs.GetComponent<TestOverwriteComp>(24u).overwritten);
	REQUIRE(ecs.GetComponent<TestOverwriteComp>(25u).overwritten);
}


TEST_CASE("controller system hierachy", "[ecs]")
{
	framework::EcsController ecs;

	// generate entities
	ecs.AddEntity(TestCComponent(0u));
	ecs.AddEntityChild(0u, TestCComponent(0u));
	ecs.AddEntityChild(1u, TestCComponent(0u));
	ecs.AddEntityChild(2u, TestCComponent(0u));
	ecs.AddEntityChild(3u, TestCComponent(0u));
	ecs.AddEntityChild(4u, TestCComponent(0u));
	ecs.AddEntityChild(5u, TestCComponent(0u));
	ecs.AddEntityChild(6u, TestCComponent(0u));

	struct TestCHierachyView final : public framework::ComponentView
	{
		TestCHierachyView() : framework::ComponentView()
		{
			Declare(parentC);
			Declare(c);
		}

		ParentRead<TestCComponent> parentC;
		WriteAccess<TestCComponent> c;
	};

	class TestCHierachySystem final : public framework::System<TestCHierachySystem, TestCHierachyView>
	{
	public:
		TestCHierachySystem() = default;
		// base class constructors don't do anything so are not needed unless we declare dependencies or init lookup variables

		void Process(framework::ComponentRange<TestCHierachyView>& range) const
		{
			for (TestCHierachyView& view : range)
			{
				if (view.parentC.IsValid())
				{
					view.c->val = view.parentC->val + 1u;
				}
			}
		}
	};

	ecs.RegisterSystem<TestCHierachySystem>();

	ecs.Process();

	REQUIRE(ecs.GetEntityCount() == 8u);
	REQUIRE(ecs.GetComponent<TestCComponent>(0u).val == 0u);
	REQUIRE(ecs.GetComponent<TestCComponent>(1u).val == 1u);
	REQUIRE(ecs.GetComponent<TestCComponent>(2u).val == 2u);
	REQUIRE(ecs.GetComponent<TestCComponent>(3u).val == 3u);
	REQUIRE(ecs.GetComponent<TestCComponent>(4u).val == 4u);
	REQUIRE(ecs.GetComponent<TestCComponent>(5u).val == 5u);
	REQUIRE(ecs.GetComponent<TestCComponent>(6u).val == 6u);
	REQUIRE(ecs.GetComponent<TestCComponent>(7u).val == 7u);
}


TEST_CASE("controller component construction, component events", "[ecs]")
{
	framework::EcsController ecs;

	uint32 counter = 0;
	uint32 counter2 = 0;

	auto onAdded = [&counter2](framework::EcsController& controller, TestRefCountComp& comp, framework::T_EntityId const entity) -> void
	{
		UNUSED(controller);
		UNUSED(comp);
		UNUSED(entity);

		++counter2;
	};

	auto onRemoved = [&counter2](framework::EcsController& controller, TestRefCountComp& comp, framework::T_EntityId const entity) -> void
	{
		UNUSED(controller);
		UNUSED(comp);
		UNUSED(entity);

		--counter2;
	};

	framework::T_ComEventId id = ecs.RegisterOnComponentAdded(
		std::function<void(framework::EcsController&, TestRefCountComp&, framework::T_EntityId const)>(onAdded));
	framework::T_ComEventId id2 = ecs.RegisterOnComponentRemoved(
		std::function<void(framework::EcsController&, TestRefCountComp&, framework::T_EntityId const)>(onRemoved));

	// generate entities
	ecs.AddEntity(TestRefCountComp(&counter));
	REQUIRE(counter == 1u);
	REQUIRE(counter2 == 1u);

	ecs.AddEntity(TestRefCountComp(&counter));
	REQUIRE(counter == 2u);
	REQUIRE(counter2 == 2u);

	ecs.RemoveEntity(0u);
	REQUIRE(counter == 1u);
	REQUIRE(counter2 == 1u);

	ecs.RemoveEntity(1u);
	REQUIRE(counter == 0u);
	REQUIRE(counter2 == 0u);

	ecs.AddEntity(TestCComponent(0u));
	REQUIRE(counter2 == 0u);

	ecs.AddComponents(0u, TestRefCountComp(&counter));
	REQUIRE(counter2 == 1u);

	ecs.RemoveComponents<TestRefCountComp>(0u);
	REQUIRE(counter2 == 0u);

	ecs.UnregisterComponentEvent<TestRefCountComp>(id);
	ecs.AddEntity(TestRefCountComp(&counter));
	REQUIRE(counter == 1u);
	REQUIRE(counter2 == 0u);
}
