#include "stdafx.hpp"
#include "JSONparser.h"

#include <functional>
#include <cctype>
#include <locale>

JSON::Parser::Parser(const std::string &textFile)
{
	MoveToNonWhitespace(textFile);
	if (!m_Completed && ReadToken(textFile) == JT_BeginObject)
	{
		m_Root = ParseObject(textFile);
		return;
	}
	LOG("Expected '{' token, parsing JSON failed", Warning);
}

JSON::Parser::~Parser()
{
	delete m_Root;
	m_Root = nullptr;
}

void JSON::Parser::MoveToNonWhitespace(const std::string &textFile)
{
	if (!std::isspace(textFile[m_ReadIdx]))return;
	auto it = std::find_if(textFile.begin()+m_ReadIdx, textFile.end(), [](const uint8 c) { return !std::isspace(c); });
	if (it == textFile.end())
	{
		m_Completed = true;
		m_ReadIdx = 0;
		return;
	}
	m_ReadIdx = (uint32)(it - textFile.begin());
}

JSON::Parser::Token JSON::Parser::ReadToken(const std::string &textFile)
{
	switch (textFile[m_ReadIdx++])
	{
	case '{': return JT_BeginObject;
	case '}': return JT_EndObject;
	case '[': return JT_BeginArray;
	case ']': return JT_EndArray;
	case '"': return JT_String;
	case ',': return JT_Delim;
	case ':': return JT_Value;
	case 't': if (m_ReadIdx + 4 > (uint32)textFile.size()) { m_Completed = true; return JT_Undefined; }
		if (textFile.substr(m_ReadIdx, 3) == "rue")
		{
			m_ReadIdx += 3;
			return JT_True;
		}
		return JT_Undefined;
	case 'f': if (m_ReadIdx + 5 > (uint32)textFile.size()) { m_Completed = true; return JT_Undefined; }
		if (textFile.substr(m_ReadIdx, 4) == "alse")
		{
			m_ReadIdx += 4;
			return JT_False;
		}
		return JT_Undefined;
	case 'n': if (m_ReadIdx + 4 > (uint32)textFile.size()) { m_Completed = true; return JT_Undefined; }
		if (textFile.substr(m_ReadIdx, 3) == "ull")
		{
			m_ReadIdx += 3;
			return JT_Null;
		}
		return JT_Undefined;
	default: 
		m_ReadIdx--;
		if (textFile[m_ReadIdx] == '-' || std::isdigit(textFile[m_ReadIdx])) return JT_Numeric;
		return JT_Undefined;
	}
}

JSON::Object* JSON::Parser::ParseObject(const std::string & textFile)
{
	JSON::Object* ret = new JSON::Object;
	bool parseSuccess = false;
	bool parseFail = false;
	bool prevPair = false;
	bool prevDelim = false;
	while (!m_Completed && !parseSuccess && !parseFail)
	{
		MoveToNonWhitespace(textFile);
		if(CheckEOF(textFile))continue;
		Token token = ReadToken(textFile);
		if(!(token == JT_EndObject) && CheckEOF(textFile))continue;
		switch (token)
		{
		case JT_EndObject:
			if(prevDelim)
			{
				LOG("Expected a new Key Value pair after delimiter", Warning);
				parseFail = true;
				continue;
			}
			parseSuccess = true;
			break;
		case JT_String:
		{
			if (prevPair)
			{
				LOG("Expected a delimiter before new key value pair", Warning);
				parseFail = true;
				continue;
			}
			prevDelim = false;
			JSON::Pair keyVal = ParsePair(textFile);
			if (keyVal.second)
			{
				ret->value.push_back(keyVal);
				prevPair = true;
				continue;
			}
			parseFail = true;
			continue;
		}
		case JT_Delim:
			if (!prevPair)
			{
				LOG("Expected a new Key Value pair before delimiter", Warning);
				parseFail = true;
				continue;
			}
			prevPair = false;
			prevDelim = true;
			continue;
		default:
			parseFail = true;
			continue;
		}
	}
	if (!parseSuccess)
	{
		delete ret;
		LOG("Couldn't successfully parse object", Warning);
		return nullptr;
	}
	return ret;
}

JSON::Pair JSON::Parser::ParsePair(const std::string & textFile)
{
	JSON::Pair ret;
	ret.second = nullptr;
	if (ParseString(textFile, ret.first))
	{
		MoveToNonWhitespace(textFile);
		if (!CheckEOF(textFile) && ReadToken(textFile) == JT_Value)
		{
			MoveToNonWhitespace(textFile);
			if (!CheckEOF(textFile)) ret.second = ParseValue(textFile);
		}
	}
	return ret;
}

JSON::Value* JSON::Parser::ParseValue(const std::string & textFile)
{
	MoveToNonWhitespace(textFile);
	if(CheckEOF(textFile))return nullptr;
	Token token = ReadToken(textFile);
	if(CheckEOF(textFile))return nullptr;

	switch (token)
	{
	case JT_String:
	{
		JSON::String* ret = new JSON::String();
		if (!ParseString(textFile, ret->value))
		{
			delete ret;
			return nullptr;
		}
		return ret;
	}
	case JT_Numeric:
		return ParseNumber(textFile);
	case JT_BeginObject:
		return ParseObject(textFile);
	case JT_BeginArray:
		return ParseArray(textFile);
	case JT_True:
	case JT_False:
	{
		JSON::Bool* ret = new JSON::Bool();
		ret->value = token == JT_True;
		return ret;
	}
	case JT_Null:
		return new JSON::Value();
	}

	LOG("Couldn't successfully parse value, unexpected token", Warning);
	return nullptr;
}

bool JSON::Parser::ParseString(const std::string & textFile, std::string &parsed)
{
	while (!m_Completed)
	{
		if (CheckEOF(textFile))return false;
		char next = textFile[m_ReadIdx++];
		if (next == '"')
		{
			//m_ReadIdx++;
			return true;
		}
		else if (next == '\\')
		{
			if (CheckEOF(textFile))return false;
			next = textFile[m_ReadIdx++];
			switch (next)
			{
			case '\"': parsed += '\"'; break;
			case '\\': parsed += '\\'; break;
			case '/': parsed += '/'; break;
			case 'b': parsed += '\b'; break;
			case 'f': parsed += '\f'; break;
			case 'n': parsed += '\n'; break;
			case 'r': parsed += '\r'; break;
			case 't': parsed += '\t'; break;
			case 'u': 
			{
				if (m_ReadIdx + 4 > (uint32)textFile.size()) return false;
				std::string hexString = "0x" + textFile.substr(m_ReadIdx, 4);
				parsed += static_cast<char>(std::stoul(hexString, nullptr, 16));
				m_ReadIdx += 4;
			} 
			break;
			default:
				LOG("unexpected symbol after escape character while parsing string", Warning);
				return false;
			}
		}
		else
		{
			parsed += next;
		}
	}
	return false;
}

JSON::Array* JSON::Parser::ParseArray(const std::string & textFile)
{
	JSON::Array* ret = new JSON::Array;
	bool parseSuccess = false;
	bool parseFail = false;
	bool prevVal = false;
	bool prevDelim = false;
	while (!m_Completed && !parseSuccess && !parseFail)
	{
		MoveToNonWhitespace(textFile);
		if(CheckEOF(textFile))continue;
		Token token = ReadToken(textFile);
		if(!(token == JT_Delim) && !(token == JT_EndArray) && !(token == JT_Numeric))m_ReadIdx--;
		if(CheckEOF(textFile))continue;
		switch (token)
		{
		case JT_EndArray:
			if(prevDelim)
			{
				LOG("Expected a new Key Value pair after delimiter", Warning);
				parseFail = true;
				continue;
			}
			parseSuccess = true;
			break;
		case JT_Delim:
			if (!prevVal)
			{
				LOG("Expected a new Key Value pair before delimiter", Warning);
				parseFail = true;
				continue;
			}
			prevVal = false;
			prevDelim = true;
			continue;
		default:
			if (!prevVal)
			{
				JSON::Value* val = ParseValue(textFile);
				prevDelim = false;
				if (val)
				{
					ret->value.push_back(val);
					prevVal = true;
					continue;
				}
			}
			else 
				LOG("Expected a delimiter before new Key Value pair", Warning);
			parseFail = true;
			continue;
		}
	}
	if (!parseSuccess)
	{
		delete ret;
		LOG("Couldn't successfully parse array", Warning);
		return nullptr;
	}
	return ret;
}

JSON::Number* JSON::Parser::ParseNumber(const std::string & textFile)
{
	uint32 endNumberIdx = m_ReadIdx;
	enum class NumStage
	{
		SIGN,
		NUM,
		FRACTION,
		EXPSIGN,
		EXPONENT
	}stage = NumStage::SIGN;
	bool endFound = false;
	bool isInt = true;
	while (!m_Completed && !endFound)
	{
		if (CheckEOF(textFile))return false;
		char next = textFile[endNumberIdx];
		switch (stage)
		{
		case NumStage::SIGN:
			if (!(std::isdigit(next) || next == '-'))return nullptr;
			if(next == '-') endNumberIdx++;
			stage = NumStage::NUM;
			break;
		case NumStage::NUM:
			if (!(std::isdigit(next) || next == '.' || next == 'e' || next == 'E'))
			{
				endFound = true;
				continue;
			}
			endNumberIdx++;
			if (next == '.') 
			{
				stage = NumStage::FRACTION;
			}
			if (next == 'e' || next == 'E') 
			{
				stage = NumStage::EXPSIGN;
			}
			break;
		case NumStage::FRACTION:
			isInt = false;
			if (!(std::isdigit(next) || next == 'e' || next == 'E'))
			{
				endFound = true;
				continue;
			}
			endNumberIdx++;
			if (next == 'e' || next == 'E') 
			{
				stage = NumStage::EXPSIGN;
			}
			break;
		case NumStage::EXPSIGN:
			if (!(std::isdigit(next) || next == '-' || next == '+'))return nullptr;
			if(next == '-' || next == '+') endNumberIdx++;
			stage = NumStage::EXPONENT;
			break;
		case NumStage::EXPONENT:
			if (!std::isdigit(next))
			{
				endFound = true;
				continue;
			}
			endNumberIdx++;
			break;
		}
	}
	std::string numString = textFile.substr(m_ReadIdx, endNumberIdx - m_ReadIdx);
	double num = std::atof(numString.c_str());
	int64 numInt = std::stol(numString);
	m_ReadIdx = endNumberIdx;
	JSON::Number* ret = new JSON::Number();
	ret->value = num;
	ret->valueInt = numInt;
	ret->isInt = isInt;
	return ret;
}

bool JSON::Parser::CheckEOF(const std::string &textFile)
{
	if (m_Completed || m_ReadIdx >= (uint32)textFile.size())
	{
		m_Completed = true;
		LOG("Reached end of file unexpected", Warning);
		return true;
	}
	return false;
}
