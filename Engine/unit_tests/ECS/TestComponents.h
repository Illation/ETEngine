#pragma once
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

	TestBComponent() = default;
	TestBComponent(std::string const& value) : name(value) {}

	std::string name = "whatever";
};

struct TestCComponent final
{
	ECS_DECLARE_COMPONENT
public:

	TestCComponent() = default;
	TestCComponent(uint32 const value) : val(value) {}

	uint32 val = 0u;
};
