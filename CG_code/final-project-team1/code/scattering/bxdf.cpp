#include <bxdf.h>


// Constructor
BxDF::BxDF(BxDFType type) : type(type) { }

// Methods
bool BxDF::MatchesFlags(BxDFType t) const
{
    return (int(type) & int(t)) == int(type);
}

/*virtual*/ Spectrum BxDF::Sample_F(const Vector3f& wo, Vector3f* wi, 
    const Point2f& sample, Float* pdf, 
    BxDFType* sampledType/* = nullptr*/) const
{
    *wi = Sampler::GetInUnitHemisphere(sample);
    if (wo.z < 0) wi->z *= -1;
    *pdf = Pdf(wo, *wi);
    if (sampledType) *sampledType = type;
    return F(wo, *wi);
}

/*virtual*/ Spectrum BxDF::Rho(const Vector3f& wo, int nSamples, const Point2f* samples) const
{
    Spectrum res;
    for (int i = 0; i < nSamples; ++i)
    {
        Vector3f wi;
        Float pdf = 0;
        Spectrum f = Sample_F(wo, &wi, samples[i], &pdf);
        if (pdf > 0) res += f * AbsCosTheta(Vector3f(wi.Normalized())) / pdf;
    }
    return res / nSamples;
}

/*virtual*/ Spectrum BxDF::Rho(int nSamples, const Point2f* samples1, const Point2f* samples2) const
{
    Spectrum res;
    for (int i = 0; i < nSamples; ++i)
    {
        Vector3f wo, wi;
        wo = Sampler::GetInUnitHemisphere(samples1[i]);
        Float pdfo = Pix2Inv, pdfi = 0;
        Spectrum f = Sample_F(wo, &wi, samples2[i], &pdfi);
        if (pdfi > 0)
            res += f * AbsCosTheta(Vector3f(wi.Normalized())) * AbsCosTheta(Vector3f(wo.Normalized())) / (pdfo * pdfi);
    }
    return res / (Pi * nSamples);
}

/*virtual*/ Float BxDF::Pdf(const Vector3f& wo, const Vector3f& wi) const
{
    return SameHemisphere(wo, wi) ? AbsCosTheta(Vector3f(wi.Normalized())) * PiInv : 0;
}





/*static*/ Float Fresnel::FrDielectric(Float etaOu, Float etaIn, Float cosThetaFrom)
{
    cosThetaFrom = std::clamp(cosThetaFrom, Float(-1), Float(1)); //! 一定要做 clamp，否则会有诡异的数学运算结果出现
    Float etaFrom, etaTo;
    if (cosThetaFrom >= 0) // Is entering
        etaFrom = etaOu, etaTo = etaIn;
    else // Is exiting
    {
        etaFrom = etaIn, etaTo = etaOu;
        cosThetaFrom = std::abs(cosThetaFrom);
    }
    Float sinThetaFrom = std::sqrt(std::max(Float(0), 1 - cosThetaFrom * cosThetaFrom));
    Float sinThetaTo   = (etaFrom / etaTo) * sinThetaFrom;
    Float cosThetaTo   = std::sqrt(std::max(Float(0), 1 - sinThetaTo * sinThetaTo));
    Float r_prep = ((etaTo   * cosThetaFrom) - (etaFrom * cosThetaTo)) /
                   ((etaTo   * cosThetaFrom) + (etaFrom * cosThetaTo));
    Float r_proj = ((etaFrom * cosThetaFrom) - (etaTo   * cosThetaTo)) /
                   ((etaFrom * cosThetaFrom) + (etaTo   * cosThetaTo));
    return (r_prep * r_prep + r_proj * r_proj) / 2;
}

// Constructors
FresnelOne::FresnelOne() { }
// Methods
/*override*/ Spectrum FresnelOne::Evaluate(Float cosThetaFrom) const
{
    return Spectrum(Float(1));
}

// Constructors
FresnelDielectric::FresnelDielectric(Float etaOu, Float etaIn) : 
    etaOu(etaOu), etaIn(etaIn) { }
// Methods
/*override*/ Spectrum FresnelDielectric::Evaluate(Float cosThetaFrom) const
{
    return FrDielectric(etaOu, etaIn, cosThetaFrom);
}

// Constructors
FresnelConductor::FresnelConductor(const Spectrum& etaOu, const Spectrum& etaIn, const Spectrum& k) : 
    etaOu(etaOu), etaIn(etaIn), k(k) { }
// Methods
/*override*/ Spectrum FresnelConductor::Evaluate(Float cosThetaFrom) const
{
    Assert(cosThetaFrom >= 0, 
           "FresnelConductor::Evaluate(): Conductors should never support transmission");
    cosThetaFrom = std::clamp(cosThetaFrom, Float(0), Float(1));
    Spectrum eta = etaIn / etaOu;
    Spectrum etak = k / etaOu;

    Float cosThetaI2 = cosThetaFrom * cosThetaFrom;
    Float sinThetaI2 = 1. - cosThetaI2;
    Spectrum eta2 = eta * eta;
    Spectrum etak2 = etak * etak;

    Spectrum t0 = eta2 - etak2 - sinThetaI2;
    Spectrum a2plusb2 = (t0 * t0 + 4 * eta2 * etak2).Sqrted();
    Spectrum t1 = a2plusb2 + cosThetaI2;
    Spectrum a = (0.5 * (a2plusb2 + t0)).Sqrted();
    Spectrum t2 = (Float)2 * cosThetaFrom * a;
    Spectrum Rs = (t1 - t2) / (t1 + t2);

    Spectrum t3 = cosThetaI2 * a2plusb2 + sinThetaI2 * sinThetaI2;
    Spectrum t4 = t2 * sinThetaI2;
    Spectrum Rp = Rs * (t3 - t4) / (t3 + t4);

    return 0.5 * (Rp + Rs);
}