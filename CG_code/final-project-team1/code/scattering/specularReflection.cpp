#include <specularReflection.h>

// Constructors
SpecularReflection::SpecularReflection(const Spectrum& albedo, const Fresnel* fresnel) : 
    BxDF(BxDFType(int(BxDFType::Reflection) | int(BxDFType::Specular))), 
    albedo(albedo), 
    fresnel(fresnel)
{ }

// Methods
/*override*/ Spectrum SpecularReflection::F(const Vector3f& wo, const Vector3f& wi) const
{
    return Spectrum();
}

/*override*/ Spectrum SpecularReflection::Sample_F(const Vector3f& wo, Vector3f* wi, 
                                                   const Point2f& sample, Float* pdf, 
                                                   BxDFType* sampledType/* = nullptr*/) const
{
    Vector3f wo_unit(wo.Normalized());
    *wi = Vector3f(-wo_unit.x, -wo_unit.y, wo_unit.z);
    *pdf = 1;
    if (sampledType) *sampledType = type;
    Float cosThetaFrom = CosTheta(*wi);
    Float absCosThetaFrom = std::abs(cosThetaFrom);
    return albedo * fresnel->Evaluate(cosThetaFrom) / absCosThetaFrom;
}

/*override*/ Float SpecularReflection::Pdf(const Vector3f& wo, const Vector3f& wi) const
{
    return 0;
}