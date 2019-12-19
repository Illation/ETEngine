#pragma once
#include "TestComponents.h"

#include <EtFramework/ECS/Archetype.h>
#include <EtFramework/ECS/ComponentView.h>


struct TestBCView final : public framework::ComponentView
{
	WriteAccess<TestBComponent> b;
	ReadAccess<TestCComponent> c;

	void Register() override
	{
		Declare(b);
		Declare(c);
	}
};

struct TestCView final : public framework::ComponentView
{
	ReadAccess<TestCComponent> c;

	void Register() override
	{
		Declare(c);
	}
};

framework::Archetype GenTestArchetype(size_t const count);
