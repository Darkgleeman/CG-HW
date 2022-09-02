#ifndef MICROFACET_REFLECTION_H
#define MICROFACET_REFLECTION_H

#include <microfacet.h>

class MicrofacetReflection : public BxDF
{
public:
    // Fields
    const Spectrum albedoR;
    const MicrofacetDistribution* distribution;
    const Fresnel* fresnel;
    
    // Constructors
    MicrofacetReflection(const Spectrum& albedoR, const MicrofacetDistribution* distribution, const Fresnel* fresnel);

    // Methods
    virtual Spectrum F(const Vector3f& wo, const Vector3f& wi) const override;
    virtual Spectrum Sample_F(const Vector3f& wo, Vector3f* wi, 
                              const Point2f& sample, Float* pdf, 
                              BxDFType* sampledType = nullptr) const override;
    virtual Float Pdf(const Vector3f& wo, const Vector3f& wi) const override;
};

#endif