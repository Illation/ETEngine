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
		rttr::value("ClientInfo", TracePackage::E_Type::ClientInfo),
		rttr::value("HasClient", TracePackage::E_Type::HasClient),
		rttr::value("ContextName", TracePackage::E_Type::ContextName),
		rttr::value("ContextsDone", TracePackage::E_Type::ContextsDone),
		rttr::value("InitSuccess", TracePackage::E_Type::InitSuccess),
		rttr::value("TraceMessage", TracePackage::E_Type::TraceMessage),
		rttr::value("UpdateClientName", TracePackage::E_Type::UpdateClientName));
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

//------------------------
// WriteHeaderOnlyPackage
//
std::vector<uint8> WriteHeaderOnlyPackage(TracePackage::E_Type const type)
{
	std::vector<uint8> ret;

	NetworkWriter writer(ret);
	writer.FormatBuffer(TracePackage::GetHeaderSize());

	WriteHeader(type, 0u, writer);

	return ret;
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

//-------------------------------
// TracePackage::WriteClientInfo
//
std::vector<uint8> TracePackage::WriteClientInfo(std::string const& clientName, uint16 const contextCount)
{
	std::vector<uint8> ret;

	size_t const packageSize = sizeof(contextCount) + clientName.size();

	NetworkWriter writer(ret);
	writer.FormatBuffer(TracePackage::GetHeaderSize() + packageSize);

	WriteHeader(E_Type::ClientInfo, static_cast<uint16>(packageSize), writer);
	writer.Write(contextCount);
	writer.WriteString(clientName);

	return ret;
}

//------------------------------
// TracePackage::WriteHasClient
//
std::vector<uint8> TracePackage::WriteHasClient()
{
	return WriteHeaderOnlyPackage(E_Type::HasClient);
}

//--------------------------------
// TracePackage::WriteContextName
//
std::vector<uint8> TracePackage::WriteContextName(std::string const& contextName)
{
	return WriteBasicStringPackage(E_Type::ContextName, contextName);
}

//---------------------------------
// TracePackage::WriteContextsDone
//
std::vector<uint8> TracePackage::WriteContextsDone()
{
	return WriteHeaderOnlyPackage(E_Type::ContextsDone);
}

//--------------------------------
// TracePackage::WriteInitSuccess
//
std::vector<uint8> TracePackage::WriteInitSuccess()
{
	return WriteHeaderOnlyPackage(E_Type::InitSuccess);
}

//---------------------------------
// TracePackage::WriteTraceMessage
//
std::vector<uint8> TracePackage::WriteTraceMessage(T_Hash const context,
	E_TraceLevel const level, 
	std::string const& timestamp, 
	std::string const& message)
{
	std::vector<uint8> ret;

	size_t const packageSize = sizeof(context) + sizeof(level) + sizeof(uint16) + timestamp.size() + message.size();

	NetworkWriter writer(ret);
	writer.FormatBuffer(TracePackage::GetHeaderSize() + packageSize);

	WriteHeader(E_Type::TraceMessage, static_cast<uint16>(packageSize), writer);
	writer.Write(context);
	writer.Write(level);
	writer.Write(static_cast<uint16>(timestamp.size()));
	writer.WriteString(timestamp);
	writer.WriteString(message);

	return ret;
}

//-------------------------------------
// TracePackage::WriteUpdateClientName
//
std::vector<uint8> TracePackage::WriteUpdateClientName(std::string const& clientName)
{
	return WriteBasicStringPackage(E_Type::UpdateClientName, clientName);
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

//------------------------------
// TracePackage::ReadClientInfo
//
void TracePackage::ReadClientInfo(std::vector<uint8> const& inBuffer, std::string& outClientName, uint16& outContextCount)
{
	NetworkReader reader;
	reader.Open(inBuffer);

	outContextCount = reader.Read<uint16>();
	outClientName = reader.ReadString(reader.GetBufferSize() - reader.GetBufferPosition());
}

//------------------------------
// TracePackage::ReadClientInfo
//
void TracePackage::ReadContextName(std::vector<uint8> const& inBuffer, std::string& outContextName)
{
	NetworkReader reader;
	reader.Open(inBuffer);

	outContextName = reader.ReadString(reader.GetBufferSize());
}

//--------------------------------
// TracePackage::ReadTraceMessage
//
void TracePackage::ReadTraceMessage(std::vector<uint8> const& inBuffer, 
	T_Hash& outContext,
	E_TraceLevel& outLevel, 
	std::string& outTimestamp, 
	std::string& outMessage)
{
	NetworkReader reader;
	reader.Open(inBuffer);

	outContext = reader.Read<T_Hash>();
	outLevel = reader.Read<E_TraceLevel>();

	uint16 const timestampSize = reader.Read<uint16>();
	outTimestamp = reader.ReadString(timestampSize);

	outMessage = reader.ReadString(reader.GetBufferSize() - reader.GetBufferPosition());
}

//------------------------------------
// TracePackage::ReadUpdateClientName
//
void TracePackage::ReadUpdateClientName(std::vector<uint8> const& inBuffer, std::string& outClientName)
{
	NetworkReader reader;
	reader.Open(inBuffer);

	outClientName = reader.ReadString(reader.GetBufferSize());
}


} // namespace core
} // namespace et

