#include <EtFramework/stdafx.h>
#include "EcsTestUtilities.h"

#include <catch2/catch.hpp>

#include <thread>
#include <chrono>

#include <mainTesting.h>

#include <EtFramework/ECS/ComponentSignature.h>


TEST_CASE( "register", "[ecs]" )
{
	fw::T_CompTypeIdx const aTypeIdx = TestAComponent::GetTypeIndex();
	fw::T_CompTypeIdx const bTypeIdx = TestBComponent::GetTypeIndex();
	fw::T_CompTypeIdx const cTypeIdx = TestCComponent::GetTypeIndex();

	REQUIRE(aTypeIdx == 0u);
	REQUIRE(bTypeIdx == 1u);
	REQUIRE(cTypeIdx == 2u);

	size_t const aSize = fw::ComponentRegistry::Instance().GetSize(aTypeIdx);
	size_t const bSize = fw::ComponentRegistry::Instance().GetSize(bTypeIdx);
	size_t const cSize = fw::ComponentRegistry::Instance().GetSize(cTypeIdx);

	REQUIRE(aSize == sizeof(int32) + sizeof(float));
	REQUIRE(bSize == sizeof(std::string));
	REQUIRE(cSize == sizeof(uint32));

	rttr::type const& aType = fw::ComponentRegistry::Instance().GetType(aTypeIdx);
	rttr::type const& bType = fw::ComponentRegistry::Instance().GetType(bTypeIdx);
	rttr::type const& cType = fw::ComponentRegistry::Instance().GetType(cTypeIdx);

	REQUIRE(aType == rttr::type::get<TestAComponent>());
	REQUIRE(bType == rttr::type::get<TestBComponent>());
	REQUIRE(cType == rttr::type::get<TestCComponent>());

	REQUIRE(aTypeIdx == fw::ComponentRegistry::Instance().GetTypeIdx(rttr::type::get<TestAComponent>()));
	REQUIRE(bTypeIdx == fw::ComponentRegistry::Instance().GetTypeIdx(rttr::type::get<TestBComponent>()));
	REQUIRE(cTypeIdx == fw::ComponentRegistry::Instance().GetTypeIdx(rttr::type::get<TestCComponent>()));
}


TEST_CASE("signature", "[ecs]")
{
	fw::T_CompTypeList cTypes = fw::GenCompTypeList<TestBComponent, TestAComponent>();

	REQUIRE(cTypes.size() == 2u);
	REQUIRE(cTypes[0] == 1u);
	REQUIRE(cTypes[1] == 0u);

	fw::ComponentSignature const sig(cTypes);
	REQUIRE(sig.GetSize() == 2u);
	REQUIRE(sig.GetTypes()[0] == 0u);
	REQUIRE(sig.GetTypes()[1] == 1u);
	REQUIRE(sig.GetMaxComponentType() == 1u);

	fw::ComponentSignature const abSig = fw::GenSignature<TestAComponent, TestBComponent>();
	REQUIRE(abSig == sig);
	REQUIRE(abSig.Contains(sig));
	REQUIRE(sig.Contains(abSig));

	fw::ComponentSignature const bcSig = fw::GenSignature<TestCComponent, TestBComponent>();
	REQUIRE(bcSig.GetTypes()[0] == 1u);
	REQUIRE(bcSig.GetTypes()[1] == 2u);
	REQUIRE_FALSE(abSig == bcSig);

	std::vector<fw::RawComponentPtr> compList{
		fw::RawComponentPtr(TestBComponent::GetTypeIndex(), nullptr),
		fw::RawComponentPtr(TestAComponent::GetTypeIndex(), nullptr)
	};
	REQUIRE(abSig.MatchesComponentsUnsorted(compList));
	REQUIRE_FALSE(bcSig.MatchesComponentsUnsorted(compList));
}
