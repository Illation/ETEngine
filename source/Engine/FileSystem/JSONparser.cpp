#include "stdafx.hpp"
#include "JSONparser.h"

#include <functional>
#include <cctype>
#include <locale>

JSONparser::JSONparser(const std::string &textFile)
{
	MoveToNonWhitespace(textFile);
	if (!m_Completed && ReadToken(textFile) == JT_BeginObject)
	{
		m_Root = ParseObject(textFile);
		return;
	}
	std::cout << "Expected '{' token, parsing JSON failed" << std::endl;
}

JSONparser::~JSONparser()
{
	delete m_Root;
	m_Root = nullptr;
}

void JSONparser::MoveToNonWhitespace(const std::string &textFile)
{
	if (!std::isspace(textFile[m_ReadIdx]))return;
	auto it = std::find_if(textFile.begin()+m_ReadIdx, textFile.end(), [](const uint8 c) { return !std::isspace(c); });
	if (it == textFile.end())
	{
		m_Completed = true;
		m_ReadIdx = 0;
		return;
	}
	m_ReadIdx = it - textFile.begin();
}

JSONparser::JSONtoken JSONparser::ReadToken(const std::string &textFile)
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
			m_ReadIdx += 4;
			return JT_True;
		}
		return JT_Undefined;
	case 'f': if (m_ReadIdx + 5 > (uint32)textFile.size()) { m_Completed = true; return JT_Undefined; }
		if (textFile.substr(m_ReadIdx, 4) == "alse")
		{
			m_ReadIdx += 5;
			return JT_False;
		}
		return JT_Undefined;
	case 'n': if (m_ReadIdx + 4 > (uint32)textFile.size()) { m_Completed = true; return JT_Undefined; }
		if (textFile.substr(m_ReadIdx, 3) == "ull")
		{
			m_ReadIdx += 4;
			return JT_Null;
		}
		return JT_Undefined;
	default: 
		m_ReadIdx--;
		if (textFile[m_ReadIdx] == '-' || std::isdigit(textFile[m_ReadIdx])) return JT_Numeric;
		return JT_Undefined;
	}
}

JSONobject* JSONparser::ParseObject(const std::string & textFile)
{
	JSONobject* ret = new JSONobject;
	bool parseSuccess = false;
	bool parseFail = false;
	bool prevPair = false;
	bool prevDelim = false;
	while (!m_Completed && !parseSuccess && !parseFail)
	{
		MoveToNonWhitespace(textFile);
		if(CheckEOF(textFile))continue;
		JSONtoken token = ReadToken(textFile);
		if(!(token == JT_EndObject) && CheckEOF(textFile))continue;
		switch (token)
		{
		case JT_EndObject:
			if(prevDelim)
			{
				std::cout << "Expected a new Key Value pair after delimiter" << std::endl;
				parseFail = true;
				continue;
			}
			parseSuccess = true;
			break;
		case JT_String:
		{
			if (prevPair)
			{
				std::cout << "Expected a delimiter before new key value pair" << std::endl;
				parseFail = true;
				continue;
			}
			prevDelim = false;
			JSONpair keyVal = ParsePair(textFile);
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
				std::cout << "Expected a new Key Value pair before delimiter" << std::endl;
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
		std::cout << "Couldn't successfully parse object" << std::endl;
		return nullptr;
	}
	return ret;
}

JSONpair JSONparser::ParsePair(const std::string & textFile)
{
	JSONpair ret;
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

JSONvalue* JSONparser::ParseValue(const std::string & textFile)
{
	MoveToNonWhitespace(textFile);
	if(CheckEOF(textFile))return nullptr;
	JSONtoken token = ReadToken(textFile);
	if(CheckEOF(textFile))return nullptr;

	switch (token)
	{
	case JT_String:
	{
		JSONstring* ret = new JSONstring();
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
		JSONbool* ret = new JSONbool();
		ret->value = token == JT_True;
		return ret;
	}
	case JT_Null:
		return new JSONvalue();
	}

	std::cout << "Couldn't successfully parse value, unexpected token" << std::endl;
	return nullptr;
}

bool JSONparser::ParseString(const std::string & textFile, std::string &parsed)
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
			m_ReadIdx++;
			if (CheckEOF(textFile))return false;
			next = textFile[m_ReadIdx++];
			switch (next)
			{
			case '"': parsed += '"';
			case '\\': parsed += '\\';
			case '/': parsed += '/';
			case 'b': parsed += '\b';
			case 'f': parsed += '\f';
			case 'n': parsed += '\n';
			case 'r': parsed += '\r';
			case 't': parsed += '\t';
			case 'u': 
			{
				if (m_ReadIdx + 4 > (uint32)textFile.size()) return false;
				std::string hexString = "0x" + textFile.substr(m_ReadIdx, 4);
				parsed += static_cast<char>(std::stoul(hexString, nullptr, 16));
				m_ReadIdx += 4;
			}
			default:
				std::cout << "unexpected symbol after escape character while parsing string" << std::endl;
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

JSONarray* JSONparser::ParseArray(const std::string & textFile)
{
	JSONarray* ret = new JSONarray;
	bool parseSuccess = false;
	bool parseFail = false;
	bool prevVal = false;
	bool prevDelim = false;
	while (!m_Completed && !parseSuccess && !parseFail)
	{
		MoveToNonWhitespace(textFile);
		if(CheckEOF(textFile))continue;
		JSONtoken token = ReadToken(textFile);
		if(!(token == JT_Delim) && !(token == JT_EndArray))m_ReadIdx--;
		if(CheckEOF(textFile))continue;
		switch (token)
		{
		case JT_EndArray:
			if(prevDelim)
			{
				std::cout << "Expected a new Key Value pair after delimiter" << std::endl;
				parseFail = true;
				continue;
			}
			parseSuccess = true;
			break;
		case JT_Delim:
			if (!prevVal)
			{
				std::cout << "Expected a new Key Value pair before delimiter" << std::endl;
				parseFail = true;
				continue;
			}
			prevVal = false;
			prevDelim = true;
			continue;
		default:
			if (!prevVal)
			{
				JSONvalue* val = ParseValue(textFile);
				prevDelim = false;
				if (val)
				{
					ret->value.push_back(val);
					prevVal = true;
					continue;
				}
			}
			else 
				std::cout << "Expected a delimiter before new Key Value pair" << std::endl;
			parseFail = true;
			continue;
		}
	}
	if (!parseSuccess)
	{
		delete ret;
		std::cout << "Couldn't successfully parse array" << std::endl;
		return nullptr;
	}
	return ret;
}

JSONnumber* JSONparser::ParseNumber(const std::string & textFile)
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
	while (!m_Completed && !endFound)
	{
		if (CheckEOF(textFile))return false;
		char next = textFile[endNumberIdx++];
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
				endNumberIdx++;
				stage = NumStage::FRACTION;
			}
			if (next == 'e' || next == 'E') 
			{
				endNumberIdx++;
				stage = NumStage::EXPSIGN;
			}
			break;
		case NumStage::FRACTION:
			if (!(std::isdigit(next) || next == 'e' || next == 'E'))
			{
				endFound = true;
				continue;
			}
			endNumberIdx++;
			if (next == 'e' || next == 'E') 
			{
				endNumberIdx++;
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
	m_ReadIdx = endNumberIdx + 1;
	JSONnumber* ret = new JSONnumber();
	ret->value = num;
	return ret;
}

bool JSONparser::CheckEOF(const std::string &textFile)
{
	if (m_Completed || m_ReadIdx >= (uint32)textFile.size())
	{
		m_Completed = true;
		std::cout << "Reached end of file unexpected" << std::endl;
		return true;
	}
	return false;
}
