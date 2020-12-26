#pragma once


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

public:
	std::string const& GetName() const { return m_Name; }
	void SetName(std::string const& val);

	std::string const& GetPath() const { return m_Path; }
	void SetPath(std::string const& val) { m_Path = val; }

	HashString GetId() const;


	// Data
	///////

private:
	// reflected
	std::string m_Name;
	std::string m_Path;

	// derived
	HashString m_Id;
};


} // namespace core
} // namespace et
