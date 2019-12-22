#include <EtFramework/stdafx.h>
#include "EcsTestUtilities.h"

#include <catch2/catch.hpp>
#include <rttr/registration>

#include <thread>
#include <chrono>

#include <mainTesting.h>

#include <EtFramework/ECS/EcsController.h>


// some structures to test correct system ordering
///////////////////////////////////////////////////

struct TestEmptyView final : public framework::ComponentView {};

class TestFirstSystem final : public framework::System<TestFirstSystem, TestEmptyView>
{
public:
	TestFirstSystem(std::vector<framework::T_SystemType>* const order)
		: m_Order(order)
	{
	}

	void Process(framework::ComponentRange<TestEmptyView>& range) const override
	{
		m_Order->push_back(GetTypeId());
	}
private:
	std::vector<framework::T_SystemType>* const m_Order;
};

class TestLastSystem final : public framework::System<TestLastSystem, TestEmptyView>
{
public:
	TestLastSystem(std::vector<framework::T_SystemType>* const order)
		: m_Order(order)
	{
		DeclareDependencies<TestFirstSystem>();
	}

	void Process(framework::ComponentRange<TestEmptyView>& range) const override
	{
		m_Order->push_back(GetTypeId());
	}
private:
	std::vector<framework::T_SystemType>* const m_Order;
};

class TestBetweenSystem final : public framework::System<TestBetweenSystem, TestEmptyView>
{
public:
	TestBetweenSystem(std::vector<framework::T_SystemType>* const order)
		: m_Order(order)
	{
		DeclareDependencies<TestFirstSystem>();
		DeclareDependents<TestLastSystem>();
	}

	void Process(framework::ComponentRange<TestEmptyView>& range) const override
	{
		m_Order->push_back(GetTypeId());
	}
private:
	std::vector<framework::T_SystemType>* const m_Order;
};


// the test
////////////

TEST_CASE("controller system scheduling", "[ecs]")
{
	std::vector<framework::T_SystemType> executedOrder;

	framework::EcsController ecs;

	framework::T_EntityId const ent0 = ecs.AddEntity();

	ecs.RegisterSystem<TestLastSystem>(&executedOrder);
	ecs.RegisterSystem<TestFirstSystem>(&executedOrder);
	ecs.RegisterSystem<TestBetweenSystem>(&executedOrder);

	framework::T_EntityId const ent1 = ecs.AddEntity(TestAComponent());
	// every system will be called twice (once for each entity archetype)

	ecs.Process();

	REQUIRE(executedOrder.size() == 6u);
	REQUIRE(executedOrder[0] == rttr::type::get<TestFirstSystem>().get_id());
	REQUIRE(executedOrder[1] == rttr::type::get<TestFirstSystem>().get_id());
	REQUIRE(executedOrder[2] == rttr::type::get<TestBetweenSystem>().get_id());
	REQUIRE(executedOrder[3] == rttr::type::get<TestBetweenSystem>().get_id());
	REQUIRE(executedOrder[4] == rttr::type::get<TestLastSystem>().get_id());
	REQUIRE(executedOrder[5] == rttr::type::get<TestLastSystem>().get_id());
}

