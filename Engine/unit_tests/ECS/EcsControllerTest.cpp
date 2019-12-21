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
