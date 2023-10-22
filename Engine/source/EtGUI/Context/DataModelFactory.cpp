#include "stdafx.h"
#include "DataModelFactory.h"

#include <RmlUi/Core/Context.h>


namespace et {
namespace gui {


//====================
// Data Model Factory
//====================


//---------------------------------
// DataModelFactory::CreateModel
//
// Instanciate a data model for a context
//
RefPtr<I_DataModel> DataModelFactory::CreateModel(Rml::Context& context, core::HashString const modelId) const
{
	T_Instancers::const_iterator const foundInstancerIt = m_Instancers.find(modelId);
	ET_ASSERT(foundInstancerIt != m_Instancers.cend(), "No data model instancer '%s' found", modelId.ToStringDbg());
	Instancer const& instancer = foundInstancerIt->second;

	if (Rml::DataModelConstructor modelConstructor = context.CreateDataModel(instancer.m_ModelName))
	{
		RefPtr<I_DataModel> ret = instancer.m_InstanceFn(modelConstructor);
		ret->m_ModelHandle = modelConstructor.GetModelHandle();
		return std::move(ret);
	}

	return nullptr;
}

//---------------------------------
// DataModelFactory::GetModelName
//
void DataModelFactory::GetModelName(core::HashString const modelId, std::string& outModelName) const
{
	T_Instancers::const_iterator const foundInstancerIt = m_Instancers.find(modelId);
	ET_ASSERT(foundInstancerIt != m_Instancers.cend(), "No data model instancer '%s' found", modelId.ToStringDbg());
	Instancer const& instancer = foundInstancerIt->second;

	outModelName = instancer.m_ModelName;
}

//-------------------------------------
// DataModelFactory::RegisterInstancer
//
// Register a new instancer function to the factory
//
void DataModelFactory::RegisterInstancer(std::string const& modelName, T_InstanceFn const& instanceFn)
{
	core::HashString const instancerId(modelName.c_str());
	ET_ASSERT(m_Instancers.find(instancerId) == m_Instancers.cend());

	std::pair<T_Instancers::iterator, bool> res = m_Instancers.emplace(instancerId, Instancer());
	ET_ASSERT(res.second);

	res.first->second.m_InstanceFn = instanceFn;
	res.first->second.m_ModelName = modelName;
}


} // namespace gui
} // namespace et
