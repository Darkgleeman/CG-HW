#include <fresnelSpecular.h>

// Constructors
FresnelSpecular::FresnelSpecular(const Spectrum& albedoR, const Spectrum& albedoT, Float etaOu, Float etaIn) : 
    BxDF(BxDFType(int(BxDFType::Reflection) | int(BxDFType::Transmisstion) | int(BxDFType::Specular))), 
    albedoR(albedoR), 
    albedoT(albedoT), 
    etaOu(etaOu), 
    etaIn(etaIn), 
    fresnel(etaOu, etaIn)
{ }

// Methods
/*override*/ Spectrum FresnelSpecular::F(const Vector3f& wo, const Vector3f& wi) const
{
    return Spectrum();
}

/*override*/ Spectrum FresnelSpecular::Sample_F(const Vector3f& wo, Vector3f* wi, 
                                                   const Point2f& sample, Float* pdf, 
                                                   BxDFType* sampledType/* = nullptr*/) const
{
    Float etaFrom;
    Float etaTo;
    Vector3f wo_unit(wo.Normalized());
    Float cosThetaTo = CosTheta(wo_unit);
    bool enteringT = cosThetaTo < 0; //! wi 是源于 light source 的光线，wo 是源于 camera 的射线，以 “光线来自光源” 为基础
    if (enteringT)                   //! 因此，对于 “来自光源的光线”，当 “假定为 transmission 时”，cos(wo) < 0 即 cos(wi) > 0 意味着 entering
        etaFrom = etaOu, etaTo = etaIn;
    else
        etaFrom = etaIn, etaTo = etaOu;
    Float fresnelReflectance = Fresnel::FrDielectric(etaOu, etaIn, cosThetaTo); //! 注意这里考虑的是 “假定为 reflection 时” 的 Fr，因此 etaFrom, etaTo 交换
    Float refractionRatio = etaFrom / etaTo;
    if (sample.x >= fresnelReflectance && // Transmission
        Vector3f::Refract(wo_unit, enteringT ? Normal3f(0, 0, -1) : Normal3f(0, 0, 1), 1 / refractionRatio, wi)) //! 同样的，由于考虑 “来自光源的光线”，因此 entering 时，wo 与 (0, 0, -1) 在同侧
    {
        *pdf = 1 - fresnelReflectance;
        if (sampledType) *sampledType = BxDFType(int(BxDFType::Transmisstion) | int(BxDFType::Specular));
        Float cosThetaFrom = CosTheta(*wi);
        Float absCosThetaFrom = std::abs(cosThetaFrom);
        return albedoT * refractionRatio * refractionRatio * (Spectrum(Float(1)) - fresnel.Evaluate(cosThetaFrom)) / absCosThetaFrom;
    }
    else // Reflection
    {
        *wi = Vector3f(-wo_unit.x, -wo_unit.y, wo_unit.z);
        *pdf = fresnelReflectance;
        if (sampledType) *sampledType = BxDFType(int(BxDFType::Reflection) | int(BxDFType::Specular));
        Float absCosThetaFrom = std::abs(cosThetaTo);
        return albedoR * fresnelReflectance / absCosThetaFrom; //! 对于 reflection，cosTheta(wo) 与 cosTheta(wi) 总是相同
    }
}

/*override*/ Float FresnelSpecular::Pdf(const Vector3f& wo, const Vector3f& wi) const
{
    return 0;
}