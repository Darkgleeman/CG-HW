#include <lambertianReflection.h>

// Constructors
LambertianReflection::LambertianReflection(const Spectrum& albedo) : 
    BxDF(BxDFType(int(BxDFType::Reflection) | int(BxDFType::Diffuse))), 
    albedo(albedo) { }

// Methods
/*override*/ Spectrum LambertianReflection::F(const Vector3f& wo, const Vector3f& wi) const
{
    return albedo * PiInv;
}

/*override*/ Spectrum LambertianReflection::Rho(const Vector3f& wo, int nSamples, const Point2f* samples) const
{
    return albedo;
}