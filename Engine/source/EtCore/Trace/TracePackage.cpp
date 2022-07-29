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


//-------------------------------------------
// TracePackage::WriteConnectionAcknowledged
//
std::vector<uint8> TracePackage::WriteConnectionAcknowledged()
{
	return WriteBasicStringPackage(E_Type::ConnectionAcknowledged, s_ConnectionAcknowledgedStr);
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
	return (FileUtil::AsText(inBuffer) == s_ConnectionAcknowledgedStr);
}


} // namespace core
} // namespace et

