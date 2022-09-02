#ifndef SPECULAR_TRANSMISSION_H
#define SPECULAR_TRANSMISSION_H

#include <bxdf.h>

class SpecularTransmission : public BxDF
{
public:
    // Fields
    const Spectrum albedo;
    const Float etaOu;
    const Float etaIn;
    const FresnelDielectric fresnel;

    // Constructors
    SpecularTransmission(const Spectrum& albedo, Float etaOu, Float etaIn);

    // Methods
    virtual Spectrum F(const Vector3f& wo, const Vector3f& wi) const override;
    virtual Spectrum Sample_F(const Vector3f& wo, Vector3f* wi, 
                              const Point2f& sample, Float* pdf, 
                              BxDFType* sampledType = nullptr) const override;
    virtual Float Pdf(const Vector3f& wo, const Vector3f& wi) const override;
};

#endif