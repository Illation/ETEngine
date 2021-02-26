#pragma once
#include "XmlDom.h"


namespace et {
namespace core {

namespace XML {


//--------
// Parser
//
// Converts XML text into an XML document object model.
//  - the goal is not completeness, but having a minimal viable solution for most use cases
//
class Parser final
{
	// definitions
	//-------------
	static constexpr char s_TagOpen = '<';
	static constexpr char s_TagClose = '>';
	static constexpr char s_EndElement = '/';
	static constexpr char s_Equal = '=';
	static constexpr char s_Quote = '"';
	static constexpr char s_Apostrophe = '\'';
	static constexpr char s_QuestionMark = '?';
	static constexpr char s_EntityStart = '&';
	static constexpr char s_EntityEnd = ';';
	static constexpr char s_NumericEntity = '#';
	static constexpr char s_HexEntity = 'x';
	static constexpr char s_Exclamation = '!';
	static constexpr char s_Dash = '-';

public:
	Parser(std::string const& sourceText);
	~Parser() = default;

	// accessors
	//-----------
	Document const& GetDocument() const { return m_Document; }

	// utility
	//---------
private:
	void MoveToNonWhitespace(std::string const& sourceText);
	void SkipComment(std::string const& sourceText);
	std::string ReadName(std::string const& sourceText);
	char ReadEntity(std::string const& sourceText);
	std::string ReadAttributeContent(std::string const& sourceText);
	void ReadElement(std::string const& sourceText, Element& el);

	// Data
	///////

	Document m_Document;
	size_t m_ReadIdx = 0u;
	bool m_Completed = false;
};


} // namespace XML

} // namespace core
} // namespace et
