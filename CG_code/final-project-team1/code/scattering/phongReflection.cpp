#include <phongReflection.h>

// Constructors
PhongReflection::PhongReflection(const Spectrum& albedo, int smoothness) : 
    BxDF(BxDFType(int(BxDFType::Reflection) | int(BxDFType::Glossy))), 
    albedo(albedo), 
    smoothness(smoothness) { }

// Methods
/*override*/ Spectrum PhongReflection::F(const Vector3f& wo, const Vector3f& wi) const
{
    Normal3f normal = CosTheta(wo) > 0 ? Normal3f(0, 0, 1) : Normal3f(0, 0, -1);
    Vector3f wo_unit(wo.Normalized());
    Vector3f wi_unit(wi.Normalized());
    return albedo * std::max(Vector3f::Dot(normal, wi_unit), Float(0)) + 
           std::pow(std::max(Vector3f::Dot(Vector3f::Reflect(wi_unit, normal), wo_unit), Float(0)), smoothness);
}
/*override*/ Spectrum PhongReflection::Sample_F(const Vector3f& wo, Vector3f* wi, 
                                                const Point2f& sample, Float* pdf, 
                                                BxDFType* sampledType/* = nullptr*/) const
{
    // TODO: PhongReflection::Sample_F()
    NotImplemented();
    return Spectrum();
}