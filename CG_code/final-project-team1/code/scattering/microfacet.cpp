#include <microfacet.h>

// Constructors
MicrofacetDistribution::MicrofacetDistribution(bool sampleVisibleArea/* = true*/) : 
    sampleVisibleArea(sampleVisibleArea)
{ }
// Methods
Float MicrofacetDistribution::G1(const Vector3f& w) const
{
    return 1 / (1 + Lambda(w));
}
/*virtual*/ Float MicrofacetDistribution::G(const Vector3f& wo, const Vector3f& wi) const
{
    return 1 / (1 + Lambda(wo) + Lambda(wi));
}

Float MicrofacetDistribution::Pdf(const Vector3f& wo, const Vector3f& wh) const
{
    if (sampleVisibleArea)
        return D(wh) * G1(wo) * std::abs(Vector3f::Dot(wo, wh)) / AbsCosTheta(wo);
    else
        return D(wh) * AbsCosTheta(wh);
}



// Constructors
BeckmannDistribution::BeckmannDistribution(Float alphaX, Float alphaY, bool sampleVisibleArea/* = true*/) : 
    MicrofacetDistribution(sampleVisibleArea), 
    alphaX(std::max(Float(0.001), alphaX)), 
    alphaY(std::max(Float(0.001), alphaY))
{ }
// Methods
/*override*/ Float BeckmannDistribution::D(const Vector3f& wh) const
{
    Float tan2Theta = Tan2Theta(wh);
    if (std::isinf(tan2Theta)) //! 优化：如果是 inf，后面都不用再算了
        return 0;
    Float cos2Theta = Cos2Theta(wh);
    Float cos4Theta = cos2Theta * cos2Theta;
    return std::exp(-tan2Theta * (Cos2Phi(wh) / (alphaX * alphaX) + Sin2Phi(wh) / (alphaY * alphaY))) / 
           (Pi * alphaX * alphaY * cos4Theta);
}
/*override*/ Float BeckmannDistribution::Lambda(const Vector3f& w) const
{
    //! 参考 PBRT：采用有理多项式近似：https://pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models
    Float absTanTheta = std::abs(TanTheta(w));
    if (std::isinf(absTanTheta)) //! 优化：如果是 inf，后面都不用再算了
        return 0;
    // Compute alpha for direction w
    Float alpha = std::sqrt(Cos2Phi(w) * alphaX * alphaX + Sin2Phi(w) * alphaY * alphaY);
    Float a = 1 / (alpha * absTanTheta);
    if (a >= 1.6f) return 0;
    return (1 - 1.259f * a + 0.396f * a * a) / (3.535f * a + 2.181f * a * a);
}

/*override*/ Vector3f BeckmannDistribution::Sample_Wh(const Vector3f& wo, const Point2f& samples) const
{
    if (!sampleVisibleArea)
    {
        Float tan2Theta, phi;
        // Sample tan2Theta, phi
        if (alphaX == alphaY) // Isotropic
        {
            Float logSample = std::log(1 - samples.x); //! 数学上不需要 1 - x
            if (std::isinf(logSample)) // Sample.x = 1
                logSample = 0; // Convert to sample.x = 0
            tan2Theta = -alphaX * alphaX * logSample;
            phi = samples.y * 2 * Pi;
        }
        else // Anisotropic
        {
            Float logSample = std::log(1 - samples.x); //! 数学上不需要 1 - x
            if (std::isinf(logSample)) // Sample.x = 1
                logSample = 0; // Convert to sample.x = 0
            phi = std::atan(alphaY / alphaX * std::tan(2 * Pi * samples.y + 0.5 * Pi));
            if (samples.y > 0.5) // 处理 atan 取负数
                phi += Pi;
            Float sinPhi = std::sin(phi), cosPhi = std::cos(phi);
            Float alphaX2 = alphaX * alphaX, alphaY2 = alphaY * alphaY;
            tan2Theta = -logSample / (cosPhi * cosPhi / alphaX2 + sinPhi * sinPhi / alphaY2);
        }
        // Sample wh
        Float cosTheta = 1 / std::sqrt(1 + tan2Theta);
        Float sinTheta = std::sqrt(std::max(Float(0), 1 - cosTheta * cosTheta));
        Vector3f wh(Vector3f::FromSpherical(phi, sinTheta, cosTheta));
        if (!SameHemisphere(wo, wh))
            wh = Vector3f(-wh);
        return wh;
    }
    else
    {
        // TODO: 
        NotImplemented();
        return Vector3f();
    }
}

/*static*/ Float BeckmannDistribution::RoughnessToAlpha(Float roughness)
{
    // From pbrt-v3
    roughness = std::max(roughness, (Float)1e-3);
    Float x = std::log(roughness);
    return 1.62142f + 0.819955f * x + 0.1734f * x * x +
           0.0171201f * x * x * x + 0.000640711f * x * x * x * x;
}