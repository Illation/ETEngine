#pragma once


namespace et {
namespace core {


//-----
// URI
//
// Implementation of a "Unique Resource Identifier"
//  - Implementation isn't exhaustive of all types but enough to decode a GLTF asset
//
class URI
{
	// definitions
	//-------------
	static std::string const s_Base64Mime;
	static std::string const s_UriFile;
	static std::string const s_UriData;
	static inline bool IsBase64(unsigned char c) { return (isalnum(c) || (c == '+') || (c == '/')); }

public:
	enum class E_Type : uint8
	{
		None, 
		FileRelative,
		FileAbsolute,
		Data,

		Invalid
	};

	// static functionality
	//----------------------
	static bool DecodeBase64(std::string const& encoded, std::vector<uint8>& decoded);

	// construct destruct
	//--------------------
	URI() = default;
	URI(std::string const& path);

	// functionality
	//---------------
	void SetPath(std::string const& path);
	bool Evaluate();
	bool Evaluate(std::string const& basePath);
	void ClearEvaluatedData(); // need to set path again before reevaluating for data types

	// accessors
	//-----------
	E_Type GetType() const { return m_Type; }
	std::string const& GetPath() const { return m_Path; }

	bool CanEvaluate() const { return ((m_Type != E_Type::Invalid) && (m_Type != E_Type::None)); }
	bool IsEvaluated() const { return m_IsEvaluated; }
	std::vector<uint8> const& GetEvaluatedData() const { return m_BinData; }

	// Data
	///////

private:
	E_Type m_Type = E_Type::None;
	std::string m_Path;
	std::string m_Extension;

	bool m_IsEvaluated = false;
	std::vector<uint8> m_BinData;
};


} // namespace core
} // namespace et
