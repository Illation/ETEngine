#include <EtFramework/stdafx.h>

#include <catch2/catch.hpp>
#include <rttr/registration>

#include <thread>
#include <chrono>

#include <mainTesting.h>

#include <EtFramework/ECS/ComponentRegistry.h>


struct TestAComponent final
{
	ECS_DECLARE_COMPONENT
public:

	int32 x = 0;
	float y = 1.f;
};

struct TestBComponent final
{
	ECS_DECLARE_COMPONENT
public:

	std::string name = "whatever";
};

// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<TestAComponent>("test A component")
		.property("x", &TestAComponent::x)
		.property("y", &TestAComponent::y);

	registration::class_<TestBComponent>("test B component")
		.property("name", &TestBComponent::name);
}

ECS_REGISTER_COMPONENT(TestAComponent);
ECS_REGISTER_COMPONENT(TestBComponent);


TEST_CASE( "register", "[ecs]" )
{
	framework::T_CompTypeIdx const aTypeIdx = TestAComponent::GetTypeIndex();
	framework::T_CompTypeIdx const bTypeIdx = TestBComponent::GetTypeIndex();

	REQUIRE(aTypeIdx == 0u);
	REQUIRE(bTypeIdx == 1u);

	size_t const aSize = framework::ComponentRegistry::Instance().GetSize(aTypeIdx);
	size_t const bSize = framework::ComponentRegistry::Instance().GetSize(bTypeIdx);

	REQUIRE(aSize == sizeof(int32) + sizeof(float));
	REQUIRE(bSize == sizeof(std::string));

	rttr::type const& aType = framework::ComponentRegistry::Instance().GetType(aTypeIdx);
	rttr::type const& bType = framework::ComponentRegistry::Instance().GetType(bTypeIdx);

	REQUIRE(aType == rttr::type::get<TestAComponent>());
	REQUIRE(bType == rttr::type::get<TestBComponent>());

	REQUIRE(aTypeIdx == framework::ComponentRegistry::Instance().GetTypeIdx(rttr::type::get<TestAComponent>()));
	REQUIRE(bTypeIdx == framework::ComponentRegistry::Instance().GetTypeIdx(rttr::type::get<TestBComponent>()));
}
