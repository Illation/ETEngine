#pragma once


namespace et {
namespace fw {


// fwd
class Archetype;
class EcsController;


//-------------------
// BaseComponentRange
//
// Basic information for a range of components in an archetype
//
class BaseComponentRange 
{
	// definitions
	//-------------
	friend class ComponentView;

	// construct destruct
	//--------------------
protected:
	BaseComponentRange(EcsController* const controller, Archetype* const archetype, size_t const offset, size_t const count);
	virtual ~BaseComponentRange() = default;

	// Data
	///////

private:
	EcsController* const m_Controller = nullptr;
	Archetype* const m_Archetype = nullptr;
	size_t const m_Offset = 0u;
	size_t const m_Count = 0u;
};


//---------------
// ComponentRange
//
// Provides an iteratable view into an archetype - guaranteeing access safety while maintaining fast access
//
template<typename TViewType>
class ComponentRange final : public BaseComponentRange
{
	// definitions
	//-------------
public:

	//---------------
	// iterator
	//
	// iterates over the component range. upon initialization, creates the view type and can only iterate forwards
	//
	class iterator final
	{
		// definitions
		//-------------
		friend class iterator;
	public:
		using iterator_category = std::input_iterator_tag;

		// construct destruct
		//--------------------
		iterator() = default;
		iterator(BaseComponentRange* const range);
		~iterator() { delete m_View; }

		// functionality
		//---------------
		iterator& operator++(); // prefix - postfix is omitted as we do not wish to make a copy of the view type

		// accessors
		//-----------
		bool IsEnd() const;

		bool operator==(iterator const& other) const;

		bool operator!=(iterator const& other) const;

		TViewType& operator*() { return *m_View; }
		TViewType* operator->() { return m_View; }

		// Data
		///////

	private:
		TViewType* m_View = nullptr;
	};

	// construct destruct
	//--------------------
	ComponentRange(EcsController* const controller, Archetype* const archetype, size_t const offset, size_t const count);
	~ComponentRange() = default;

	// accessors
	//-----------
	iterator begin() { return iterator(this); }
	iterator end() { return iterator(); } // null iterator
};


} // namespace fw
} // namespace et


#include "ComponentRange.inl"
