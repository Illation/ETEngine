#include <EtFramework/stdafx.h>

#include <catch2/catch.hpp>

#include <mainTesting.h>

#include <EtCore/Memory/UniquePointer.h>


using namespace et;


class TestClass 
{
	static size_t s_RefCount;
public:
	static size_t GetRefCount() { return s_RefCount; }

	TestClass(size_t const data = 0u) : m_Data(data) { s_RefCount++; }
	TestClass(TestClass const& other) { m_Data = other.m_Data; s_RefCount++; }
	virtual ~TestClass() { s_RefCount--; }

	size_t GetData() const { return m_Data; }
	virtual bool IsDerived() const { return false; }

private:
	size_t m_Data = 0u;
};

class DerivedClass final : public TestClass
{
public:
	DerivedClass(size_t const data = 0u) : TestClass(data) {}
	~DerivedClass() = default;

	bool IsDerived() const override { return true; }
};

size_t TestClass::s_RefCount = 0u;


TEST_CASE("create", "[pointer_framework]")
{
	REQUIRE(TestClass::GetRefCount() == 0u);

	// test memory being created
	{
		Create<TestClass> testCreate;
		REQUIRE(TestClass::GetRefCount() == 1u);

		// moved
		Create<TestClass> moved = std::move(testCreate);
		REQUIRE(TestClass::GetRefCount() == 1u);
	}

	// and deleted after going out of scope
	REQUIRE(TestClass::GetRefCount() == 0u);

	// next - this should not be done but for testing purposes we release ownership from create to a raw pointer
	// make sure memory isn't deleted when Create<> goes out of scope after ownership has been transferred
	TestClass* rawPtr = nullptr;
	{
		Create<TestClass> testCreate;
		REQUIRE(TestClass::GetRefCount() == 1u);

		rawPtr = testCreate.Release();
		REQUIRE(TestClass::GetRefCount() == 1u);
		REQUIRE(rawPtr != nullptr);
	}

	REQUIRE(TestClass::GetRefCount() == 1u);

	delete rawPtr;
	REQUIRE(TestClass::GetRefCount() == 0u);
}

TEST_CASE("unique", "[pointer_framework]")
{
	REQUIRE(TestClass::GetRefCount() == 0u);

	// creating memory
	{
		UniquePtr<TestClass> t1 = Create<TestClass>();
		REQUIRE(TestClass::GetRefCount() == 1u);

		// create from raw prohibited
		//UniquePtr<TestClass> t2 = new TestClass();
		//UniquePtr<TestClass> t3(new TestClass());
	}

	// and deleted after going out of scope
	REQUIRE(TestClass::GetRefCount() == 0u);

	// moving memory
	{
		UniquePtr<TestClass> t1 = Create<TestClass>(2u);
		REQUIRE(TestClass::GetRefCount() == 1u);
		REQUIRE(!t1.IsNull());
		REQUIRE(t1->GetData() == 2u);

		// copy prohibited
		//UniquePtr<TestClass> t3 = t1;

		// moved
		UniquePtr<TestClass> t2 = std::move(t1);
		REQUIRE(TestClass::GetRefCount() == 1u);
		REQUIRE(t1.IsNull());
		REQUIRE(!t2.IsNull());
		REQUIRE(t2->GetData() == 2u);
	}

	REQUIRE(TestClass::GetRefCount() == 0u);

	// override content
	{
		UniquePtr<TestClass> t1 = Create<TestClass>(2u);
		REQUIRE(TestClass::GetRefCount() == 1u);
		REQUIRE(!t1.IsNull());
		REQUIRE(t1->GetData() == 2u);

		// moved
		t1 = Create<TestClass>(3u);
		REQUIRE(TestClass::GetRefCount() == 1u);
		REQUIRE(!t1.IsNull());
		REQUIRE(t1 != nullptr);
		REQUIRE(t1->GetData() == 3u);

		t1 = nullptr;
		REQUIRE(TestClass::GetRefCount() == 0u);
		REQUIRE(t1.IsNull());
		REQUIRE(t1 == nullptr);
	}

	REQUIRE(TestClass::GetRefCount() == 0u);

	// polymorphic
	{
		UniquePtr<TestClass> t1 = Create<TestClass>(2u);
		UniquePtr<TestClass> t2 = Create<DerivedClass>(2u);
		REQUIRE(TestClass::GetRefCount() == 2u);
		REQUIRE(!(t1->IsDerived()));
		REQUIRE(t2->IsDerived());
	}

	REQUIRE(TestClass::GetRefCount() == 0u);
}
