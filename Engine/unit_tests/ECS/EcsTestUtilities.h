#pragma once
#include <EtFramework/ECS/ComponentRegistry.h>
#include <EtFramework/ECS/Archetype.h>
#include <EtFramework/ECS/ComponentView.h>
#include <EtFramework/ECS/System.h>


using namespace et;


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

struct TestRefCountComp final
{
	ECS_DECLARE_COMPONENT
public:

	TestRefCountComp() = default;
	TestRefCountComp(uint32* const p) 
		: ptr(p) 
	{ 
		(*ptr)++; 
	}
	TestRefCountComp(TestRefCountComp const& other)
	{
		ptr = other.ptr;
		(*ptr)++;
	}
	~TestRefCountComp() 
	{ 
		(*ptr)--; 
	}

	uint32* ptr = nullptr;
};


// Views
//*******

struct TestBCView final : public fw::ComponentView
{
	TestBCView() : fw::ComponentView()
	{
		Declare(b);
		Declare(c);
	}

	WriteAccess<TestBComponent> b;
	ReadAccess<TestCComponent> c;
};

struct TestCView final : public fw::ComponentView
{
	TestCView() : fw::ComponentView()
	{
		Declare(c);
	}

	ReadAccess<TestCComponent> c;
};

struct TestCWriteView final : public fw::ComponentView
{
	TestCWriteView() : fw::ComponentView()
	{
		Declare(c);
	}

	WriteAccess<TestCComponent> c;
};

struct TestOverwriteSystemView final : public fw::ComponentView
{
	TestOverwriteSystemView() : fw::ComponentView()
	{
		Declare(overwrite);
		Declare(c);
	}

	WriteAccess<TestOverwriteComp> overwrite;
	ReadAccess<TestCComponent> c;
};

// utility
//*********

fw::Archetype GenTestArchetype(size_t const count);

// systems
//*********

class TestBCSystem final : public fw::System<TestBCSystem, TestBCView>
{
public:
	TestBCSystem() = default;
	// base class constructors don't do anything so are not needed unless we declare dependencies or init lookup variables

	void Process(fw::ComponentRange<TestBCView>& range) override;
};

class TestOverwriteSystem final : public fw::System<TestOverwriteSystem, TestOverwriteSystemView>
{
public:
	TestOverwriteSystem(size_t const min)
		: m_OverwriteMin(min)
	{
		DeclareDependents<TestBCSystem>();
	}

	void Process(fw::ComponentRange<TestOverwriteSystemView>& range) override;

private:
	size_t const m_OverwriteMin = 0u;
};
