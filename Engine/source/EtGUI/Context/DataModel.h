#pragma once
#include <RmlUi/Core/DataModelHandle.h>


namespace et {
namespace gui {


//---------------------------------
// I_DataModel
//
// Abstract data model class
//
struct I_DataModel
{
	virtual ~I_DataModel() = default;

	Rml::DataModelHandle m_ModelHandle;
};


} // namespace gui
} // namespace et
