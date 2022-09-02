#include <microfacetReflection.h>

// Constructors
MicrofacetReflection::MicrofacetReflection(const Spectrum& albedoR, const MicrofacetDistribution* distribution, const Fresnel* fresnel) : 
    BxDF(BxDFType(int(BxDFType::Reflection) | int(BxDFType::Glossy))), 
    albedoR(albedoR), 
    distribution(distribution), 
    fresnel(fresnel)
{ }

// Methods
/*override*/ Spectrum MicrofacetReflection::F(const Vector3f& wo, const Vector3f& wi) const
{
    Vector3f wo_unit(wo.Normalized()), wi_unit(wi.Normalized());
    Float cosThetaO = AbsCosTheta(wo_unit), cosThetaI = AbsCosTheta(wi_unit);
    //! 优化：Degenerate cases
    if (cosThetaO == 0 || cosThetaI == 0)
        return Spectrum();
    Vector3f wh(wo_unit + wi_unit);
    //! 优化：Degenerate cases
    if (wh.x == 0 && wh.y == 0 && wh.z == 0)
        return Spectrum();
    Vector3f wh_unit(wh.Normalized());
    Spectrum fr = fresnel->Evaluate(Vector3f::Dot(wi_unit, wh_unit));
    return albedoR * distribution->D(wh_unit) * distribution->G(wo_unit, wi_unit) * fr / (4 * cosThetaO * cosThetaI);
}
/*override*/ Spectrum MicrofacetReflection::Sample_F(const Vector3f& wo, Vector3f* wi, 
                                                     const Point2f& sample, Float* pdf, 
                                                     BxDFType* sampledType/* = nullptr*/) const
{
    Vector3f wo_unit(wo.Normalized());
    Vector3f wh_unit = distribution->Sample_Wh(wo_unit, sample); //! Wh 默认已单位化
    if (Normal3f::Dot(wo_unit, wh_unit) < 0)
    {
        *pdf = 0; // TODO: 确定一个规则：是否一定要将 *pdf 置为零
        return Spectrum();
    }
    *wi = Vector3f(Vector3f::Reflect(wo_unit, wh_unit)); //! Wi 是单位向量
    if (sampledType) *sampledType = type;
    if (!SameHemisphere(wo_unit, *wi))
    {
        *pdf = 0;
        return Spectrum();
    }
    else
    {
        *pdf = distribution->Pdf(wo_unit, wh_unit) / (4 * Normal3f::Dot(wo_unit, wh_unit));
        return F(wo_unit, *wi);
    }
}
/*override*/ Float MicrofacetReflection::Pdf(const Vector3f& wo, const Vector3f& wi) const
{
    if (!SameHemisphere(wo, wi))
        return 0;
    else
    {
        Vector3f wo_unit(wo.Normalized());
        Vector3f wh_unit((wo + wi).Normalized());
        return distribution->Pdf(wo_unit, wh_unit) / (4 * Normal3f::Dot(wo_unit, wh_unit));
    }
}