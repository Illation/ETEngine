#pragma once
#include <EtCore/Reflection/Registration.h>


namespace et {
	REGISTRATION_NS(core);
}


namespace et {
namespace core {


//---------------------------------
// PackageDescriptor
//
// Serializable information about packages
//
class PackageDescriptor final
{
	RTTR_ENABLE()
	REGISTRATION_FRIEND_NS(core)

public:
	// accessors
	//-----------
	std::string const& GetName() const { return m_Name; }
	std::string const& GetPath() const { return m_Path; }
	bool IsRuntime() const { return m_IsRuntime; }
	HashString GetId() const;

	// utility
	//---------
private:
	void SetName(std::string const& val);

	// Data
	///////

	// reflected
	std::string m_Name;
	std::string m_Path;
	bool m_IsRuntime = true;

	// derived
	HashString m_Id;
};


} // namespace core
} // namespace et
