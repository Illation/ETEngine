#include <EtFramework/stdafx.h>
#include "TestComponents.h"

#include <catch2/catch.hpp>
#include <rttr/registration>

#include <thread>
#include <chrono>

#include <mainTesting.h>

#include <EtFramework/ECS/ComponentSignature.h>


// reflection
//------------
// #note: this is also used for other unit tests

RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<TestAComponent>("test A component")
		.property("x", &TestAComponent::x)
		.property("y", &TestAComponent::y);

	registration::class_<TestBComponent>("test B component")
		.property("name", &TestBComponent::name);

	registration::class_<TestCComponent>("test C component")
		.property("value", &TestCComponent::val);
}

ECS_REGISTER_COMPONENT(TestAComponent);
ECS_REGISTER_COMPONENT(TestBComponent);
ECS_REGISTER_COMPONENT(TestCComponent);


// tests
//-------


TEST_CASE( "register", "[ecs]" )
{
	framework::T_CompTypeIdx const aTypeIdx = TestAComponent::GetTypeIndex();
	framework::T_CompTypeIdx const bTypeIdx = TestBComponent::GetTypeIndex();
	framework::T_CompTypeIdx const cTypeIdx = TestCComponent::GetTypeIndex();

	REQUIRE(aTypeIdx == 0u);
	REQUIRE(bTypeIdx == 1u);
	REQUIRE(cTypeIdx == 2u);

	size_t const aSize = framework::ComponentRegistry::Instance().GetSize(aTypeIdx);
	size_t const bSize = framework::ComponentRegistry::Instance().GetSize(bTypeIdx);
	size_t const cSize = framework::ComponentRegistry::Instance().GetSize(cTypeIdx);

	REQUIRE(aSize == sizeof(int32) + sizeof(float));
	REQUIRE(bSize == sizeof(std::string));
	REQUIRE(cSize == sizeof(uint32));

	rttr::type const& aType = framework::ComponentRegistry::Instance().GetType(aTypeIdx);
	rttr::type const& bType = framework::ComponentRegistry::Instance().GetType(bTypeIdx);
	rttr::type const& cType = framework::ComponentRegistry::Instance().GetType(cTypeIdx);

	REQUIRE(aType == rttr::type::get<TestAComponent>());
	REQUIRE(bType == rttr::type::get<TestBComponent>());
	REQUIRE(cType == rttr::type::get<TestCComponent>());

	REQUIRE(aTypeIdx == framework::ComponentRegistry::Instance().GetTypeIdx(rttr::type::get<TestAComponent>()));
	REQUIRE(bTypeIdx == framework::ComponentRegistry::Instance().GetTypeIdx(rttr::type::get<TestBComponent>()));
	REQUIRE(cTypeIdx == framework::ComponentRegistry::Instance().GetTypeIdx(rttr::type::get<TestCComponent>()));
}


TEST_CASE("signature", "[ecs]")
{
	framework::T_CompTypeList cTypes = framework::GenCompTypeList<TestBComponent, TestAComponent>();

	REQUIRE(cTypes.size() == 2u);
	REQUIRE(cTypes[0] == 1u);
	REQUIRE(cTypes[1] == 0u);

	framework::ComponentSignature const sig(cTypes);
	REQUIRE(sig.GetSize() == 2u);
	REQUIRE(sig.GetTypes()[0] == 0u);
	REQUIRE(sig.GetTypes()[1] == 1u);
	REQUIRE(sig.GetMaxComponentType() == 1u);

	framework::ComponentSignature const abSig = framework::GenSignature<TestAComponent, TestBComponent>();
	REQUIRE(abSig == sig);
	REQUIRE(abSig.Contains(sig));
	REQUIRE(sig.Contains(abSig));

	framework::ComponentSignature const bcSig = framework::GenSignature<TestCComponent, TestBComponent>();
	REQUIRE(bcSig.GetTypes()[0] == 1u);
	REQUIRE(bcSig.GetTypes()[1] == 2u);
	REQUIRE_FALSE(abSig == bcSig);

	std::vector<framework::RawComponentData> compList{
		framework::RawComponentData(TestBComponent::GetTypeIndex(), nullptr),
		framework::RawComponentData(TestAComponent::GetTypeIndex(), nullptr)
	};
	REQUIRE(abSig.MatchesComponentsUnsorted(compList));
	REQUIRE_FALSE(bcSig.MatchesComponentsUnsorted(compList));
}
