#pragma once
#include "ComponentRegistry.h"
#include "ComponentRange.h"
#include "ComponentSignature.h"


namespace framework {


//---------------
// E_ComponentAccess
//
// Abstract implementation of a system
//
enum class E_ComponentAccess : uint8
{
	Read,
	ReadWrite,
	Exclude,
	Undefined
};


//-------------------
// ComponentView
//
// Iteratable view of a selection of components
//
class ComponentView
{
	// definitions
	//-------------
	struct Accessor
	{
		Accessor(void** pointerRef, T_CompTypeIdx const type, bool const readAccess);

		uint8*& currentElement;
		T_CompTypeIdx const typeIdx = INVALID_COMP_TYPE_IDX;
		bool const read = true;
	};

protected:

	//-------------------
	// ReadAccess
	//
	// Read only component row
	//
	template<typename TComponentType>
	class ReadAccess
	{
		friend class ComponentView;
	public:	
		TComponentType const* operator&() const { return m_Data; }
		TComponentType const& operator*() const { return *m_Data; }
		TComponentType const* operator->() const { return m_Data; }

	private:
		TComponentType* m_Data;
	};

	//-------------------
	// WriteAccess
	//
	// Writable component row
	//
	template<typename TComponentType>
	class WriteAccess
	{
		friend class ComponentView;
	public:
		TComponentType* operator&() { return m_Data; }
		TComponentType& operator*() { return *m_Data; }
		TComponentType* operator->() { return m_Data; }

	private:
		TComponentType* m_Data;
	};

	// construct destruct
	//--------------------
public:
	ComponentView() = default;
	virtual ~ComponentView() = default;

	void Init(BaseComponentRange* const range);

	// accessors
	//-----------
	bool IsEnd() const;
	T_CompTypeList GetTypeList() const;

	// functionality
	//---------------
	bool Next();

	// interface
	//-----------
	virtual void Register() = 0;

protected:
	template<typename TComponentType>
	void Declare(ReadAccess<TComponentType>& read);

	template<typename TComponentType>
	void Declare(WriteAccess<TComponentType>& write);

	// Data
	///////

private:
	std::vector<Accessor> m_Accessors;
	size_t m_Current = 0u;
	BaseComponentRange* m_Range = nullptr;
};


// create a signature from a view
template<typename TViewType>
ComponentSignature SignatureFromView();


} // namespace framework


#include "ComponentView.inl"
