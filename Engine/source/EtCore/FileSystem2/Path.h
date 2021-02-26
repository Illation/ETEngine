#pragma once


namespace et {
namespace core {


//------
// Path
//
// Platform independent path utility
//
class Path
{
	// definitions
	//-------------
	struct Component
	{
		Component(uint16 const start, uint16 const length, std::string const& inPath);

		uint16 const m_Start;
		uint16 const m_Length;
		core::HashString const m_Hash;
	};

	static std::string const s_InvalidString;
	static std::vector<char> const s_Delimiters;

	// static functionality
	//----------------------
	static bool IsDelim(char const c);

public:
	enum class E_Type : uint8
	{
		Relative,
		Absolute,
		AbsoluteVolume,

		Invalid
	};

	// construct destruct
	//--------------------
	Path() = default;
	Path(std::string const& inPath);

	// accessors
	//-----------
	std::string const& Get() const { return m_Impl; }
	core::HashString GetId() const { return m_Id; }

	std::string GetName() const;

	std::string GetRawName() const;
	core::HashString GetRawNameId() const;

	std::string GetExtension() const;
	core::HashString GetExtensionId() const;

	E_Type GetType() const { return m_Type; }
	bool IsFile() const; // or directory

	std::string GetVolume() const;
	core::HashString GetVolumeId() const;


	// Data
	///////

private: 
	std::string m_Impl;
	core::HashString m_Id;

	std::vector<Component> m_Components;

	E_Type m_Type = E_Type::Invalid;
	bool m_HasExtension = false;
};


} // namespace core
} // namespace et
