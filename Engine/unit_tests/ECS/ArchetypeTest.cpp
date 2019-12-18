#include <EtFramework/stdafx.h>
#include "TestComponents.h"

#include <catch2/catch.hpp>
#include <rttr/registration>

#include <thread>
#include <chrono>

#include <mainTesting.h>

#include <EtFramework/ECS/Archetype.h>


// components should already have been registered in ComponentRegistryTest.cpp


TEST_CASE("archetype create", "[ecs]")
{
	framework::Archetype archBC(framework::GenSignature<TestBComponent, TestCComponent>());

	REQUIRE_FALSE(archBC.HasComponent(TestAComponent::GetTypeIndex()));
	REQUIRE(archBC.HasComponent(TestBComponent::GetTypeIndex()));
	REQUIRE(archBC.HasComponent(TestCComponent::GetTypeIndex()));

	REQUIRE(archBC.GetSignature() == framework::GenSignature<TestCComponent, TestBComponent>());
	REQUIRE_FALSE(archBC.GetSignature() == framework::GenSignature<TestCComponent, TestAComponent>());

	REQUIRE(archBC.GetPool(TestBComponent::GetTypeIndex()).GetType() == TestBComponent::GetTypeIndex());
	REQUIRE(archBC.GetPool(TestCComponent::GetTypeIndex()).GetType() == TestCComponent::GetTypeIndex());

	REQUIRE(archBC.GetSize() == 0u);
}


TEST_CASE("archetype add", "[ecs]")
{
	framework::Archetype archBC(framework::GenSignature<TestBComponent, TestCComponent>());

	REQUIRE(archBC.GetSize() == 0u);

	framework::T_EntityId const ent0 = 2u;
	framework::T_EntityId const ent1 = 3u;
	framework::T_EntityId const ent2 = 6u;
	framework::T_EntityId const ent3 = 4u;
	
	std::string const val0 = "first";

	std::vector<framework::RawComponentData> compList{
		framework::MakeRawComponent(TestBComponent(val0)),
		framework::MakeRawComponent(TestCComponent(0u))
	};
	size_t const idx0 = archBC.AddEntity(ent0, compList);

	REQUIRE(archBC.GetSize() == 1u);

	size_t const idx1 = archBC.AddEntity(ent1, {framework::MakeRawComponent(TestBComponent("1")), framework::MakeRawComponent(TestCComponent(1u))});
	size_t const idx2 = archBC.AddEntity(ent2, {framework::MakeRawComponent(TestBComponent("2")), framework::MakeRawComponent(TestCComponent(2u))});
	size_t const idx3 = archBC.AddEntity(ent3, {framework::MakeRawComponent(TestBComponent("3")), framework::MakeRawComponent(TestCComponent(3u))});

	REQUIRE(archBC.GetSize() == 4u);

	REQUIRE(idx0 == 0u);
	REQUIRE(idx1 == 1u);
	REQUIRE(idx2 == 2u);
	REQUIRE(idx3 == 3u);

	REQUIRE(archBC.GetEntity(idx0) == ent0);
	REQUIRE(archBC.GetEntity(idx1) == ent1);
	REQUIRE(archBC.GetEntity(idx2) == ent2);
	REQUIRE(archBC.GetEntity(idx3) == ent3);

	TestBComponent const& b = archBC.GetPool(TestBComponent::GetTypeIndex()).Get<TestBComponent>(idx0);
	REQUIRE(b.name == val0);

	REQUIRE(archBC.GetPool(TestCComponent::GetTypeIndex()).Get<TestCComponent>(idx1).val == 1u);
	REQUIRE(archBC.GetPool(TestCComponent::GetTypeIndex()).Get<TestCComponent>(idx2).val == 2u);
	REQUIRE(archBC.GetPool(TestCComponent::GetTypeIndex()).Get<TestCComponent>(idx3).val == 3u);
}


TEST_CASE("archetype remove", "[ecs]")
{
	framework::Archetype archBC(framework::GenSignature<TestBComponent, TestCComponent>());

	framework::T_EntityId const ent0 = 10u;
	framework::T_EntityId const ent1 = 11u;
	framework::T_EntityId const ent2 = 12u;
	framework::T_EntityId const ent3 = 13u;
	framework::T_EntityId const ent4 = 14u;
	framework::T_EntityId const ent5 = 15u;

	size_t const idx0 = archBC.AddEntity(ent0, { framework::MakeRawComponent(TestBComponent("0")), framework::MakeRawComponent(TestCComponent(0u)) });
	size_t const idx1 = archBC.AddEntity(ent1, { framework::MakeRawComponent(TestBComponent("1")), framework::MakeRawComponent(TestCComponent(1u)) });
	size_t const idx2 = archBC.AddEntity(ent2, { framework::MakeRawComponent(TestBComponent("2")), framework::MakeRawComponent(TestCComponent(2u)) });
	size_t const idx3 = archBC.AddEntity(ent3, { framework::MakeRawComponent(TestBComponent("3")), framework::MakeRawComponent(TestCComponent(3u)) });

	REQUIRE(archBC.GetSize() == 4u);

	REQUIRE(archBC.GetEntity(idx1) == ent1);
	REQUIRE(archBC.GetPool(TestCComponent::GetTypeIndex()).Get<TestCComponent>(idx1).val == 1u);

	REQUIRE(archBC.GetEntity(idx3) == ent3);
	REQUIRE(archBC.GetPool(TestCComponent::GetTypeIndex()).Get<TestCComponent>(idx3).val == 3u);

	archBC.RemoveEntity(idx1);

	REQUIRE(archBC.GetSize() == 3u);

	REQUIRE(archBC.GetPool(TestBComponent::GetTypeIndex()).GetSize() == 3u);
	REQUIRE(archBC.GetPool(TestCComponent::GetTypeIndex()).GetSize() == 3u);

	REQUIRE(archBC.GetEntity(idx1) == ent3);
	REQUIRE_FALSE(archBC.GetEntity(idx1) == ent1);
	REQUIRE(archBC.GetPool(TestCComponent::GetTypeIndex()).Get<TestCComponent>(idx1).val == 3u);
	REQUIRE(archBC.GetPool(TestBComponent::GetTypeIndex()).Get<TestBComponent>(idx1).name == "3");

	size_t const idx4 = archBC.AddEntity(ent4, { framework::MakeRawComponent(TestBComponent("4")), framework::MakeRawComponent(TestCComponent(4u)) });
	size_t const idx5 = archBC.AddEntity(ent5, { framework::MakeRawComponent(TestBComponent("5")), framework::MakeRawComponent(TestCComponent(5u)) });

	REQUIRE(archBC.GetSize() == 5u);

	REQUIRE(idx4 == idx3);

	REQUIRE(archBC.GetEntity(idx1) == ent3);
	REQUIRE(archBC.GetPool(TestCComponent::GetTypeIndex()).Get<TestCComponent>(idx1).val == 3u);

	REQUIRE_FALSE(archBC.GetEntity(idx3) == ent3);
	REQUIRE(archBC.GetEntity(idx4) == ent4);
	REQUIRE(archBC.GetPool(TestCComponent::GetTypeIndex()).Get<TestCComponent>(idx4).val == 4u);

	REQUIRE(archBC.GetEntity(idx5) == ent5);
	REQUIRE(archBC.GetPool(TestCComponent::GetTypeIndex()).Get<TestCComponent>(idx5).val == 5u);

	size_t const compCount = archBC.GetSize();
	for (size_t idx = 0u; idx < compCount; ++idx)
	{
		archBC.RemoveEntity(0u);
	}

	REQUIRE(archBC.GetSize() == 0u);

	REQUIRE(archBC.GetPool(TestBComponent::GetTypeIndex()).GetSize() == 0u);
	REQUIRE(archBC.GetPool(TestCComponent::GetTypeIndex()).GetSize() == 0u);
}
