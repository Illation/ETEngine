#pragma once
#include "JSONdom.h"

namespace JSON {
	
//---------------------------------
// Writer
//
// Main class for this project
//
class Writer
{
public:
	Writer(bool compact) : m_Compact(compact) {}
	bool Write(Object const* const rootObj);

	std::string const& GetResult() const { return m_JsonString; }

private:

	bool WriteObject(Object const* const jObj);
	bool WritePair(Pair const& jPair);
	bool WriteValue(Value const* const jVal);

	bool WriteNumber(Number const* const jNum);
	void WriteString(std::string const& str);

	bool WriteArray(Array const* const jArray);

	void WriteIndentations();

	// Data
	///////

	// static const

	static constexpr char s_BeginObject = '{';
	static constexpr char s_EndObject = '}';
	static constexpr char s_BeginArray = '[';
	static constexpr char s_EndArray = ']';
	static constexpr char s_StringScope = '"';
	static constexpr char s_Delimiter = ',';
	static constexpr char s_PairSeparator = ':';

	static constexpr char s_True[] = "true";
	static constexpr char s_False[] = "false";
	static constexpr char s_Null[] = "null";

	static constexpr char s_NewLine = '\n';
	static constexpr char s_Tab = '\t';
	static constexpr char s_Whitespace = ' ';

	// runtime

	std::string m_JsonString;

	uint16 m_IndentationLevel = 0;

	bool m_Compact = false;
	bool m_UseDoubleWhitespace = true;
};

} // namespace JSON
