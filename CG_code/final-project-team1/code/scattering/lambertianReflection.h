#ifndef LAMBERTIAN_REFLECTION_H
#define LAMBERTIAN_REFLECTION_H

#include <bxdf.h>

class LambertianReflection : public BxDF
{
public:
    // Fields
    const Spectrum albedo;

    // Constructors
    LambertianReflection(const Spectrum& albedo);

    // Methods
    virtual Spectrum F(const Vector3f& wo, const Vector3f& wi) const override;
    virtual Spectrum Rho(const Vector3f& wo, int nSamples, const Point2f* samples) const override;
};

#endif