#pragma once
#include "JSONdom.h"


namespace et {
namespace core {


namespace JSON
{
	//The parser manages the lifetime of the dom
	class Parser
	{
	public:
		Parser(const std::string &textFile);
		~Parser();
	
		JSON::Object* GetRoot() const { return m_Root; }
	private:
		enum Token : uint8
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
		Token ReadToken(const std::string &textFile);
	
		JSON::Object* ParseObject(const std::string & textFile);
		JSON::Pair ParsePair(const std::string & textFile);
		bool ParseString(const std::string & textFile, std::string &parsed);
		JSON::Value* ParseValue(const std::string & textFile);
		JSON::Array* ParseArray(const std::string & textFile);
		JSON::Number* ParseNumber(const std::string & textFile);
	
		bool CheckEOF(const std::string &textFile);
	
		JSON::Object* m_Root = nullptr;
		uint32 m_ReadIdx = 0;
	
		bool m_Completed = false;
	};
}


} // namespace core
} // namespace et
