#ifndef FRESNEL_SPECULAR_H
#define FRESNEL_SPECULAR_H

#include <bxdf.h>

class FresnelSpecular : public BxDF
{
public:
    // Fields
    const Spectrum albedoR;
    const Spectrum albedoT;
    const Float etaOu;
    const Float etaIn;
    const FresnelDielectric fresnel;

    // Constructors
    FresnelSpecular(const Spectrum& albedoR, const Spectrum& albedoT, Float etaOu, Float etaIn);

    // Methods
    virtual Spectrum F(const Vector3f& wo, const Vector3f& wi) const override;
    virtual Spectrum Sample_F(const Vector3f& wo, Vector3f* wi, 
                              const Point2f& sample, Float* pdf, 
                              BxDFType* sampledType = nullptr) const override;
    virtual Float Pdf(const Vector3f& wo, const Vector3f& wi) const override;
};

#endif