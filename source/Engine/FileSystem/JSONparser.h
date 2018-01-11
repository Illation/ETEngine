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
	void MoveToNonWhitespace(const std::string &textFile);

	JSONobject* m_Root = nullptr;
	uint32 m_ReadIdx = 0;
};