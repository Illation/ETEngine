#pragma once
#include "JSONdom.h"

//The parser manages the lifetime of the dom
class JSONparser
{
public:
	JSONparser(const std::string &textFile);
	~JSONparser();

	JSONobject* GetRoot() const { return m_Root; }
private:
	enum JSONtoken : uint8
	{
		JT_BeginObject,
		JT_EndObject,
		JT_BeginArray,
		JT_EndArray,
		JT_String,
		JT_Delim,
		JT_Value,
		JT_True,
		JT_False,
		JT_Null,
		JT_Numeric,
		JT_Undefined
	};

	void MoveToNonWhitespace(const std::string &textFile);
	JSONtoken ReadToken(const std::string &textFile);

	JSONobject* ParseObject(const std::string & textFile);
	JSONpair ParsePair(const std::string & textFile);
	bool ParseString(const std::string & textFile, std::string &parsed);
	JSONvalue* ParseValue(const std::string & textFile);
	JSONarray* ParseArray(const std::string & textFile);
	JSONnumber* ParseNumber(const std::string & textFile);

	bool CheckEOF(const std::string &textFile);

	JSONobject* m_Root = nullptr;
	uint32 m_ReadIdx = 0;

	bool m_Completed = false;
};