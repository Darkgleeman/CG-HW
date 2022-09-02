#ifndef RGB_SPECTRUM_H
#define RGB_SPECTRUM_H

#include <coefficientSpectrum.h>

class RGBSpectrum : public CoefficientSpectrum<3>
{
public:
    // Constructors
    RGBSpectrum(Float value = 0.f);
    RGBSpectrum(Float* values);
    RGBSpectrum(Float r, Float g, Float b);
    RGBSpectrum(const CoefficientSpectrum<3>& spectrum);
    RGBSpectrum(const RGBSpectrum& spectrum);

    // Methods
    void ToRGB(Float* rgb) const;

};

#endif