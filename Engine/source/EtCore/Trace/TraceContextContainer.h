#pragma once
#include "TraceFwd.h"


namespace et {
namespace core {


//-----------------------
// TraceContextContainer
//
struct TraceContextContainer
{
	T_TraceContext RegisterContext(std::string const& name);
	std::string const& GetContextName(T_TraceContext const hash) const;

	std::vector<std::string> GetAllContextNames() const;
	size_t GetContextCount() const { return m_Contexts.size(); }

private:
	std::unordered_map<T_TraceContext, std::string> m_Contexts;
};


} // namespace core
} // namespace et
