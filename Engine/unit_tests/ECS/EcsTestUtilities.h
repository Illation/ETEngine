#pragma once
#include <EtFramework/ECS/ComponentRegistry.h>
#include <EtFramework/ECS/Archetype.h>
#include <EtFramework/ECS/ComponentView.h>
#include <EtFramework/ECS/System.h>


// Components
//************

struct TestAComponent final
{
	ECS_DECLARE_COMPONENT
public:
	TestAComponent() = default;

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

struct TestOverwriteComp final
{
	ECS_DECLARE_COMPONENT
public:

	TestOverwriteComp() = default;

	bool overwritten = false;
};

// Views
//*******

struct TestBCView final : public framework::ComponentView
{
	TestBCView() : framework::ComponentView()
	{
		Declare(b);
		Declare(c);
	}

	WriteAccess<TestBComponent> b;
	ReadAccess<TestCComponent> c;
};

struct TestCView final : public framework::ComponentView
{
	TestCView() : framework::ComponentView()
	{
		Declare(c);
	}

	ReadAccess<TestCComponent> c;
};

struct TestOverwriteSystemView final : public framework::ComponentView
{
	TestOverwriteSystemView() : framework::ComponentView()
	{
		Declare(overwrite);
		Declare(c);
	}

	WriteAccess<TestOverwriteComp> overwrite;
	ReadAccess<TestCComponent> c;
};

// utility
//*********

framework::Archetype GenTestArchetype(size_t const count);

// systems
//*********

class TestBCSystem final : public framework::System<TestBCSystem, TestBCView>
{
public:
	TestBCSystem() = default;
	// base class constructors don't do anything so are not needed unless we declare dependencies or init lookup variables

	void Process(framework::ComponentRange<TestBCView>& range) const override;
};

class TestOverwriteSystem final : public framework::System<TestOverwriteSystem, TestOverwriteSystemView>
{
public:
	TestOverwriteSystem(size_t const min)
		: m_OverwriteMin(min)
	{
		DeclareDependents<TestBCSystem>();
	}

	void Process(framework::ComponentRange<TestOverwriteSystemView>& range) const override;

private:
	size_t const m_OverwriteMin = 0u;
};
