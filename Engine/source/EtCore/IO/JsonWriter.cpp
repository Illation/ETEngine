#include "stdafx.h"
#include "JsonWriter.h"


namespace et {
namespace core {


namespace JSON {


//---------------------------------
// Writer::Write
//
// Write the json root object to the internal string, which gets cleared before
//
bool Writer::Write(Object const* const rootObj)
{
	if (rootObj == nullptr)
	{
		LOG("JSON::Writer::Write > Failed to write JSON to string, root object was null", LogLevel::Warning);
		return false;
	}

	m_JsonString.clear();
	return WriteObject(rootObj);
}

//---------------------------------
// Writer::WriteObject
//
// Write the pairs of an object
//
bool Writer::WriteObject(Object const* const jObj)
{
	if (jObj == nullptr)
	{
		LOG("JSON::Writer::WriteObject > Failed to write Object to string, object was null", LogLevel::Warning);
		return false;
	}

	// begin the object
	m_JsonString += s_BeginObject;

	if (!m_Compact)
	{
		m_JsonString += s_NewLine;
		++m_IndentationLevel;
	}

	// write the pairs
	bool allPairsSucceeded = true;
	size_t pairIndex = 0;
	if (!(jObj->value.empty()))
	{
		while (true)
		{
			// Write the pair
			if (!WritePair(jObj->value[pairIndex]))
			{
				LOG("JSON::Writer::WriteObject > Failed to write pair.", LogLevel::Warning);
				allPairsSucceeded = false;
			}

			// check if there will be a successor
			++pairIndex;
			bool isNotLast = pairIndex < jObj->value.size();

			// write delimiter
			if (isNotLast)
			{
				m_JsonString += s_Delimiter;
			}

			// write newline
			if (!m_Compact)
			{
				m_JsonString += s_NewLine;
			}

			if (!isNotLast)
			{
				break;
			}
		}
	}

	// end the object
	if (!m_Compact)
	{
		--m_IndentationLevel;
		WriteIndentations();
	}

	m_JsonString += s_EndObject;

	// return success
	return allPairsSucceeded;
}

//---------------------------------
// Writer::WritePair
//
// Write a pair
//
bool Writer::WritePair(Pair const& jPair)
{
	if (!m_Compact)
	{
		WriteIndentations();
	}

	// key
	WriteString(jPair.first);

	m_JsonString += s_PairSeparator;

	// value
	if (!m_Compact)
	{
		m_JsonString += s_Whitespace;
	}
	return WriteValue(jPair.second);
}

//---------------------------------
// Writer::WriteValue
//
// Write a value, whatever it may be
//
bool Writer::WriteValue(Value const* const jVal)
{
	if (jVal == nullptr)
	{
		LOG("JSON::Writer::WriteObject > Failed to write value to string, value was null", LogLevel::Warning);
		return false;
	}

	// do something depending on the type
	switch (jVal->GetType())
	{
	case ValueType::JSON_Null:
		m_JsonString += s_Null;
		break;
	case ValueType::JSON_Bool:
		m_JsonString += static_cast<Bool const*>(jVal)->value ? s_True : s_False;
		break;

	case ValueType::JSON_Number:
		return WriteNumber(static_cast<Number const*>(jVal));
	case ValueType::JSON_String:
		WriteString(static_cast<String const*>(jVal)->value);
		break;

	case ValueType::JSON_Array:
		return WriteArray(static_cast<Array const*>(jVal));
	case ValueType::JSON_Object:
		return WriteObject(static_cast<Object const*>(jVal));
	}

	return true;
}

//---------------------------------
// Writer::WriteNumber
//
// Write a number
//
bool Writer::WriteNumber(Number const* const jNum)
{
	if (jNum == nullptr)
	{
		LOG("JSON::Writer::WriteNumber > Failed to write json number to string, number object was null", LogLevel::Warning);
		return false;
	}

	// use either the double or int val. if its a double the ostream should automatically convert to scientific notation if necessary
	if (jNum->isInt)
	{
		m_JsonString += std::to_string(jNum->valueInt);
	}
	else
	{
		std::ostringstream streamObj;
		streamObj << jNum->value;

		std::string numString = streamObj.str();
		if (numString.find('.') == std::string::npos)
		{
			numString += ".0";
		}

		m_JsonString += numString;
	}
	
	return true;
}

//---------------------------------
// Writer::WriteString
//
// Write a string and decode escape sequences
//
void Writer::WriteString(std::string const& str)
{
	m_JsonString += s_StringScope;
	
	for (char const character : str)
	{
		switch (character)
		{
		case '\"':
			m_JsonString += "\\\"";
			break;
		case '\\':
			m_JsonString += "\\\\";
			break;
		case '\b':
			m_JsonString += "\\b";
			break;
		case '\f':
			m_JsonString += "\\f";
			break;
		case '\n':
			m_JsonString += "\\n";
			break;
		case '\r':
			m_JsonString += "\\r";
			break;
		case '\t':
			m_JsonString += "\\t";
			break;
		default:
			m_JsonString += character; // since we only have ascii strings internally we don't have to worry about writing unicode characters
			break;
		}
	}

	m_JsonString += s_StringScope;
}

//---------------------------------
// Writer::WriteArray
//
// Write an array
//
bool Writer::WriteArray(Array const* const jArray)
{
	if (jArray == nullptr)
	{
		LOG("JSON::Writer::WriteArray > Failed to write json array to string, array object was null", LogLevel::Warning);
		return false;
	}

	// begin the array
	m_JsonString += s_BeginArray;

	// if the array is empty, no further processing is needed
	if (jArray->value.size() == 0u)
	{
		m_JsonString += s_EndArray;
		return true;
	}
	
	// if this is an array of complex types and we are not writing a compact json file write a new line per element
	bool writeNlPerElement = false;
	if (!m_Compact)
	{
		auto const foundComplexTypeIt = std::find_if(jArray->value.cbegin(), jArray->value.cend(), [](Value const* const val)
		{
			return val->GetType() == ValueType::JSON_Array || val->GetType() == ValueType::JSON_Object || val->GetType() == ValueType::JSON_String;
		});

		if (foundComplexTypeIt != jArray->value.cend())
		{
			writeNlPerElement = true;

			m_JsonString += s_NewLine;
			++m_IndentationLevel;
		}
	}

	// write the values
	bool allValuesSucceeded = true;
	size_t idx = 0;
	while (true)
	{
		if (writeNlPerElement)
		{
			WriteIndentations();
		}

		// Write the pair
		if (!WriteValue(jArray->value[idx]))
		{
			LOG("JSON::Writer::WriteArray > Failed to write value at index #" + std::to_string(idx) + std::string("!"), LogLevel::Warning);
			allValuesSucceeded = false;
		}

		// check if there will be a successor
		++idx;
		bool isNotLast = idx < jArray->value.size();

		// write delimiter
		if (isNotLast)
		{
			m_JsonString += s_Delimiter;
		}

		// write newline or whitespace for non complex arrays
		if (!m_Compact)
		{
			if (writeNlPerElement)
			{
				m_JsonString += s_NewLine;
			}
			else
			{
				m_JsonString += s_Whitespace;
			}
		}

		if (!isNotLast)
		{
			break;
		}
	}

	// end the array
	if (writeNlPerElement)
	{
		--m_IndentationLevel;
		WriteIndentations();
	}

	m_JsonString += s_EndArray;

	return allValuesSucceeded;
}

//---------------------------------
// Writer::WriteIndentations
//
// Write indentations to the file up to the indentation level
//
void Writer::WriteIndentations()
{
	for (uint16 i = 0; i < m_IndentationLevel; ++i)
	{
		if (m_UseDoubleWhitespace)
		{
			m_JsonString += s_Whitespace;
			m_JsonString += s_Whitespace;
		}
		else
		{
			m_JsonString += s_Tab;
		}
	}
}


} // namespace JSON

} // namespace core
} // namespace et
