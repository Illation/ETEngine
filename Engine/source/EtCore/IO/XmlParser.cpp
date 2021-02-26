#include "stdafx.h"
#include "XmlParser.h"

#include <cctype>
#include <locale>


namespace et {
namespace core {

namespace XML {


//---------------
// Parser::c-tor
//
// Parse on construct
//
Parser::Parser(std::string const& sourceText)
{
	MoveToNonWhitespace(sourceText);
	SkipComment(sourceText);


	if ((m_ReadIdx < sourceText.size()) && (sourceText[m_ReadIdx++] == s_TagOpen))
	{
		// Read declaration
		//------------------
		if (sourceText[m_ReadIdx] == s_QuestionMark)
		{
			// declaration start
			++m_ReadIdx;
			if (ReadName(sourceText) != "xml")
			{
				LOG("Expected declaration to start with 'xml', parsing XML failed", Warning);
				return;
			}

			MoveToNonWhitespace(sourceText);
			while (m_ReadIdx < sourceText.size())
			{
				// declaration end
				if (sourceText[m_ReadIdx] == s_QuestionMark)
				{
					if (sourceText[++m_ReadIdx] != s_TagClose)
					{
						LOG(FS("Expected '%c' at end of declaration, parsing XML failed", s_TagClose), Warning);
						return;
					}

					m_ReadIdx++;
					break;
				}

				// declaration attributes
				std::string declarationId(ReadName(sourceText));
				if (declarationId == "version")
				{
					m_Document.m_Version = ReadAttributeContent(sourceText);
				}
				else if (declarationId == "encoding")
				{
					std::string encoding(ReadAttributeContent(sourceText));
					std::transform(encoding.begin(), encoding.end(), encoding.begin(), ::tolower);
					switch (GetHash(encoding))
					{
					case "utf-8"_hash: 
						m_Document.m_Encoding = Document::E_Enconding::UTF_8;
						break;

					case "utf-16"_hash:
						m_Document.m_Encoding = Document::E_Enconding::UTF_16;
						break;

					case "iso-10646-ucs-2"_hash:
						m_Document.m_Encoding = Document::E_Enconding::ISO_10646_UCS_2;
						break;

					case "iso-10646-ucs-4"_hash:
						m_Document.m_Encoding = Document::E_Enconding::ISO_10646_UCS_4;
						break;

					case "iso-8859-1"_hash:
						m_Document.m_Encoding = Document::E_Enconding::ISO_8859_1;
						break;

					case "iso-8859-2"_hash:
						m_Document.m_Encoding = Document::E_Enconding::ISO_8859_2;
						break;

					case "iso-8859-3"_hash:
						m_Document.m_Encoding = Document::E_Enconding::ISO_8859_3;
						break;

					case "iso-8859-4"_hash:
						m_Document.m_Encoding = Document::E_Enconding::ISO_8859_4;
						break;

					case "iso-8859-5"_hash:
						m_Document.m_Encoding = Document::E_Enconding::ISO_8859_5;
						break;

					case "iso-8859-6"_hash:
						m_Document.m_Encoding = Document::E_Enconding::ISO_8859_6;
						break;

					case "iso-8859-7"_hash:
						m_Document.m_Encoding = Document::E_Enconding::ISO_8859_7;
						break;

					case "iso-8859-8"_hash:
						m_Document.m_Encoding = Document::E_Enconding::ISO_8859_8;
						break;

					case "iso-8859-9"_hash:
						m_Document.m_Encoding = Document::E_Enconding::ISO_8859_9;
						break;

					case "iso-2022-jp"_hash:
						m_Document.m_Encoding = Document::E_Enconding::ISO_2022_JP;
						break;

					case "shift_jis"_hash:
						m_Document.m_Encoding = Document::E_Enconding::Shift_JIS;
						break;

					case "euc-jp"_hash:
						m_Document.m_Encoding = Document::E_Enconding::EUC_JP;
						break;

					default:
						LOG(FS("Unrecognized encoding '%s' declared, parsing XML failed", encoding.c_str()), Warning);
						return;
					}
				}
				else if (declarationId == "standalone")
				{
					std::string const standalone(ReadAttributeContent(sourceText));
					if (standalone == "yes")
					{
						m_Document.m_IsStandalone = true;
					}
					else
					{
						ET_ASSERT(standalone == "no");
						m_Document.m_IsStandalone = true;
					}
				}
				else 
				{
					LOG("Unhandled declaration attribute, parsing XML failed", Warning);
					return;
				}

				MoveToNonWhitespace(sourceText);
			}

			// now move on to content start
			MoveToNonWhitespace(sourceText);
			SkipComment(sourceText);
			if ((m_ReadIdx > sourceText.size()) || (sourceText[m_ReadIdx++] != s_TagOpen))
			{
				LOG(FS("Expected '%c' token, parsing XML failed", s_TagOpen), Warning);
				return;
			}
		}

		// Read content
		//--------------

		if (m_Document.m_Encoding != Document::E_Enconding::UTF_8)
		{
			LOG("Currently only UTF-8 encoding is supported, aborting XML parsing", Warning);
			return;
		}

		ReadElement(sourceText, m_Document.m_Root);
		return;
	}

	LOG(FS("Expected '%c' token, parsing XML failed", s_TagOpen), Warning);
}

//-----------------------------
// Parser::MoveToNonWhitespace
//
void Parser::MoveToNonWhitespace(std::string const& sourceText)
{
	if (!std::isspace(sourceText[m_ReadIdx]))
	{
		return;
	}

	auto const it = std::find_if(sourceText.cbegin() + m_ReadIdx, sourceText.cend(), [](const uint8 c)
		{
			return !std::isspace(c);
		});

	if (it == sourceText.cend())
	{
		m_Completed = true;
		m_ReadIdx = 0u;
		return;
	}

	m_ReadIdx = static_cast<size_t>(it - sourceText.cbegin());
}

//---------------------
// Parser::SkipComment
//
void Parser::SkipComment(std::string const& sourceText)
{
	// find out if we are inside a comment
	if (sourceText[m_ReadIdx] != s_TagOpen)
	{
		return;
	}

	if (sourceText[m_ReadIdx + 1u] != s_Exclamation)
	{
		return;
	}

	if (sourceText[m_ReadIdx + 2u] != s_Dash)
	{
		return;
	}

	if (sourceText[m_ReadIdx + 3u] != s_Dash)
	{
		return;
	}

	// we're in a comment, skip to the end of it
	m_ReadIdx += 4u;
	while (m_ReadIdx < sourceText.size())
	{
		if (sourceText[m_ReadIdx++] != s_Dash)
		{
			continue;
		}

		if (sourceText.size() < m_ReadIdx + 2u)
		{
			LOG("XML file wasn't long enough to exit comment, exiting", Warning);
			return;
		}

		if (sourceText[m_ReadIdx++] != s_Dash)
		{
			continue;
		}

		if (sourceText[m_ReadIdx++] == s_TagClose)
		{
			MoveToNonWhitespace(sourceText);
			SkipComment(sourceText); // there could be a second comment after the first, so we keep recurssively skipping comments until no more where found
			return;
		}
	}

	LOG("XML reached end of document before exiting comment", Warning);
	return;
}

//--------------------
// Parser::ReadName
//
// Read a single one word name
//
std::string Parser::ReadName(std::string const& sourceText)
{
	std::string ret;
	while (m_ReadIdx < sourceText.size())
	{
		char const c = sourceText[m_ReadIdx];
		if (std::isspace(c) || (c == s_Equal) || (c == s_EndElement) || (c == s_TagClose))
		{
			break;
		}
		else if (c == s_EntityStart)
		{
			ret += ReadEntity(sourceText);
		}
		else
		{
			ret += c;
		}

		m_ReadIdx++;
	}

	return ret;
}

//--------------------
// Parser::ReadEntity
//
// Convert a character reference into a character
//  - ends on the entity end delimeter
//
char Parser::ReadEntity(std::string const& sourceText)
{
	ET_ASSERT(sourceText[m_ReadIdx - 1] == s_EntityStart);

	// entity type
	//-------------
	enum class E_Mode
	{
		Named,
		Numeric,
		Hex
	} mode = E_Mode::Named;

	if (m_ReadIdx + 3u >= sourceText.size())
	{
		LOG("Source text is too short to read an entity, aborting!", Warning);
		return 0u;
	}

	if (sourceText[m_ReadIdx] == s_NumericEntity)
	{
		if (sourceText[++m_ReadIdx] == s_HexEntity)
		{
			mode = E_Mode::Hex;
			++m_ReadIdx;
		}
		else
		{
			mode = E_Mode::Numeric;
		}
	}

	// read the entity
	//-----------------
	std::string content;
	{
		char next = sourceText[m_ReadIdx];
		while (next != s_EntityEnd)
		{
			content += next; 

			++m_ReadIdx;
			if (m_ReadIdx >= sourceText.size())
			{
				LOG("Source text is too short to read an entity, aborting!", Warning);
				return 0u;
			}

			next = sourceText[m_ReadIdx];
		}
	}

	m_ReadIdx++;

	// convert it
	//------------
	switch (mode)
	{
	case E_Mode::Named:
		switch (GetHash(content))
		{
		case "quot"_hash: return s_Quote;
		case "amp"_hash: return s_EntityStart;
		case "apos"_hash: return s_Apostrophe;
		case "lt"_hash: return s_TagOpen;
		case "gt"_hash: return s_TagClose;

		default:
			LOG("Unsupported named entity", Warning);
			return 0u;
		}
		
		break;

	case E_Mode::Numeric:
		return static_cast<char>(std::stoul(content));
		break;

	case E_Mode::Hex:
		return static_cast<char>(std::stoul(content, nullptr, 16));
		break;
	}

	return 0u;
}

//------------------------------
// Parser::ReadAttributeContent
//
std::string Parser::ReadAttributeContent(std::string const& sourceText)
{
	MoveToNonWhitespace(sourceText);
	if ((m_ReadIdx >= sourceText.size()) || (sourceText[m_ReadIdx++] != s_Equal))
	{
		LOG(FS("Expected attribute starting with '%c'", s_Equal), Warning);
		return "";
	}

	MoveToNonWhitespace(sourceText);
	if ((m_ReadIdx >= sourceText.size()) || (sourceText[m_ReadIdx++] != s_Quote))
	{
		LOG(FS("Expected attribute starting with '%c'", s_Quote), Warning);
		return "";
	}

	std::string ret;
	while (m_ReadIdx < sourceText.size())
	{
		char const c = sourceText[m_ReadIdx++];
		if (c == s_Quote)
		{
			break;
		}
		else if (c == s_EntityStart)
		{
			ret += ReadEntity(sourceText);
		}
		else
		{
			ret += c;
		}
	}

	return ret;
}

//---------------------
// Parser::ReadElement
//
// Recursively read XML elements
//
void Parser::ReadElement(std::string const& sourceText, Element& el)
{
	el.m_Name = core::HashString(ReadName(sourceText).c_str());

	MoveToNonWhitespace(sourceText); 
	while (m_ReadIdx < sourceText.size())
	{
		// empty element end
		if (sourceText[m_ReadIdx] == s_EndElement)
		{
			if (++m_ReadIdx >= sourceText.size())
			{
				LOG("Source text is too short to read an element, aborting!", Warning);
				return;
			}

			if (sourceText[m_ReadIdx] != s_TagClose)
			{
				LOG(FS("Expected '%c' at end of declaration, parsing XML failed", s_TagClose), Warning);
				return;
			}

			m_ReadIdx++;
			return;
		}
		else if (sourceText[m_ReadIdx] == s_TagClose) // element content
		{
			m_ReadIdx++;

			size_t const cachedPos = m_ReadIdx;
			MoveToNonWhitespace(sourceText);
			SkipComment(sourceText);

			if (sourceText[m_ReadIdx++] != s_TagOpen) // leaf element gets string filled in
			{
				el.m_Value = sourceText.substr(cachedPos, m_ReadIdx - cachedPos);
				while (true)
				{
					if (m_ReadIdx >= sourceText.size())
					{
						LOG("Source text ended early!", Warning);
						return;
					}

					char const c = sourceText[m_ReadIdx++];
					if (c == s_TagOpen)
					{
						break;
					}
					else if (c == s_EntityStart)
					{
						el.m_Value += ReadEntity(sourceText);
					}
					else
					{
						el.m_Value += c;
					}
				}

				if (sourceText[m_ReadIdx++] != s_EndElement)
				{
					LOG("Source text ended early!", Warning);
					return;
				}

				MoveToNonWhitespace(sourceText);
				if (m_ReadIdx >= sourceText.size())
				{
					LOG("Source text is too short to read an element, aborting!", Warning);
					return;
				}

				if (core::HashString(ReadName(sourceText).c_str()) != el.m_Name)
				{
					LOG("Incorrect element closure!", Warning);
				}

				MoveToNonWhitespace(sourceText);
				if ((m_ReadIdx >= sourceText.size()) || (sourceText[m_ReadIdx++] != s_TagClose))
				{
					LOG(FS("Incorrect element closure, expected '%c'", s_TagClose), Warning);
				}

				return;
			}

			// otherwise we fill in children recursively
			while (sourceText[m_ReadIdx] != s_EndElement)
			{
				Element child;
				ReadElement(sourceText, child);
				el.m_Children.push_back(child);

				MoveToNonWhitespace(sourceText);
				SkipComment(sourceText);
				if ((m_ReadIdx >= sourceText.size()) || (sourceText[m_ReadIdx++] != s_TagOpen))
				{
					LOG("Expected opening tag after reading child", Warning);
					return;
				}
			}

			++m_ReadIdx;
			MoveToNonWhitespace(sourceText);
			if (m_ReadIdx >= sourceText.size())
			{
				LOG("Source text is too short to read an element, aborting!", Warning);
				return;
			}

			if (core::HashString(ReadName(sourceText).c_str()) != el.m_Name)
			{
				LOG("Incorrect element closure!", Warning);
			}

			MoveToNonWhitespace(sourceText);
			if (m_ReadIdx >= sourceText.size())
			{
				LOG("Source text is too short to read an element, aborting!", Warning);
				return;
			}

			if (sourceText[m_ReadIdx++] != s_TagClose)
			{
				LOG(FS("Incorrect element closure, expected '%c'", s_TagClose), Warning);
			}

			return;
		}

		// attributes

		Attribute attr;
		attr.m_Name = core::HashString(ReadName(sourceText).c_str());
		attr.m_Value = ReadAttributeContent(sourceText);
		el.m_Attributes.push_back(attr);

		MoveToNonWhitespace(sourceText);
	}

	LOG("Reading element ended unexpectedly!", Warning);
}


} // namespace XML

} // namespace core
} // namespace et
