#include <EtFramework/stdafx.h>

#include <catch2/catch.hpp>

#include <mainTesting.h>

#include <EtCore/Memory/UniquePointer.h>
#include <EtCore/Memory/RefPointer.h>
#include <EtCore/Memory/WeakPointer.h>
#include <EtCore/Memory/Pointer.h>


using namespace et;


class TestClass 
{
	static size_t s_InstanceCount;
public:
	static size_t GetInstanceCount() { return s_InstanceCount; }

	TestClass(size_t const data = 0u) : m_Data(data) { s_InstanceCount++; }
	TestClass(TestClass const& other) { m_Data = other.m_Data; s_InstanceCount++; }
	virtual ~TestClass() { s_InstanceCount--; }

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

size_t TestClass::s_InstanceCount = 0u;


TEST_CASE("create", "[pointer_framework]")
{
	REQUIRE(TestClass::GetInstanceCount() == 0u);

	// test memory being created
	{
		Create<TestClass> testCreate;
		REQUIRE(TestClass::GetInstanceCount() == 1u);

		// moved
		Create<TestClass> moved = std::move(testCreate);
		REQUIRE(TestClass::GetInstanceCount() == 1u);
	}

	// and deleted after going out of scope
	REQUIRE(TestClass::GetInstanceCount() == 0u);

	// next - this should not be done but for testing purposes we release ownership from create to a raw pointer
	// make sure memory isn't deleted when Create<> goes out of scope after ownership has been transferred
	TestClass* rawPtr = nullptr;
	{
		Create<TestClass> testCreate;
		REQUIRE(TestClass::GetInstanceCount() == 1u);

		rawPtr = testCreate.Release();
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(rawPtr != nullptr);
	}

	REQUIRE(TestClass::GetInstanceCount() == 1u);

	delete rawPtr;
	REQUIRE(TestClass::GetInstanceCount() == 0u);
}

TEST_CASE("unique", "[pointer_framework]")
{
	REQUIRE(TestClass::GetInstanceCount() == 0u);

	// creating memory
	{
		UniquePtr<TestClass> t1 = Create<TestClass>();
		REQUIRE(TestClass::GetInstanceCount() == 1u);

		// create from raw prohibited
		//UniquePtr<TestClass> t2 = new TestClass();
		//UniquePtr<TestClass> t3(new TestClass());
	}

	// and deleted after going out of scope
	REQUIRE(TestClass::GetInstanceCount() == 0u);

	// moving memory
	{
		UniquePtr<TestClass> t1 = Create<TestClass>(2u);
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(!t1.IsNull());
		REQUIRE(t1->GetData() == 2u);

		// copy prohibited
		//UniquePtr<TestClass> t3 = t1;

		// moved
		UniquePtr<TestClass> t2 = std::move(t1);
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(t1.IsNull());
		REQUIRE(!t2.IsNull());
		REQUIRE(t2->GetData() == 2u);
	}

	REQUIRE(TestClass::GetInstanceCount() == 0u);

	// override content
	{
		UniquePtr<TestClass> t1 = Create<TestClass>(2u);
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(!t1.IsNull());
		REQUIRE(t1->GetData() == 2u);

		// moved
		t1 = Create<TestClass>(3u);
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(!t1.IsNull());
		REQUIRE(t1 != nullptr);
		REQUIRE(t1->GetData() == 3u);

		t1 = nullptr;
		REQUIRE(TestClass::GetInstanceCount() == 0u);
		REQUIRE(t1.IsNull());
		REQUIRE(t1 == nullptr);
	}

	REQUIRE(TestClass::GetInstanceCount() == 0u);

	// polymorphic
	{
		UniquePtr<TestClass> t1 = Create<TestClass>(2u);
		UniquePtr<TestClass> t2 = Create<DerivedClass>(2u);
		REQUIRE(TestClass::GetInstanceCount() == 2u);
		REQUIRE(!(t1->IsDerived()));
		REQUIRE(t2->IsDerived());
	}

	REQUIRE(TestClass::GetInstanceCount() == 0u);
}

TEST_CASE("ref", "[pointer_framework]")
{
	REQUIRE(TestClass::GetInstanceCount() == 0u);

	// empty
	{
		RefPtr<TestClass> t1;
		REQUIRE(TestClass::GetInstanceCount() == 0u);
		REQUIRE(t1.IsNull());
		REQUIRE(t1 == nullptr);

		RefPtr<TestClass> t2 = nullptr;
		REQUIRE(TestClass::GetInstanceCount() == 0u);
		REQUIRE(t2.IsNull());
		REQUIRE(t2 == nullptr);

		REQUIRE(t1 == t2);
	}

	REQUIRE(TestClass::GetInstanceCount() == 0u);

	// creating memory
	{
		RefPtr<TestClass> t1 = Create<TestClass>(4u);
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(t1->GetData() == 4u);

		// create from raw prohibited
		//RefPtr<TestClass> t2 = new TestClass();
		//RefPtr<TestClass> t3(new TestClass());
	}

	REQUIRE(TestClass::GetInstanceCount() == 0u);

	// polymorphic
	{
		RefPtr<TestClass> t1 = Create<TestClass>(2u);
		RefPtr<TestClass> t2 = Create<DerivedClass>(2u);
		REQUIRE(TestClass::GetInstanceCount() == 2u);
		REQUIRE(!(t1->IsDerived()));
		REQUIRE(t2->IsDerived());
	}

	REQUIRE(TestClass::GetInstanceCount() == 0u);

	// override content
	{
		RefPtr<TestClass> t1 = Create<TestClass>(2u);
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(!t1.IsNull());
		REQUIRE(t1->GetData() == 2u);

		// moved
		t1 = Create<TestClass>(3u);
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(!t1.IsNull());
		REQUIRE(t1 != nullptr);
		REQUIRE(t1->GetData() == 3u);

		t1 = nullptr;
		REQUIRE(TestClass::GetInstanceCount() == 0u);
		REQUIRE(t1.IsNull());
		REQUIRE(t1 == nullptr);
	}

	REQUIRE(TestClass::GetInstanceCount() == 0u);

	// multiple references
	{
		RefPtr<TestClass> t1 = Create<TestClass>(5u);
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(!t1.IsNull());
		REQUIRE(t1->GetData() == 5u);

		{
			RefPtr<TestClass> const t2 = t1;
			REQUIRE(TestClass::GetInstanceCount() == 1u);
			REQUIRE(!t2.IsNull());
			REQUIRE(t2->GetData() == 5u);
			REQUIRE(!t1.IsNull());
			REQUIRE(t1->GetData() == 5u);
			REQUIRE(t1 == t2);
		}

		REQUIRE(TestClass::GetInstanceCount() == 1u);

		RefPtr<TestClass> t3(t1);
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(!t3.IsNull());
		REQUIRE(t3->GetData() == 5u);
		REQUIRE(!t1.IsNull());
		REQUIRE(t1->GetData() == 5u);
		REQUIRE(t1 == t3);

		t1 = nullptr;
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(t1 == nullptr);
		REQUIRE(t3 != nullptr);
		REQUIRE(t3->GetData() == 5u);

		t3 = t1;
		REQUIRE(TestClass::GetInstanceCount() == 0u);
		REQUIRE(t3 == nullptr);
	}

	REQUIRE(TestClass::GetInstanceCount() == 0u);

	// overwrite with multiple references
	{
		RefPtr<TestClass> t1 = Create<TestClass>(6u);
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(!t1.IsNull());
		REQUIRE(t1->GetData() == 6u);

		RefPtr<TestClass> const t3(t1);
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(!t3.IsNull());
		REQUIRE(t3->GetData() == 6u);
		REQUIRE(!t1.IsNull());
		REQUIRE(t1->GetData() == 6u);
		REQUIRE(t1 == t3);

		t1 = Create<TestClass>(7u);
		REQUIRE(TestClass::GetInstanceCount() == 2u);
		REQUIRE(t1 != nullptr);
		REQUIRE(t1->GetData() == 7u);
		REQUIRE(t3 != nullptr);
		REQUIRE(t3->GetData() == 6u);
		REQUIRE(t1 != t3);
	}

	REQUIRE(TestClass::GetInstanceCount() == 0u);

	// moving from another ref ptr
	{
		RefPtr<TestClass> t1 = Create<TestClass>(2u);
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(!t1.IsNull());
		REQUIRE(t1->GetData() == 2u);

		// moved
		RefPtr<TestClass> t2 = std::move(t1);
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(t1.IsNull());
		REQUIRE(!t2.IsNull());
		REQUIRE(t2->GetData() == 2u);
	}

	REQUIRE(TestClass::GetInstanceCount() == 0u);

	// moving from a unique ptr
	{
		UniquePtr<TestClass> t1 = Create<TestClass>(2u);
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(!t1.IsNull());
		REQUIRE(t1->GetData() == 2u);

		RefPtr<TestClass> t2 = Create<TestClass>(3u);
		REQUIRE(TestClass::GetInstanceCount() == 2u);
		REQUIRE(t2->GetData() == 3u);

		// moved
		t2 = std::move(t1);
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(t1.IsNull());
		REQUIRE(!t2.IsNull());
		REQUIRE(t2->GetData() == 2u);
	}

	REQUIRE(TestClass::GetInstanceCount() == 0u);
}

TEST_CASE("weak", "[pointer_framework]")
{
	REQUIRE(TestClass::GetInstanceCount() == 0u);

	// empty
	{
		WeakPtr<TestClass> t1;
		REQUIRE(TestClass::GetInstanceCount() == 0u);
		REQUIRE(t1.IsNull());
		REQUIRE(t1 == nullptr);

		WeakPtr<TestClass> t2 = nullptr;
		REQUIRE(TestClass::GetInstanceCount() == 0u);
		REQUIRE(t2.IsNull());
		REQUIRE(t2 == nullptr);

		REQUIRE(t1 == t2);
	}

	REQUIRE(TestClass::GetInstanceCount() == 0u);

	// create from refptr
	{
		RefPtr<TestClass> t1 = Create<TestClass>(2u);
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(!t1.IsNull());
		REQUIRE(t1->GetData() == 2u);

		RefPtr<TestClass> t2(t1);
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(!t2.IsNull());
		REQUIRE(t2->GetData() == 2u);
		REQUIRE(!t1.IsNull());
		REQUIRE(t1->GetData() == 2u);
		REQUIRE(t1 == t2);

		WeakPtr<TestClass> weak = t1;
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(weak->GetData() == 2u);
		REQUIRE(weak != nullptr);
		REQUIRE(weak == t1);

		WeakPtr<TestClass> weak2(t2);
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(weak2->GetData() == 2u);
		REQUIRE(weak2 != nullptr);
		REQUIRE(weak2 == t2);
		REQUIRE(weak2 == t1);
		REQUIRE(weak2 == weak);

		t2 = nullptr;
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(t1 != nullptr);
		REQUIRE(t2 == nullptr);
		REQUIRE(weak != nullptr);
		REQUIRE(weak2 != nullptr);

		t1 = nullptr;
		REQUIRE(TestClass::GetInstanceCount() == 0u);
		REQUIRE(t1 == nullptr);
		REQUIRE(weak == nullptr);
		REQUIRE(weak2 == nullptr);
	}

	REQUIRE(TestClass::GetInstanceCount() == 0u);

	// copy from weak
	{
		RefPtr<TestClass> t1 = Create<TestClass>(2u);
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(!t1.IsNull());
		REQUIRE(t1->GetData() == 2u);

		WeakPtr<TestClass> weak = t1;
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(weak->GetData() == 2u);
		REQUIRE(weak != nullptr);

		WeakPtr<TestClass> weak2 = weak;
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(weak2->GetData() == 2u);
		REQUIRE(weak2 != nullptr);

		WeakPtr<TestClass> weak3(weak2);
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(weak3->GetData() == 2u);
		REQUIRE(weak3 != nullptr);

		t1 = nullptr;
		REQUIRE(TestClass::GetInstanceCount() == 0u);
		REQUIRE(t1 == nullptr);
		REQUIRE(weak == nullptr);
		REQUIRE(weak2 == nullptr);
		REQUIRE(weak3 == nullptr);
	}

	REQUIRE(TestClass::GetInstanceCount() == 0u);

	// reset
	{
		RefPtr<TestClass> t1 = Create<TestClass>(2u);
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(!t1.IsNull());
		REQUIRE(t1->GetData() == 2u);

		WeakPtr<TestClass> weak = t1;
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(weak->GetData() == 2u);
		REQUIRE(weak != nullptr);
		REQUIRE(weak == t1);

		RefPtr<TestClass> t2(t1);
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(!t2.IsNull());
		REQUIRE(t1 == t2);
		REQUIRE(t2 == weak);

		t1 = nullptr;
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(t1.IsNull());
		REQUIRE(!weak.IsNull());
		REQUIRE(!t2.IsNull());

		weak = nullptr;
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(t1.IsNull());
		REQUIRE(weak.IsNull());
		REQUIRE(!t2.IsNull());

		t2 = nullptr;
		REQUIRE(TestClass::GetInstanceCount() == 0u);
		REQUIRE(t2 == nullptr);
		REQUIRE(weak == nullptr);
	}

	REQUIRE(TestClass::GetInstanceCount() == 0u);

	// move from weak
	{
		RefPtr<TestClass> t1 = Create<TestClass>(2u);
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(!t1.IsNull());
		REQUIRE(t1->GetData() == 2u);

		WeakPtr<TestClass> weak = t1;
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(weak != nullptr);
		REQUIRE(weak->GetData() == 2u);

		WeakPtr<TestClass> weak2 = std::move(weak);
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(weak2 != nullptr);
		REQUIRE(weak2->GetData() == 2u);
		REQUIRE(weak == nullptr);

		WeakPtr<TestClass> weak3(std::move(weak2));
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(weak3 != nullptr);
		REQUIRE(weak3->GetData() == 2u);
		REQUIRE(weak2 == nullptr);
		REQUIRE(weak == nullptr);
	}

	REQUIRE(TestClass::GetInstanceCount() == 0u);

	// polymorphic
	{
		RefPtr<TestClass> t1 = Create<TestClass>(2u);
		RefPtr<TestClass> t2 = Create<DerivedClass>(2u);
		RefPtr<DerivedClass> t3 = Create<DerivedClass>(2u);
		REQUIRE(TestClass::GetInstanceCount() == 3u);
		REQUIRE(!(t1->IsDerived()));
		REQUIRE(t2->IsDerived());
		REQUIRE(t3->IsDerived());

		// derived from ref
		WeakPtr<TestClass> weak1 = t1;
		WeakPtr<TestClass> weak2 = t2;
		WeakPtr<DerivedClass> weak3 = t3;
		WeakPtr<TestClass> weak4 = t3;
		REQUIRE(TestClass::GetInstanceCount() == 3u);
		REQUIRE(!(weak1->IsDerived()));
		REQUIRE(weak2->IsDerived());
		REQUIRE(weak3->IsDerived());
		REQUIRE(weak4->IsDerived());

		// derived from weak
		WeakPtr<TestClass> weak5 = weak1;
		WeakPtr<TestClass> weak6 = weak2;
		WeakPtr<TestClass> weak7 = weak3;
		REQUIRE(TestClass::GetInstanceCount() == 3u);
		REQUIRE(!(weak5->IsDerived()));
		REQUIRE(weak6->IsDerived());
		REQUIRE(weak7->IsDerived());
	}

	REQUIRE(TestClass::GetInstanceCount() == 0u);
}

TEST_CASE("pointer_wrapper", "[pointer_framework]")
{
	REQUIRE(TestClass::GetInstanceCount() == 0u);

	// empty
	{
		Ptr<TestClass> t1;
		REQUIRE(TestClass::GetInstanceCount() == 0u);
		REQUIRE(t1.IsNull());
		REQUIRE(t1 == nullptr);

		Ptr<TestClass> t2 = nullptr;
		REQUIRE(TestClass::GetInstanceCount() == 0u);
		REQUIRE(t2.IsNull());
		REQUIRE(t2 == nullptr);

		REQUIRE(t1 == t2);
	}

	REQUIRE(TestClass::GetInstanceCount() == 0u);

	// creating from raw
	{
		UniquePtr<TestClass> t1 = Create<TestClass>(3u);
		REQUIRE(TestClass::GetInstanceCount() == 1u);
		REQUIRE(t1->GetData() == 3u);

		// create from raw prohibited
		Ptr<TestClass> p1 = Ptr<TestClass>::CreateFromRaw(t1.Get());
		REQUIRE(t1 != nullptr);
		REQUIRE(p1 == t1);

		Ptr<TestClass> p2 = ToPtr(t1.Get());
		REQUIRE(p2 == t1);
		Ptr<TestClass> p3(ToPtr(t1.Get()));
		REQUIRE(p3 == t1);
		REQUIRE(p2 == p1);
		REQUIRE(p3 == p2);

		REQUIRE(t1 != nullptr);
		REQUIRE(t1->GetData() == 3u);

		t1 = nullptr;
		REQUIRE(TestClass::GetInstanceCount() == 0u);

		// Despite the original pointer being deleted, these wrapper pointers won't know about it and contain invalid pointers
		REQUIRE(p1 != nullptr);
		REQUIRE(p2 != nullptr);
		REQUIRE(p3 != nullptr);
	}

	REQUIRE(TestClass::GetInstanceCount() == 0u);
}
