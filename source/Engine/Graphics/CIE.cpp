#include "stdafx.h"
#include "CIE.h"
#include "FileSystem\JSONparser.h"
#include "FileSystem\Entry.h"
#include "FileSystem\FileUtil.h"

void CIE::LoadData()
{
	File* jsonFile = new File("./cie.json", nullptr);
	if (!jsonFile->Open(FILE_ACCESS_MODE::Read))
		return;

	JSON::Parser parser = JSON::Parser(FileUtil::AsText(jsonFile->Read()));
	delete jsonFile;
	jsonFile = nullptr;

	JSON::Object* root = parser.GetRoot();
	m_Table = (*root)["2 deg color matching"]->arr()->NumArr();
	auto jxyz = (*root)["xyz to rgb"];
	JSON::ArrayMatrix(jxyz, m_CieToRgb);
}

dvec3 CIE::GetValue(double wavelength, double lambdaMin, double lambdaMax)
{
	dvec3 ret;
	for (uint8 column = 1; column < 4; ++column)
	{
		if (wavelength <= lambdaMin || wavelength >= lambdaMax)
		{
			return 0.0;
		}
		double u = (wavelength - lambdaMin) / 5.0;
		int row = static_cast<int>(std::floor(u));
		assert(row >= 0 && row + 1 < 95);
		assert(m_Table[4 * row] <= wavelength && m_Table[4 * (row + 1)] >= wavelength);
		u -= row;
		ret[column-1] = m_Table[4 * row + column] * (1.0 - u) + m_Table[4 * (row + 1) + column] * u;
	}
	return ret;
}

dvec3 CIE::GetRGB(const dvec3 &xyz)
{
	return m_CieToRgb * xyz;
}

double CIE::Interpolate(const std::vector<double>& wavelengths, const std::vector<double>& wavelength_function, double wavelength)
{
	assert(wavelength_function.size() == wavelengths.size());
	if (wavelength < wavelengths[0])
	{
		return wavelength_function[0];
	}
	for (unsigned int i = 0; i < wavelengths.size() - 1; ++i)
	{
		if (wavelength < wavelengths[i + 1])
		{
			double u = (wavelength - wavelengths[i]) / (wavelengths[i + 1] - wavelengths[i]);
			return wavelength_function[i] * (1.0 - u) + wavelength_function[i + 1] * u;
		}
	}
	return wavelength_function[wavelength_function.size() - 1];
}

dvec3 CIE::Interpolate(const std::vector<double>& wavelengths, const std::vector<double>& wavelength_function, const dvec3 &xyz)
{
	dvec3 ret = dvec3();
	for (uint8 i = 0; i < 3; ++i)
		ret[i] = Interpolate(wavelengths, wavelength_function, xyz[i]);
	return ret;
}