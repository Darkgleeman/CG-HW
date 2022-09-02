#include <RGBSpectrum.h>

// Constructors
RGBSpectrum::RGBSpectrum(Float value/* = 0.f*/) : CoefficientSpectrum<3>(value) { }
RGBSpectrum::RGBSpectrum(Float* values) : CoefficientSpectrum(values) { }
RGBSpectrum::RGBSpectrum(Float r, Float g, Float b) : CoefficientSpectrum<3>({ r, g, b }) { }
RGBSpectrum::RGBSpectrum(const CoefficientSpectrum<3>& spectrum) : CoefficientSpectrum<3>(spectrum) { }
RGBSpectrum::RGBSpectrum(const RGBSpectrum& spectrum) { *this = spectrum; }

// Methods
void RGBSpectrum::ToRGB(Float* rgb) const
{
    rgb[0] = c[0];
    rgb[1] = c[1];
    rgb[2] = c[2];
}