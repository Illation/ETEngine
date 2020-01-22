#pragma once

namespace et {
namespace render {


class CIE final
{
public:
	void LoadData();
	dvec3 GetValue(double wavelength, double lambdaMin, double lambdaMax);
	dvec3 GetRGB(const dvec3 &xyz);
	double Interpolate(const std::vector<double>& wavelengths, const std::vector<double>& wavelength_function, double wavelength);
	dvec3 Interpolate(const std::vector<double>& wavelengths, const std::vector<double>& wavelength_function, const dvec3 &xyz);
private:
	friend class RenderingSystems;
	CIE() = default;
	~CIE() = default;

	std::vector<double> m_Table;
	dmat3 m_CieToRgb;
};


} // namespace render
} // namespace et
