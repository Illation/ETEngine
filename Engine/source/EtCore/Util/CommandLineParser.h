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

	typedef bool(*T_VariantAssignFn)(uintptr_t const, rttr::variant const);

	struct Option
	{
		Option(rttr::type const type) : m_Type(type) {}

		HashString m_Id;
		char m_ShortHand = 0;

		rttr::type m_Type;
		uintptr_t m_Data;
		T_VariantAssignFn m_AssignFn;

		std::string m_Description;

		bool m_WasSet = false;
		std::string m_ProcessedArg;
	};

public:

	struct RegisterHelper
	{
		template <typename TDataType>
		RegisterHelper(TDataType& data, std::string const& name, std::string const& description, char const shorthand);
	};

	// static access
	//---------------
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
	void RegisterOption(TDataType& data, std::string const& name, std::string const& description, char const shorthand);

	void Process(int32 const argc, char* const argv[]);

	void PrintAll(bool const showState) const;


	// accessors
	//-----------
	template <typename TDataType>
	bool WasOptionSet(TDataType const& data) const;

	bool HasProcessed() const { return m_HasProcessed; }

	// utility
	//---------
private:
	void RegisterOptionInternal(uintptr_t const data, 
		rttr::type const type, 
		T_VariantAssignFn const assignFn, 
		std::string const& name, 
		char const shorthand, 
		std::string const& desc);

	void IdentifyOption(std::string const& arg, Option*& currentOption);
	bool ProcessOption(Option& option, std::string const& arg);
	bool ProcessArithmeticOption(Option &option, std::string const &arg, HashString const typeId);
	void ProcessVectorOption(Option &option, std::string const &arg, HashString const typeId);
	std::vector<std::string> SeparateListArg(std::string const &arg) const;

	template <uint8 TCount, typename TDataType>
	bool ProcessVectorInternal(std::vector<std::string> const& args, char const* const format, math::vector<TCount, TDataType>& vec) const;


	// Data
	///////

	bool m_HasProcessed = false;
	std::vector<Option> m_Options;
};


} // namespace core
} // namespace et


#include "CommandLineParser.inl"
