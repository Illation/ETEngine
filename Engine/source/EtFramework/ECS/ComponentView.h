#pragma once
#include "ComponentRegistry.h"
#include "ComponentRange.h"


namespace framework {


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
		Accessor(void*& pointerRef, T_CompTypeIdx const type, bool const readAccess);

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
		TComponentType const& operator->() const { return *m_Data; }

	private:
		TComponentType const* m_Data;
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
		TComponentType& operator->() { return *m_Data; }

	private:
		TComponentType* m_Data;
	};

	// construct destruct
	//--------------------
public:
	ComponentView(BaseComponentRange* const range);

	// accessors
	//-----------
	bool IsEnd() const { return m_Current >= m_Max; }

	// functionality
	//---------------
	bool Next();

	// interface
	//-----------
protected:
	virtual void Register() = 0;

	template<typename TComponentType>
	void Register(ReadAccess<TComponentType>& read);

	template<typename TComponentType>
	void Register(WriteAccess<TComponentType>& write);


	// Data
	///////

private:
	std::vector<Accessor> m_Accessors;
	size_t m_Current = 0u;
	size_t const m_Max;
};


} // namespace framework


#include "ComponentView.inl"
