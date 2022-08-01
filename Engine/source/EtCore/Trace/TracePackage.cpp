#include "stdafx.h"
#include "TracePackage.h"

#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/Network/NetworkReader.h>
#include <EtCore/Network/NetworkWriter.h>


namespace et {
namespace core {


//===============
// Trace Package
//===============


// reflection
//////////////
RTTR_REGISTRATION
{
	rttr::registration::enumeration<TracePackage::E_Type>("TracePackageType") (
		rttr::value("Invalid", TracePackage::E_Type::Invalid),
		rttr::value("ConnectionAcknowledged", TracePackage::E_Type::ConnectionAcknowledged),
		rttr::value("ClientName", TracePackage::E_Type::ClientName),
		rttr::value("HasClient", TracePackage::E_Type::HasClient),
		rttr::value("ContextName", TracePackage::E_Type::ContextName),
		rttr::value("ContextsDone", TracePackage::E_Type::ContextsDone),
		rttr::value("InitSuccess", TracePackage::E_Type::InitSuccess),
		rttr::value("InitFailed", TracePackage::E_Type::InitFailed),
		rttr::value("TraceMessage", TracePackage::E_Type::TraceMessage));
}


// Utility 
//*********


static std::string const s_ConnectionAcknowledgedStr("EtTrace");


//--------------
// WriteHeader
//
void WriteHeader(TracePackage::E_Type const type, uint16 const packageSize, NetworkWriter& inOutWriter)
{
	inOutWriter.Write(type);
	inOutWriter.Write(packageSize);
}

//-------------------------
// WriteBasicStringPackage
//
std::vector<uint8> WriteBasicStringPackage(TracePackage::E_Type const type, std::string const& content)
{
	std::vector<uint8> ret;

	NetworkWriter writer(ret);
	writer.FormatBuffer(TracePackage::GetHeaderSize() + content.size()); 

	WriteHeader(type, static_cast<uint16>(content.size()), writer);
	writer.WriteString(content);

	return ret;
}


// implementation
//********************


// static
std::string const TracePackage::s_TraceServerPort("6984"); // ET
std::string const TracePackage::s_ProtocolVersion("v1.0"); 


//-------------------------------------------
// TracePackage::WriteConnectionAcknowledged
//
std::vector<uint8> TracePackage::WriteConnectionAcknowledged()
{
	std::string const packageStr = s_ConnectionAcknowledgedStr + s_ProtocolVersion;
	return WriteBasicStringPackage(E_Type::ConnectionAcknowledged, packageStr);
}

//-----------------------------
// TracePackage::GetHeaderSize
//
size_t TracePackage::GetHeaderSize()
{
	return (sizeof(E_Type) + sizeof(uint16));
}

//--------------------------
// TracePackage::ReadHeader
//
TracePackage::E_Type TracePackage::ReadHeader(std::vector<uint8> const& inBuffer, uint16& outPackageSize)
{
	NetworkReader reader;
	reader.Open(inBuffer, 0u, GetHeaderSize());

	E_Type ret = reader.Read<E_Type>();
	outPackageSize = reader.Read<uint16>();

	return ret;
}

//------------------------------------------
// TracePackage::ReadConnectionAcknowledged
//
bool TracePackage::ReadConnectionAcknowledged(std::vector<uint8> const& inBuffer)
{
	std::string const receivedStr = FileUtil::AsText(inBuffer);
	if ((receivedStr.size() < s_ConnectionAcknowledgedStr.size()) || 
		(receivedStr.substr(0, s_ConnectionAcknowledgedStr.size()) != s_ConnectionAcknowledgedStr))
	{
		ET_LOG_W(ET_CTX_CORE, "Mismatching connection acknowledged ID, received '%s'!", receivedStr.c_str());
		return false;
	}

	std::string const versionStr = receivedStr.substr(s_ConnectionAcknowledgedStr.size());
	if (versionStr != s_ProtocolVersion)
	{
		ET_LOG_W(ET_CTX_CORE, "Mismatching protocol version, expected '%s', received '%s'!", s_ProtocolVersion.c_str(), versionStr.c_str());
		return false;
	}

	return true;
}


} // namespace core
} // namespace et

