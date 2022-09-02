#include <specularTransmission.h>

// Constructors
SpecularTransmission::SpecularTransmission(const Spectrum& albedo, Float etaOu, Float etaIn) : 
    BxDF(BxDFType(int(BxDFType::Transmisstion) | int(BxDFType::Specular))), 
    albedo(albedo), 
    etaOu(etaOu), 
    etaIn(etaIn), 
    fresnel(etaOu, etaIn)
{ }

// Methods
/*override*/ Spectrum SpecularTransmission::F(const Vector3f& wo, const Vector3f& wi) const
{
    return Spectrum();
}

/*override*/ Spectrum SpecularTransmission::Sample_F(const Vector3f& wo, Vector3f* wi, 
                                                    const Point2f& sample, Float* pdf, 
                                                    BxDFType* sampledType/* = nullptr*/) const
{
    *pdf = 1;
    Float etaFrom;
    Float etaTo;
    bool entering = CosTheta(wo) < 0; //! wi 是源于 light source 的光线，wo 是源于 camera 的射线，以 “光线来自光源” 为基础
    if (entering)                     //! 因此，对于 “来自光源的光线”，cos(wo) < 0 即 cos(wi) > 0 意味着 entering
        etaFrom = etaOu, etaTo = etaIn;
    else
        etaFrom = etaIn, etaTo = etaOu;
    Float refractionRatio = etaFrom / etaTo;
    //! 同样的，由于考虑 “来自光源的光线”，因此 entering 时，wo 与 (0, 0, -1) 在同侧
    Vector3f wo_unit(wo.Normalized());
    if (Vector3f::Refract(wo_unit, entering ? Normal3f(0, 0, -1) : Normal3f(0, 0, 1), 1 / refractionRatio, wi))
    {
        if (sampledType) *sampledType = type;
        Float cosThetaFrom = CosTheta(*wi);
        Float absCosThetaFrom = std::abs(cosThetaFrom);
        return albedo * refractionRatio * refractionRatio * (Spectrum(Float(1)) - fresnel.Evaluate(cosThetaFrom)) / absCosThetaFrom;
    }
    else //! 由于全内反射 wi 与 wo 同侧，因此 etaFrom, etaTo 交换
    {
        *wi = Vector3f(-wo_unit.x, -wo_unit.y, wo_unit.z);
        if (sampledType) *sampledType = BxDFType(int(BxDFType::Reflection) | int(BxDFType::Specular));
        Float cosThetaFrom = CosTheta(*wi);
        Float absCosThetaFrom = std::abs(cosThetaFrom);
        return albedo * fresnel.Evaluate(cosThetaFrom) / absCosThetaFrom;
    }
}

/*override*/ Float SpecularTransmission::Pdf(const Vector3f& wo, const Vector3f& wi) const
{
    return 0;
}