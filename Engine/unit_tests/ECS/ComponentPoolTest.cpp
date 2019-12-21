#include <EtFramework/stdafx.h>
#include "EcsTestUtilities.h"

#include <catch2/catch.hpp>
#include <rttr/registration>

#include <thread>
#include <chrono>

#include <mainTesting.h>

#include <EtFramework/ECS/ComponentPool.h>


TEST_CASE("append, get, at pool", "[ecs]")
{
	framework::ComponentPool myCPool(TestCComponent::GetTypeIndex());

	REQUIRE(myCPool.GetSize() == 0u);

	myCPool.Append(TestCComponent(0u));
	myCPool.Append<TestCComponent>(1u);
	{
		TestCComponent comp(2u);
		void const* const compPtr = static_cast<void const*>(&comp);
		myCPool.Append(compPtr);
	}
	{
		TestCComponent comp(3u);
		myCPool.Append(&comp);
	}

	REQUIRE(myCPool.GetSize() == 4u);

	TestCComponent const& at0 = myCPool.Get<TestCComponent>(0u);
	TestCComponent const& at1 = myCPool.Get<TestCComponent>(1u);
	void const* const at2Ptr = myCPool.At(2u);
	TestCComponent const& at2 = *static_cast<TestCComponent const*>(at2Ptr);
	TestCComponent& at3 = myCPool.Get<TestCComponent>(3u);

	REQUIRE(at0.val == 0u);
	REQUIRE(at1.val == 1u);
	REQUIRE(at2.val == 2u);
	REQUIRE(at3.val == 3u);

	at3 = 4u;
	REQUIRE(myCPool.Get<TestCComponent>(3u).val == 4u);
}


TEST_CASE("erase pool", "[ecs]")
{
	framework::ComponentPool myCPool(TestCComponent::GetTypeIndex());

	myCPool.Append(TestCComponent(0u));
	myCPool.Append(TestCComponent(1u));
	myCPool.Append(TestCComponent(2u));
	myCPool.Append(TestCComponent(3u));

	REQUIRE(myCPool.GetSize() == 4u);

	REQUIRE(myCPool.Get<TestCComponent>(1u).val == 1u);

	myCPool.Erase(1u);

	REQUIRE(myCPool.GetSize() == 3u);

	REQUIRE(myCPool.Get<TestCComponent>(1u).val == 3u);

	myCPool.Append(TestCComponent(5u));
	REQUIRE(myCPool.GetSize() == 4u);
	REQUIRE(myCPool.Get<TestCComponent>(1u).val == 3u);
	REQUIRE(myCPool.Get<TestCComponent>(3u).val == 5u);

	myCPool.Erase(3u);
	REQUIRE(myCPool.GetSize() == 3u);

	myCPool.Erase(0u);
	myCPool.Erase(0u);
	myCPool.Erase(0u);

	REQUIRE(myCPool.GetSize() == 0u);
}

