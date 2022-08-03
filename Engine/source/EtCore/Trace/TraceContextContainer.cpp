#include "stdafx.h"
#include "TraceContextContainer.h"


namespace et {
namespace core {


//=========================
// Trace Context Container
//=========================


//----------------------------------------
// TraceContextContainer::RegisterContext
//
T_TraceContext TraceContextContainer::RegisterContext(std::string const& name)
{
	T_TraceContext const ret(name.c_str());

	if (m_Contexts.find(ret) != m_Contexts.cend())
	{
		ET_BREAK(); // Can't reliably use asserts because contexts will be registered before asserts are initialized
		return T_TraceContext();
	}

	m_Contexts.emplace(ret, name);
	return ret;
}

//---------------------------------------
// TraceContextContainer::GetContextName
//
std::string const& TraceContextContainer::GetContextName(T_TraceContext const hash) const
{
	static std::string const s_InvalidContextName = "invalid_trace_context";

	auto const foundIt = m_Contexts.find(hash);
	if (foundIt == m_Contexts.cend())
	{
		ET_WARNING("invalid trace context name!");
		return s_InvalidContextName;
	}

	return foundIt->second;
}

//-------------------------------------------
// TraceContextContainer::GetAllContextNames
//
std::vector<std::string> TraceContextContainer::GetAllContextNames() const
{
	std::vector<std::string> ret;
	for (auto const& el : m_Contexts)
	{
		ret.push_back(el.second);
	}

	return ret;
}


} // namespace core
} // namespace et
