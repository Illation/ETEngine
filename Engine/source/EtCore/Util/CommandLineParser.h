#pragma once


namespace et {
namespace core {


//-------------------
// CommandLineParser
//
// Class to help interpret command line options
//
class CommandLineParser final
{
	// definitions
	//-------------

	struct Option
	{
	public:
		Option(rttr::type const type) : m_Type(type) {}

		HashString m_Id;
		char m_ShortHand = 0;

		rttr::type m_Type;
		uintptr_t m_Data;

		std::string m_Description;

		bool m_WasSet = false;
	};

	// static access
	//---------------
public:
	static CommandLineParser& Instance();

	// construct destruct
	//--------------------
private:
	CommandLineParser() = default;
	~CommandLineParser() = default;

	// functionality
	//---------------
public:
	template <typename TDataType>
	void RegisterOption(TDataType& data, std::string const& name, std::string const& description, char const shorthand = 0);

	void Process(int32 const argc, char* const argv[]);


	// accessors
	//-----------
	bool WasOptionSet(HashString const optionId) const;

	// utility
	//---------
private:
	void RegisterOptionInternal(uintptr_t const data, rttr::type const type, std::string const& name, char const shorthand, std::string const& desc);
	void IdentifyOption(std::string const& arg, Option*& currentOption);
	void ProcessOption(Option& option, std::string const& arg);


	// Data
	///////

	std::vector<Option> m_Options;
};


} // namespace core
} // namespace et


#include "CommandLineParser.inl"
