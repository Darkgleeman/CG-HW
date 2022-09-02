#ifndef PHONG_REFLECTION_H
#define PHONG_REFLECTION_H

#include <bxdf.h>

class PhongReflection : public BxDF
{
public:
    // Fields
    const Spectrum albedo;
    const int smoothness;

    // Constructors
    PhongReflection(const Spectrum& albedo, int smoothness);

    // Methods
    virtual Spectrum F(const Vector3f& wo, const Vector3f& wi) const override;
    virtual Spectrum Sample_F(const Vector3f& wo, Vector3f* wi, 
                              const Point2f& sample, Float* pdf, 
                              BxDFType* sampledType = nullptr) const override;
};

#endif