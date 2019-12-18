#pragma once


namespace framework {


// fwd
class Archetype;


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
public:
	BaseComponentRange(Archetype* const archetype, size_t const offset, size_t const count)
		: m_Archetype(archetype)
		, m_Offset(offset)
		, m_Count(count)
	{
		ET_ASSERT(m_Archetype != nullptr);
		ET_ASSERT(offset + count < m_Archetype->GetSize());
	}

	virtual ~BaseComponentRange() = 0; // make BaseComponentRange abstract

	// Data
	///////

private:
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
		iterator(BaseComponentRange* const range) : m_View(new TViewType(range)) {}
		~iterator() { delete m_View; }

		// functionality
		//---------------
		iterator& operator++() // prefix - postfix is omitted as we do not wish to make a copy of the view type
		{
			m_View->Next(); 
			return *this;
		}

		// accessors
		//-----------
		bool IsEnd() const
		{
			return ((m_View == nullptr) || m_View->IsEnd());
		}

		bool operator==(iterator const& other) const
		{
			return (IsEnd() ? other.IsEnd() : (m_View == other.m_View));
		}

		bool operator!=(iterator const& other) const
		{
			return !(*this == other);
		}

		TViewType& operator*() { return *m_View; }
		TViewType* operator->() { return m_View; }

		// Data
		///////

	private:
		TViewType* m_View = nullptr;
	};

	// construct destruct
	//--------------------
	ComponentRange(Archetype* const archetype, size_t const offset, size_t const count) : BaseComponentRange(archetype, offset, count) {}
	~ComponentRange() = default;

	// accessors
	//-----------
	iterator begin() { return iterator(this); }
	iterator end() { return iterator(); }
};


} // namespace framework

