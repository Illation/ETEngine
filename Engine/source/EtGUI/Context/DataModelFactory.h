#pragma once
#include "Context.h"
#include "DataModel.h"

#include <RmlUi/Core/DataModelHandle.h>


namespace et {
namespace gui {


//---------------------------------
// DataModelFactory
//
// Container for user defined instancers for data models to attach to GUI contexts
//
class DataModelFactory final
{
	// definitions
	//-------------
public:
	typedef std::function<RefPtr<I_DataModel>(Rml::DataModelConstructor)> T_InstanceFn;

private:
	struct Instancer
	{
		T_InstanceFn m_InstanceFn;
		std::string m_ModelName;
	};

	typedef std::unordered_map<core::HashString, Instancer> T_Instancers;

	// functionality
	//---------------
public:
	RefPtr<I_DataModel> CreateModel(Context& context, core::HashString const modelId) const;
	void GetModelName(core::HashString const modelId, std::string& outModelName) const;

	void RegisterInstancer(std::string const& modelName, T_InstanceFn const& instanceFn);

	// Data
	///////

private:
	T_Instancers m_Instancers;
};


} // namespace gui
} // namespace et
