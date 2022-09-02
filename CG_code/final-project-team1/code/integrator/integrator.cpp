#include <integrator.h>

// Supporting methods
Point3i Integrator::RGBFloatsToRGB256(Float* rgbFloats)
{
    // Gamma correction
    Float rf = std::pow(std::clamp(rgbFloats[0], Float(0), Float(1)), 1 / 2.2);
    Float gf = std::pow(std::clamp(rgbFloats[1], Float(0), Float(1)), 1 / 2.2);
    Float bf = std::pow(std::clamp(rgbFloats[2], Float(0), Float(1)), 1 / 2.2);

    int ri = std::clamp(int(rf * 255), 0, 255);
    int gi = std::clamp(int(gf * 255), 0, 255);
    int bi = std::clamp(int(bf * 255), 0, 255);

    // Float to int
    return Point3i(ri, gi, bi);
}