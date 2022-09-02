#ifndef SPECULAR_REFLECTION_H
#define SPECULAR_REFLECTION_H

#include <bxdf.h>

class SpecularReflection : public BxDF
{
public:
    // Fields
    const Spectrum albedo;
    const Fresnel* fresnel;

    // Constructors
    SpecularReflection(const Spectrum& albedo, const Fresnel* fresnel);

    // Methods
    virtual Spectrum F(const Vector3f& wo, const Vector3f& wi) const override;
    virtual Spectrum Sample_F(const Vector3f& wo, Vector3f* wi, 
                              const Point2f& sample, Float* pdf, 
                              BxDFType* sampledType = nullptr) const override;
    virtual Float Pdf(const Vector3f& wo, const Vector3f& wi) const override;
};

#endif