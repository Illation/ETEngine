#include <EtFramework/stdafx.h>
#include <catch2/catch.hpp>

#include <mainTesting.h>

#include <EtMath/MathUtil.h>

#include <EtCore/FileSystem/Entry.h>
#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/IO/XmlParser.h>


using namespace et;
using namespace et::core;


TEST_CASE("ParseXML", "[xml]")
{
	File* xmlFile = new File(global::g_UnitTestDir + "FileSystem/xml_test_file.xml", nullptr);
	bool openResult = xmlFile->Open(FILE_ACCESS_MODE::Read);
	REQUIRE(openResult == true);
	XML::Parser parser = XML::Parser(FileUtil::AsText(xmlFile->Read()));
	delete xmlFile;
	xmlFile = nullptr;

	XML::Document const& doc = parser.GetDocument();
	REQUIRE(doc.m_Version == "1.0");
	REQUIRE(doc.m_Encoding == XML::Document::E_Enconding::UTF_8);
	REQUIRE(doc.m_IsStandalone == true);

	XML::Element const& root = doc.m_Root;
	REQUIRE(root.m_Name == core::HashString("root"));
	REQUIRE(root.m_Attributes.size() == 3u);
	
	XML::Attribute const* const bv = root.GetAttribute("boolvalue"_hash);
	REQUIRE(bv != nullptr);
	REQUIRE(bv->m_Value == "true");

	XML::Attribute const* const fv = root.GetAttribute("falsevalue"_hash);
	REQUIRE(fv != nullptr);
	REQUIRE(fv->m_Value == "false");

	XML::Attribute const* const nv = root.GetAttribute("nullvalue"_hash);
	REQUIRE(nv != nullptr);
	REQUIRE(nv->m_Value == "0");

	REQUIRE(root.m_Value.empty());
	REQUIRE(root.m_Children.size() == 1u);

	XML::Element const& menu = root.m_Children[0];
	REQUIRE(menu.m_Name == core::HashString("menu"));
	REQUIRE(menu.m_Attributes.size() == 1u);
	REQUIRE(menu.m_Attributes[0].m_Name == core::HashString("id"));
	REQUIRE(menu.m_Attributes[0].m_Value == "file");
	REQUIRE(menu.m_Value.empty());
	REQUIRE(menu.m_Children.size() == 2u);

	XML::Element const& popup = menu.m_Children[0];
	REQUIRE(popup.m_Name == core::HashString("popup"));
	REQUIRE(popup.m_Children.size() == 4u);

	size_t pos = 0u;
	XML::Element const* menuItem = popup.GetFirstChild("menuitem"_hash, pos);
	REQUIRE(menuItem != nullptr);
	REQUIRE(menuItem->m_Value.empty());
	REQUIRE(menuItem->GetAttribute("value"_hash)->m_Value == "New");
	REQUIRE(menuItem->GetAttribute("onclick"_hash)->m_Value == "CreateNewDoc()");

	menuItem = popup.GetFirstChild("menuitem"_hash, ++pos);
	REQUIRE(menuItem != nullptr);
	REQUIRE(menuItem->m_Value == "lol");
	REQUIRE(menuItem->GetAttribute("value"_hash)->m_Value == "Open");
	REQUIRE(menuItem->GetAttribute("onclick"_hash)->m_Value == "OpenDoc()");

	menuItem = popup.GetFirstChild("menuitem"_hash, ++pos);
	REQUIRE(menuItem != nullptr);
	REQUIRE(menuItem->m_Value.empty());
	REQUIRE(menuItem->GetAttribute("value"_hash)->m_Value == "Close");
	REQUIRE(menuItem->GetAttribute("onclick"_hash)->m_Value == "CloseDoc()");

	menuItem = popup.GetFirstChild("menuitem"_hash, ++pos);
	REQUIRE(menuItem != nullptr);
	REQUIRE(menuItem->m_Value.empty());
	REQUIRE(menuItem->GetAttribute("value"_hash)->m_Value == "Save");
	REQUIRE(menuItem->GetAttribute("onclick"_hash)->m_Value == "SaveDoc()");

	XML::Element const* const numArray = menu.GetFirstChild("numarray"_hash);
	REQUIRE(numArray != nullptr);
	REQUIRE(numArray->m_Attributes.size() == 1u);
	REQUIRE(numArray->m_Attributes[0].m_Name == core::HashString("test"));
	REQUIRE(numArray->m_Attributes[0].m_Value == "\"");
	REQUIRE(numArray->m_Children.empty());
	REQUIRE(numArray->m_Value == "10 3.345 2e4 4.53E-2 0.4e12 -12.43e+3");
}
